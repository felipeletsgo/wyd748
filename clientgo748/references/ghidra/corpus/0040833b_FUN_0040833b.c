// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040833b | Name: FUN_0040833b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0040833b(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,int param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
            undefined4 param_10,undefined4 param_11,undefined4 param_12)

{
  void *pvVar1;
  undefined4 local_a8;
  undefined1 local_94 [128];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059eccc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040104c(param_2,param_3,param_4,param_5);
  local_8 = 0;
  *param_1 = &PTR_FUN_005a3c6c;
  for (local_14 = 0; local_14 < 0xe; local_14 = local_14 + 1) {
    if (local_14 == 0) {
      FUN_0058f078(local_94,&DAT_005b10c8);
    }
    else if (local_14 == 1) {
      FUN_0058f078(local_94,&DAT_005b10cc);
    }
    else if (local_14 == 0xc) {
      FUN_0058f078(local_94,&DAT_005b10d0);
    }
    else if (local_14 == 0xd) {
      FUN_0058f078(local_94,&DAT_005b10d4);
    }
    else {
      FUN_0058f078(local_94,&DAT_005b10d8,param_6 + -2 + local_14);
    }
    pvVar1 = operator_new(0x208);
    local_8._0_1_ = 1;
    if (pvVar1 == (void *)0x0) {
      local_a8 = 0;
    }
    else {
      local_a8 = FUN_00402f01(0xfffffffe,(float)(local_14 * 0x1e) + _DAT_005a3c68,0,0x41c00000,
                              0x41800000,0x55aaaaaa,1,local_94);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[local_14 + 0x19] = local_a8;
    if (param_1[local_14 + 0x19] != 0) {
      (**(code **)(*(int *)param_1[local_14 + 0x19] + 0x44))(local_14 + 0x350);
      FUN_0054ac09(param_1[local_14 + 0x19]);
    }
  }
  FUN_004085cd(param_6,param_7,param_8,param_9,param_10,param_11,param_12);
  ExceptionList = local_10;
  return param_1;
}

