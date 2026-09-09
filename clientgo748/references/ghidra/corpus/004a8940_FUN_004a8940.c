// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a8940 | Name: FUN_004a8940


float * FUN_004a8940(float *param_1,float *param_2,float *param_3,float param_4)

{
  *param_1 = (*param_3 - *param_2) * param_4 + *param_2;
  param_1[1] = (param_3[1] - param_2[1]) * param_4 + param_2[1];
  param_1[2] = (param_3[2] - param_2[2]) * param_4 + param_2[2];
  return param_1;
}

