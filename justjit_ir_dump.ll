; JustJIT LLVM IR Dump

; === add ===
; ModuleID = 'add_ir_dump'
source_filename = "add_ir_dump"

declare void @Py_IncRef(ptr) local_unnamed_addr

declare void @Py_DecRef(ptr) local_unnamed_addr

declare ptr @PyNumber_Add(ptr, ptr) local_unnamed_addr

define ptr @add_ir_dump(ptr %0, ptr %1) local_unnamed_addr {
entry:
  tail call void @Py_IncRef(ptr %0)
  tail call void @Py_IncRef(ptr %1)
  %2 = tail call ptr @PyNumber_Add(ptr %0, ptr %1)
  tail call void @Py_DecRef(ptr %0)
  tail call void @Py_DecRef(ptr %1)
  ret ptr %2
}


; === sum_while ===
; ModuleID = 'sum_while_ir_dump'
source_filename = "sum_while_ir_dump"

declare void @Py_IncRef(ptr) local_unnamed_addr

declare void @Py_DecRef(ptr) local_unnamed_addr

declare ptr @PyLong_FromLongLong(i64) local_unnamed_addr

declare ptr @PyNumber_Add(ptr, ptr) local_unnamed_addr

declare i32 @PyObject_RichCompareBool(ptr, ptr, i32) local_unnamed_addr

declare i32 @PyObject_IsTrue(ptr) local_unnamed_addr

define ptr @sum_while_ir_dump(ptr %0) local_unnamed_addr {
entry:
  %1 = tail call ptr @PyLong_FromLongLong(i64 0)
  %2 = tail call ptr @PyLong_FromLongLong(i64 0)
  tail call void @Py_IncRef(ptr %2)
  tail call void @Py_IncRef(ptr %0)
  %3 = tail call i32 @PyObject_RichCompareBool(ptr %2, ptr %0, i32 0)
  tail call void @Py_DecRef(ptr %2)
  tail call void @Py_DecRef(ptr %0)
  %4 = icmp sgt i32 %3, 0
  %5 = select i1 %4, ptr inttoptr (i64 140718740261344 to ptr), ptr inttoptr (i64 140718740261376 to ptr)
  tail call void @Py_IncRef(ptr nonnull %5)
  %istrue = tail call i32 @PyObject_IsTrue(ptr nonnull %5)
  %tobool_obj = icmp sgt i32 %istrue, 0
  br i1 %tobool_obj, label %merge_20, label %merge_52

merge_20:                                         ; preds = %entry, %store_new15
  %local_1.0 = phi ptr [ %6, %store_new15 ], [ %1, %entry ]
  %local_2.0 = phi ptr [ %8, %store_new15 ], [ %2, %entry ]
  tail call void @Py_IncRef(ptr %local_1.0)
  tail call void @Py_IncRef(ptr %local_2.0)
  %6 = tail call ptr @PyNumber_Add(ptr %local_1.0, ptr %local_2.0)
  tail call void @Py_DecRef(ptr %local_1.0)
  tail call void @Py_DecRef(ptr %local_2.0)
  %is_error = icmp eq ptr %6, null
  br i1 %is_error, label %common.ret, label %binary_op_continue_ret_22

common.ret:                                       ; preds = %store_new9, %merge_20, %merge_52
  %common.ret.op = phi ptr [ %local_1.1, %merge_52 ], [ null, %merge_20 ], [ null, %store_new9 ]
  ret ptr %common.ret.op

merge_52:                                         ; preds = %store_new15, %entry
  %local_1.1 = phi ptr [ %1, %entry ], [ %6, %store_new15 ]
  tail call void @Py_IncRef(ptr %local_1.1)
  br label %common.ret

binary_op_continue_ret_22:                        ; preds = %merge_20
  %is_not_null7.not = icmp eq ptr %local_1.0, null
  br i1 %is_not_null7.not, label %store_new9, label %decref_old8

decref_old8:                                      ; preds = %binary_op_continue_ret_22
  tail call void @Py_DecRef(ptr nonnull %local_1.0)
  br label %store_new9

store_new9:                                       ; preds = %decref_old8, %binary_op_continue_ret_22
  tail call void @Py_IncRef(ptr %local_2.0)
  %7 = tail call ptr @PyLong_FromLongLong(i64 1)
  %8 = tail call ptr @PyNumber_Add(ptr %local_2.0, ptr %7)
  tail call void @Py_DecRef(ptr %local_2.0)
  tail call void @Py_DecRef(ptr %7)
  %is_error11 = icmp eq ptr %8, null
  br i1 %is_error11, label %common.ret, label %binary_op_continue_ret_32

binary_op_continue_ret_32:                        ; preds = %store_new9
  %is_not_null13.not = icmp eq ptr %local_2.0, null
  br i1 %is_not_null13.not, label %store_new15, label %decref_old14

decref_old14:                                     ; preds = %binary_op_continue_ret_32
  tail call void @Py_DecRef(ptr nonnull %local_2.0)
  br label %store_new15

store_new15:                                      ; preds = %decref_old14, %binary_op_continue_ret_32
  tail call void @Py_IncRef(ptr nonnull %8)
  tail call void @Py_IncRef(ptr %0)
  %9 = tail call i32 @PyObject_RichCompareBool(ptr nonnull %8, ptr %0, i32 0)
  tail call void @Py_DecRef(ptr nonnull %8)
  tail call void @Py_DecRef(ptr %0)
  %10 = icmp sgt i32 %9, 0
  %11 = select i1 %10, ptr inttoptr (i64 140718740261344 to ptr), ptr inttoptr (i64 140718740261376 to ptr)
  tail call void @Py_IncRef(ptr nonnull %11)
  %istrue18 = tail call i32 @PyObject_IsTrue(ptr nonnull %11)
  %tobool_obj19 = icmp sgt i32 %istrue18, 0
  br i1 %tobool_obj19, label %merge_20, label %merge_52
}


