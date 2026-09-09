// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c92e9 | Name: FUN_004c92e9


void __fastcall FUN_004c92e9(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a081c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4784;
  local_8 = 0;
  if (param_1[0x56] != 0) {
    if ((undefined4 *)param_1[0x56] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x56])(1);
    }
    param_1[0x56] = 0;
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

