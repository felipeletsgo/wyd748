// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00540fb4 | Name: FUN_00540fb4


void __fastcall FUN_00540fb4(undefined4 *param_1)

{
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a26fd;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5a94;
  local_8 = 0;
  for (local_14 = 0; local_14 < 6; local_14 = local_14 + 1) {
    if (param_1[local_14 + 0xb] != 0) {
      if ((undefined4 *)param_1[local_14 + 0xb] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[local_14 + 0xb])(1);
      }
      param_1[local_14 + 0xb] = 0;
    }
  }
  local_8 = 0xffffffff;
  FUN_0054aa45();
  ExceptionList = local_10;
  return;
}

