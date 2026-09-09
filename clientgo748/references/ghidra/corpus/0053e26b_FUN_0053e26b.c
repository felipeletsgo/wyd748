// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053e26b | Name: FUN_0053e26b


void __fastcall FUN_0053e26b(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a26cc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5a00;
  local_8 = 0;
  if (param_1 == *(undefined4 **)(DAT_013b71e8 + 0x1af48)) {
    *(undefined4 *)(DAT_013b71e8 + 0x1af48) = 0;
  }
  if (param_1[0x18] != 0) {
    if (param_1[0x18] != 0) {
      FUN_00493d20(1);
    }
    param_1[0x18] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0054aa45();
  ExceptionList = local_10;
  return;
}

