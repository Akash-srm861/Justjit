cmake_minimum_required(VERSION 3.20)
project(justjit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

# Find Python
if (CMAKE_VERSION VERSION_LESS 3.18)
  set(DEV_MODULE Development)
else()
  set(DEV_MODULE Development.Module)
endif()
find_package(Python 3.8 COMPONENTS Interpreter ${DEV_MODULE} REQUIRED)

# Find nanobind
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
find_package(nanobind CONFIG REQUIRED)

# LLVM configuration
# conda-forge llvmdev provides DYNAMIC libraries (libLLVM.so / LLVM.dll)
# We link dynamically and bundle the shared lib into the wheel

# On Windows, LLVM may have been built with LibXml2 dependency
# Find it before LLVM to satisfy LLVMExports.cmake requirements
if(WIN32)
    find_package(LibXml2 QUIET)
    if(LibXml2_FOUND)
        message(STATUS "Found LibXml2: ${LIBXML2_LIBRARIES}")
    else()
        # Create an empty imported target to satisfy LLVM dependency
        # LLVM may not actually need it at runtime for our use case
        if(NOT TARGET LibXml2::LibXml2)
            add_library(LibXml2::LibXml2 INTERFACE IMPORTED)
            message(STATUS "Created stub LibXml2::LibXml2 target for LLVM dependency")
        endif()
    endif()
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM library dir: ${LLVM_LIBRARY_DIR}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Additional Windows-specific definitions
if(WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS
    )
endif()

# Find zlib (required by LLVM)
find_package(ZLIB REQUIRED)

# Create the Python extension module
nanobind_add_module(_core NB_STATIC NOMINSIZE
    src/jit_core.cpp
    src/bindings.cpp
)

# Link against LLVM libraries
# Windows: LLVM-C.dll only exposes C API, so we must link static component libs for C++ API
# Linux/macOS: Can use the unified libLLVM shared library

# Map all required LLVM components to library names
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    OrcJIT
    ExecutionEngine
    RuntimeDyld
    native
    Passes
    TransformUtils
    Analysis
    Target
    Object
    MC
    IRReader
    InstCombine
    ScalarOpts
    Vectorize
    IPO
    Linker
    BitWriter
    BitReader
    MCParser
    ObjCARCOpts
    AggressiveInstCombine
    CodeGen
    SelectionDAG
    AsmPrinter
    MIRParser
    GlobalISel
    DebugInfoDWARF
    DebugInfoCodeView
    DebugInfoMSF
    DebugInfoPDB
    Symbolize
    Demangle
    TextAPI
    BinaryFormat
    Remarks
    ProfileData
    Coverage
    LTO
    Extensions
    CFGuard
    Coroutines
    JITLink
    OrcTargetProcess
    OrcShared
    WindowsDriver
    WindowsManifest
)

message(STATUS "LLVM libraries to link: ${LLVM_LIBS}")

if(WIN32)
    # Windows: Must use static component libraries for C++ API
    target_link_libraries(_core PRIVATE ${LLVM_LIBS})
else()
    # Linux/macOS: Try to use the unified shared library first
    find_library(LLVM_SHARED_LIB NAMES LLVM LLVM-${LLVM_VERSION_MAJOR} PATHS ${LLVM_LIBRARY_DIR} NO_DEFAULT_PATH)
    if(LLVM_SHARED_LIB)
        message(STATUS "Found LLVM shared library: ${LLVM_SHARED_LIB}")
        target_link_libraries(_core PRIVATE ${LLVM_SHARED_LIB})
    else()
        # Fallback: use component libraries
        target_link_libraries(_core PRIVATE ${LLVM_LIBS})
    endif()
endif()

# Link zlib
target_link_libraries(_core PRIVATE ZLIB::ZLIB)

# Platform-specific system libraries
if(WIN32)
    target_link_libraries(_core PRIVATE
        version
        psapi
        shell32
        ole32
        uuid
        advapi32
    )
elseif(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(_core PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )
elseif(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(_core PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

# Install the Python extension
install(TARGETS _core LIBRARY DESTINATION justjit)

# The LLVM shared library will be bundled by wheel repair tools:
# - auditwheel (Linux) - bundles .so files
# - delocate (macOS) - bundles .dylib files  
# - delvewheel (Windows) - bundles .dll files