// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054737d | Name: FUN_0054737d


undefined4 __fastcall FUN_0054737d(int param_1)

{
  void *pvVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2a0e;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x60) == 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x358);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_20 = 0;
    }
    else {
      local_20 = FUN_004bd420(param_1 + 0x160,param_1 + 0x180,*(undefined4 *)(param_1 + 0x5c),0,0,0,
                              0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x60) = local_20;
    if (*(int *)(param_1 + 0x60) != 0) {
      FUN_004be7b1();
    }
  }
  ExceptionList = local_10;
  return 1;
}

