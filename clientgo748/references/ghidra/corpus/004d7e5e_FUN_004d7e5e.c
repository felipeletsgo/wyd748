// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d7e5e | Name: FUN_004d7e5e


void __fastcall FUN_004d7e5e(int param_1)

{
  void *pvVar1;
  undefined4 local_2c;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0b57;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x60) == 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x358);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_004bd420(param_1 + 0x160,param_1 + 0x180,*(undefined4 *)(param_1 + 0x5c),0,0,0,
                              0);
    }
    *(undefined4 *)(param_1 + 0x60) = local_28;
  }
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0x60) != 0) {
    FUN_004be7b1();
  }
  if ((0 < *(int *)(param_1 + 0x220)) && (*(int *)(param_1 + 0x21c) == 0)) {
    pvVar1 = operator_new(0x358);
    local_8 = 1;
    if (pvVar1 == (void *)0x0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_004bd420(param_1 + 0x1fc,param_1 + 0x180,*(undefined4 *)(param_1 + 0x220),0,0,0
                              ,0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x21c) = local_2c;
  }
  if (*(int *)(param_1 + 0x21c) != 0) {
    FUN_004be7b1();
  }
  ExceptionList = local_10;
  return;
}

