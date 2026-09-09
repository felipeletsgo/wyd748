// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c8f70 | Name: FUN_004c8f70


undefined4 * __thiscall
FUN_004c8f70(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0809;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0x20,0x18,4,FUN_004cb8e0);
  CSimpleArray<>();
  CSimpleArray<>();
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4784;
  param_1[0x45] = 0;
  param_1[0x52] = 0;
  param_1[0x3f] = 0;
  param_1[0x46] = 0;
  param_1[0x43] = 1;
  param_1[0x3e] = param_3;
  param_1[0x38] = param_4;
  param_1[0x39] = param_5;
  param_1[0x3a] = param_6;
  param_1[0x3b] = param_7;
  param_1[0x3c] = param_7;
  param_1[0x3d] = param_7;
  param_1[0x44] = param_2;
  param_1[0x48] = 0;
  param_1[0x49] = param_8;
  param_1[0x4a] = param_9;
  param_1[0x41] = 1;
  param_1[0x42] = 1;
  param_1[0x40] = 0;
  param_1[7] = 0;
  param_1[0x56] = 0;
  param_1[0x4f] = 0x3f800000;
  param_1[0x50] = 0x40000000;
  param_1[0x51] = 0x40c00000;
  param_1[0x47] = 0x3f333333;
  uVar1 = FUN_00430f50(0,0,0);
  uVar1 = FUN_004310a0(uVar1);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0xbf000000,0xbf000000,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0x3f000000,0xbf000000,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0x3f000000,0x3f000000,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0xbf000000,0x3f000000,0);
  FUN_004310a0(uVar1);
  param_1[0x24] = 0x3ca3d70a;
  param_1[0x25] = 0x3f7ae148;
  param_1[0x2a] = 0x3f7ae148;
  param_1[0x2b] = 0x3f7ae148;
  param_1[0x30] = 0x3f7ae148;
  param_1[0x31] = 0x3ca3d70a;
  param_1[0x36] = 0x3ca3d70a;
  param_1[0x37] = 0x3ca3d70a;
  FUN_004c94b1(0xffffffff);
  if (param_2 == 0x21) {
    param_1[0x24] = 0;
    param_1[0x25] = 0x3f800000;
    param_1[0x2a] = 0x3f800000;
    param_1[0x2b] = 0x3f800000;
    param_1[0x30] = 0x3f800000;
    param_1[0x31] = 0;
    param_1[0x36] = 0;
    param_1[0x37] = 0;
  }
  param_1[0x57] = 0;
  ExceptionList = local_10;
  return param_1;
}

