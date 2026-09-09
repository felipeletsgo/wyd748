// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d1960 | Name: FUN_004d1960


float * __thiscall FUN_004d1960(float *param_1,float param_2)

{
  *param_1 = param_2 * *param_1;
  param_1[1] = param_2 * param_1[1];
  param_1[2] = param_2 * param_1[2];
  return param_1;
}

