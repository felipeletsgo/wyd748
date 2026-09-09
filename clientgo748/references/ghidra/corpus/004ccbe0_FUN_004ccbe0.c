// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ccbe0 | Name: FUN_004ccbe0


undefined4 * __thiscall
FUN_004ccbe0(undefined4 *param_1,float param_2,float param_3,undefined4 param_4,float param_5,
            float param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
            undefined4 param_10,undefined4 param_11,undefined4 param_12)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0869;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0x22,0x18,4,FUN_004cb8e0);
  FUN_00421910(param_1 + 0x3a,0x18,4,FUN_004cb8e0);
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4824;
  param_1[0x52] = param_11;
  param_1[0x53] = param_12;
  param_1[0x54] = param_8;
  param_1[0x21] = param_10;
  param_1[0x5f] = 0;
  uVar1 = FUN_00430f50(param_2,param_3 - (float)param_1[0x53],param_4);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5,param_6 - (float)param_1[0x53],param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5,param_6 + (float)param_1[0x53],param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_2,param_3 + (float)param_1[0x53],param_4);
  FUN_004310a0(uVar1);
  param_1[0x26] = 0x3ca3d70a;
  param_1[0x27] = 0x3f7ae148;
  param_1[0x2c] = 0x3f7ae148;
  param_1[0x2d] = 0x3f7ae148;
  param_1[0x32] = 0x3f7ae148;
  param_1[0x33] = 0x3ca3d70a;
  param_1[0x38] = 0x3ca3d70a;
  param_1[0x39] = 0x3ca3d70a;
  uVar1 = FUN_00430f50(param_2 - (float)param_1[0x53],param_3,param_4);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5 - (float)param_1[0x53],param_6,param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5 + (float)param_1[0x53],param_6,param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_2 + (float)param_1[0x53],param_3,param_4);
  FUN_004310a0(uVar1);
  param_1[0x3e] = 0x3ca3d70a;
  param_1[0x3f] = 0x3f7ae148;
  param_1[0x44] = 0x3f7ae148;
  param_1[0x45] = 0x3f7ae148;
  param_1[0x4a] = 0x3f7ae148;
  param_1[0x4b] = 0x3ca3d70a;
  param_1[0x50] = 0x3ca3d70a;
  param_1[0x51] = 0x3ca3d70a;
  FUN_004ccf4e(param_9);
  param_1[0x60] = 0;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  ExceptionList = local_10;
  return param_1;
}

