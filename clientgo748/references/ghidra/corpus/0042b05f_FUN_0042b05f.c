// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b05f | Name: FUN_0042b05f


undefined4 * __thiscall
FUN_0042b05f(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  FUN_0042a7d1(&param_2,param_3,1,param_4);
  *param_1 = &PTR_FUN_005a4188;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = param_5;
  param_1[8] = 0;
  param_1[9] = 0;
  return param_1;
}

