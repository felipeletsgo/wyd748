// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cd800 | Name: FUN_004cd800


undefined4 * __thiscall
FUN_004cd800(undefined4 *param_1,undefined4 param_2,undefined4 param_3,float param_4,float param_5,
            undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0889;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0x20,0x1c,4,FUN_004cb8e0);
  FUN_00430f10();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a4864;
  param_1[0x3f] = 0;
  param_1[0x44] = 0;
  param_1[0x43] = param_2;
  param_1[0x45] = 0;
  param_1[0x46] = param_7;
  param_1[0x47] = param_8;
  param_1[0x3e] = param_3;
  param_1[0x41] = param_6;
  param_1[0x42] = param_6;
  param_1[0x48] = 0;
  param_1[0x49] = 0;
  param_1[0x3c] = param_4 * DAT_005b490c;
  param_1[0x3d] = param_5 * DAT_005b4910;
  param_1[0x4e] = 0;
  param_1[0x4f] = 0;
  param_1[0x40] = 1;
  uVar1 = FUN_00430f50(0,0,0x3c23d70a);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0,0,0x3c23d70a);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0,0,0x3c23d70a);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0,0,0x3c23d70a);
  FUN_004310a0(uVar1);
  param_1[0x23] = 0x3f800000;
  param_1[0x2a] = 0x3f800000;
  param_1[0x31] = 0x3f800000;
  param_1[0x38] = 0x3f800000;
  param_1[0x25] = 0x3c23d70a;
  param_1[0x26] = 0x3c23d70a;
  param_1[0x2c] = 0x3f7d70a4;
  param_1[0x2d] = 0x3c23d70a;
  param_1[0x33] = 0x3f7d70a4;
  param_1[0x34] = 0x3f7d70a4;
  param_1[0x3a] = 0x3c23d70a;
  param_1[0x3b] = 0x3f7d70a4;
  FUN_004cdb05(0xffffffff);
  ExceptionList = local_10;
  return param_1;
}

