// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004310a0 | Name: FUN_004310a0


undefined4 * __thiscall FUN_004310a0(undefined4 *param_1,undefined4 *param_2)

{
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  param_1[2] = param_2[2];
  return param_1;
}

