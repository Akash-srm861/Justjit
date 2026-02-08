cmake_minimum_required(VERSION 3.20)
project(justjit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIX: Allow external build type override (e.g., MinSizeRel from CI)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Find Python
find_package(Python 3.13 COMPONENTS Interpreter Development.Module REQUIRED)

# Find nanobind
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE nanobind_ROOT)
find_package(nanobind CONFIG REQUIRED)

# LLVM configuration
# Windows LibXml2 workaround
if(WIN32)
    find_package(LibXml2 QUIET)
    if(LibXml2_FOUND)
        if(NOT TARGET LibXml2::LibXml2)
            add_library(LibXml2::LibXml2 INTERFACE IMPORTED)
        endif()
    endif()
endif()

if(POLICY CMP0111)
    cmake_policy(SET CMP0111 OLD)
endif()

find_package(LLVM REQUIRED CONFIG)

# Alpine/Edge case workaround
if(NOT TARGET LLVMTestingAnnotations)
    add_library(LLVMTestingAnnotations INTERFACE IMPORTED)
endif()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# CLANG CONFIGURATION (Modified to FORCE STATIC LINKING)
# ============================================================================
find_package(Clang CONFIG HINTS ${LLVM_DIR}/../clang QUIET)
if(Clang_FOUND)
    message(STATUS "Found Clang - enabling InlineCCompiler")
    add_definitions(-DJUSTJIT_HAS_CLANG=1)
    include_directories(${CLANG_INCLUDE_DIRS})
    
    # FORCE STATIC: We manually list the component libraries.
    # We do NOT check for 'clang-cpp' (the shared lib) anymore.
    set(CLANG_LIBS
        clangFrontend
        clangCodeGen
        clangSema
        clangAST
        clangParse
        clangLex
        clangAnalysis
        clangEdit
        clangSerialization
        clangBasic
        clangSupport
    )
else()
    message(STATUS "Clang not found - InlineCCompiler will be disabled")
    set(CLANG_LIBS "")
endif()

# Find Clang resource headers (stddef.h etc)
if(Clang_FOUND)
    string(REGEX MATCH "^[0-9]+" LLVM_MAJOR_VERSION "${LLVM_PACKAGE_VERSION}")
    get_filename_component(LLVM_SOURCE_ROOT "${LLVM_DIR}/../../../.." ABSOLUTE)
    
    set(CLANG_RESOURCE_SEARCH_PATHS
        "${LLVM_LIBRARY_DIR}/clang/${LLVM_MAJOR_VERSION}/include"
        "${CMAKE_PREFIX_PATH}/lib/clang/${LLVM_MAJOR_VERSION}/include"
        "${CMAKE_PREFIX_PATH}/Library/lib/clang/${LLVM_MAJOR_VERSION}/include"
        "/usr/lib64/clang/${LLVM_MAJOR_VERSION}/include"
        "/usr/lib/clang/${LLVM_MAJOR_VERSION}/include"
        "${LLVM_SOURCE_ROOT}/clang/lib/Headers"
    )
    
    set(CLANG_RESOURCE_INCLUDE "")
    foreach(path ${CLANG_RESOURCE_SEARCH_PATHS})
        if(EXISTS "${path}/stddef.h")
            set(CLANG_RESOURCE_INCLUDE "${path}")
            message(STATUS "Found Clang resource headers: ${path}")
            break()
        endif()
    endforeach()
    
    set(LIBCXX_SEARCH_PATHS
        "${CMAKE_PREFIX_PATH}/include/c++/v1"
        "${CMAKE_PREFIX_PATH}/Library/include/c++/v1"
        "/usr/include/c++/v1"
        "${LLVM_SOURCE_ROOT}/libcxx/include"
    )
    set(LIBCXX_INCLUDE "")
    foreach(path ${LIBCXX_SEARCH_PATHS})
        if(EXISTS "${path}/__config")
            set(LIBCXX_INCLUDE "${path}")
            message(STATUS "Found libc++ headers: ${path}")
            break()
        endif()
    endforeach()
endif()

find_package(ZLIB REQUIRED)

# ============================================================================
# BUILD TARGET
# ============================================================================
nanobind_add_module(_core NB_STATIC NOMINSIZE
    src/jit_core.cpp
    src/bindings.cpp
    src/raii_wrapper.cpp
)
target_link_libraries(_core PRIVATE Python::Module)

if(APPLE)
  target_link_options(_core PRIVATE "LINKER:-undefined,dynamic_lookup")
endif()

if(CLANG_RESOURCE_INCLUDE)
    target_compile_definitions(_core PRIVATE JUSTJIT_CLANG_RESOURCE_DIR="${CLANG_RESOURCE_INCLUDE}")
endif()
if(LIBCXX_INCLUDE)
    target_compile_definitions(_core PRIVATE JUSTJIT_LIBCXX_DIR="${LIBCXX_INCLUDE}")
endif()

set(EMBEDDED_LIBC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libc-headers")
if(EXISTS "${EMBEDDED_LIBC_DIR}/stdio.h")
    target_compile_definitions(_core PRIVATE JUSTJIT_EMBEDDED_LIBC_DIR="${EMBEDDED_LIBC_DIR}")
    message(STATUS "Using embedded musl libc headers: ${EMBEDDED_LIBC_DIR}")
endif()

# Platform SDK Headers
if(WIN32)
    set(WINSDK_INCLUDE "")
    file(GLOB WINSDK_VERSIONS "C:/Program Files (x86)/Windows Kits/10/Include/*")
    if(WINSDK_VERSIONS)
        list(SORT WINSDK_VERSIONS)
        list(REVERSE WINSDK_VERSIONS)
        list(GET WINSDK_VERSIONS 0 WINSDK_LATEST)
        if(EXISTS "${WINSDK_LATEST}/ucrt")
            target_compile_definitions(_core PRIVATE
                JUSTJIT_WINSDK_UCRT_DIR="${WINSDK_LATEST}/ucrt"
                JUSTJIT_WINSDK_SHARED_DIR="${WINSDK_LATEST}/shared"
                JUSTJIT_WINSDK_UM_DIR="${WINSDK_LATEST}/um")
        endif()
    endif()
    file(GLOB MSVC_VERSIONS "C:/Program Files/Microsoft Visual Studio/2022/*/VC/Tools/MSVC/*")
    if(MSVC_VERSIONS)
        list(SORT MSVC_VERSIONS)
        list(REVERSE MSVC_VERSIONS)
        list(GET MSVC_VERSIONS 0 MSVC_LATEST)
        if(EXISTS "${MSVC_LATEST}/include")
            target_compile_definitions(_core PRIVATE JUSTJIT_MSVC_INCLUDE_DIR="${MSVC_LATEST}/include")
        endif()
    endif()
endif()

if(UNIX AND NOT APPLE)
    if(EXISTS "/usr/include")
        target_compile_definitions(_core PRIVATE JUSTJIT_LINUX_INCLUDE_DIR="/usr/include")
    endif()
endif()

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE MACOS_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(EXISTS "${MACOS_SDK_PATH}/usr/include")
        target_compile_definitions(_core PRIVATE JUSTJIT_MACOS_SDK_DIR="${MACOS_SDK_PATH}/usr/include")
    endif()
endif()


# ============================================================================
# LLVM LINKING (Modified to FORCE STATIC LINKING)
# ============================================================================
llvm_map_components_to_libnames(LLVM_LIBS
    Core Support OrcJIT ExecutionEngine RuntimeDyld native Passes TransformUtils
    Analysis Target Object MC IRReader InstCombine ScalarOpts Vectorize IPO Linker
    BitWriter BitReader MCParser ObjCARCOpts AggressiveInstCombine CodeGen
    SelectionDAG AsmPrinter MIRParser GlobalISel DebugInfoDWARF DebugInfoCodeView
    DebugInfoMSF DebugInfoPDB Symbolize Demangle TextAPI BinaryFormat Remarks
    ProfileData Coverage LTO Extensions CFGuard Coroutines JITLink OrcTargetProcess
    OrcShared WindowsDriver WindowsManifest
)

message(STATUS "LLVM libraries to link: ${LLVM_LIBS}")

# FORCE STATIC: We link the mapped component libs directly on ALL platforms.
# We Removed the logic that searched for LLVM_SHARED_LIB on Linux/macOS.
target_link_libraries(_core PRIVATE ${LLVM_LIBS})

if(Clang_FOUND)
    target_link_libraries(_core PRIVATE ${CLANG_LIBS})
endif()

target_link_libraries(_core PRIVATE ZLIB::ZLIB)

# System Libraries
if(WIN32)
    target_link_libraries(_core PRIVATE version psapi shell32 ole32 uuid advapi32)
elseif(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(_core PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
elseif(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(_core PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

install(TARGETS _core LIBRARY DESTINATION justjit)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libc-headers")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libc-headers"
            DESTINATION justjit/vendor
            FILES_MATCHING PATTERN "*.h")
endif()