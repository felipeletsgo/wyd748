// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042c160 | Name: FUN_0042c160


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0042c160(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f069;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00425910();
  local_8 = 0;
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  param_1[0xa9c1] = 0;
  param_1[0xa9c2] = 0;
  param_1[0xa9c3] = 0;
  param_1[0xa9c4] = 0;
  FUN_00421910(param_1 + 0xa9c5,0x1c,4,FUN_004cb8e0);
  param_1[0xaa15] = 0;
  param_1[0xaa16] = 0;
  param_1[0xaa17] = 0;
  *param_1 = &PTR_FUN_005a4190;
  param_1[0xac0d] = 0;
  param_1[0xa941] = param_2;
  param_1[0xa942] = param_3;
  param_1[0xa944] = param_4;
  param_1[0xa943] = param_5;
  param_1[0xa979] = 0;
  param_1[0xa97a] = 0;
  param_1[0xac13] = 0;
  param_1[0xac11] = 0x41400000;
  param_1[0xac12] = 0x41e00000;
  param_1[0xac0e] = 1;
  param_1[0xac0f] = 0;
  param_1[0xac14] = 0;
  DAT_005b490c = (float)(uint)param_1[0xa941] / _DAT_005a34a8;
  DAT_005b4910 = (float)(uint)param_1[0xa942] / _DAT_005a34a4;
  DAT_005b1094 = __ftol();
  _memset(param_1 + 0xaa25,-1,0x400);
  for (local_14 = 0; local_14 < 8; local_14 = local_14 + 1) {
    _memset(param_1 + local_14 * 0x1d + 0xab25,0,0x74);
    param_1[local_14 + 0xa9b1] = 0;
  }
  for (local_18 = 0; local_18 < 2; local_18 = local_18 + 1) {
    _memset(param_1 + local_18 * 0x1a + 0xa9e1,0,0x68);
    param_1[local_18 * 0x1a + 0xa9e1] = 3;
    param_1[local_18 * 0x1a + 0xa9ea] = 0;
    param_1[local_18 * 0x1a + 0xa9eb] = 0;
    param_1[local_18 * 0x1a + 0xa9ec] = 0;
  }
  param_1[0xa9b9] = 0x3f800000;
  param_1[0xa9ba] = 0x3f800000;
  param_1[0xa9bb] = 0x3f800000;
  param_1[0xa9bd] = 0x3f333333;
  param_1[0xa9be] = 0x3f333333;
  param_1[0xa9bf] = 0x3f333333;
  DAT_005ccf9c = param_1;
  param_1[0xac10] = 0;
  param_1[0xac15] = 0;
  ExceptionList = local_10;
  return param_1;
}

