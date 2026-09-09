// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d8910 | Name: FUN_004d8910


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004d8910(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 local_4c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0bb4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4b20;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x2b] = uVar1;
  param_1[7] = param_2;
  param_1[0x27] = param_3;
  param_1[0x28] = param_4;
  param_1[0x21] = 0x3f800000;
  param_1[0x20] = param_7;
  param_1[0x2c] = 900;
  param_1[0x29] = param_5;
  param_1[0x2a] = param_6;
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0,0,param_4);
  FUN_004310a0(uVar1);
  FUN_00430f50(0,0,0);
  for (local_14 = 0; local_14 < 5; local_14 = local_14 + 1) {
    param_1[local_14 + 0x22] = 0;
    pvVar2 = operator_new(0x184);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_4c = 0;
    }
    else {
      local_4c = FUN_004ccbe0(param_1[0x2d],param_1[0x2e],param_1[0x2f],param_1[0x30],param_1[0x31],
                              param_1[0x32],0x80,param_1[0x29],1000,
                              _DAT_005a415c * (float)param_1[0x20],
                              _DAT_005a415c * (float)param_1[0x20]);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[local_14 + 0x22] = local_4c;
    if (param_1[local_14 + 0x22] != 0) {
      *(undefined4 *)(param_1[local_14 + 0x22] + 0x6c) = 1;
      *(undefined4 *)(param_1[local_14 + 0x22] + 0x17c) = 1;
      FUN_0054ac09(param_1[local_14 + 0x22]);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

