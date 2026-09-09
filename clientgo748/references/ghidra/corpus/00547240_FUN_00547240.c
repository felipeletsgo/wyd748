// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00547240 | Name: FUN_00547240


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00547240(float *param_1,undefined4 param_2,float param_3)

{
  param_3 = _DAT_005a3660 / param_3;
  FUN_00430f50(param_3 * *param_1,param_3 * param_1[1],param_3 * param_1[2]);
  return param_2;
}

