// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00531a00 | Name: FUN_00531a00


void __fastcall FUN_00531a00(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a2539;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a57f4;
  local_8 = 0;
  if (param_1[100] != 0) {
    if ((undefined4 *)param_1[100] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[100])(1);
    }
    param_1[100] = 0;
  }
  if (param_1[0x65] != 0) {
    if ((undefined4 *)param_1[0x65] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x65])(1);
    }
    param_1[0x65] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

