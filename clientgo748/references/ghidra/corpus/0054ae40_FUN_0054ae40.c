// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ae40 | Name: FUN_0054ae40


undefined4 * __fastcall FUN_0054ae40(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 local_38;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2b24;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a5d34;
  param_1[0x5f] = 0;
  *(undefined2 *)(param_1 + 0x58) = 0;
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  pvVar2 = operator_new(0x180);
  local_8._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_004c8f70(0x38,0,0x3e99999a,0x3e99999a,0x3e99999a,0,1,0x50);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x5f] = local_38;
  if (param_1[0x5f] != 0) {
    *(undefined4 *)(param_1[0x5f] + 0x104) = 0;
    *(undefined4 *)(param_1[0x5f] + 0x6c) = 1;
    (**(code **)(*(int *)param_1[0x5f] + 0x3c))(0xffffee55);
    FUN_0054ac09(param_1[0x5f]);
  }
  ExceptionList = local_10;
  return param_1;
}

