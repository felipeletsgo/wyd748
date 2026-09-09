// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049f053 | Name: FUN_0049f053


void __fastcall FUN_0049f053(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a022c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a44b4;
  local_8 = 0;
  if (param_1[0x9b89] != 0) {
    if ((undefined4 *)param_1[0x9b89] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x9b89])(1);
    }
    param_1[0x9b89] = 0;
  }
  local_8 = 0xffffffff;
  FUN_00494c00();
  ExceptionList = local_10;
  return;
}

