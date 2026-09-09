// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c5c10 | Name: FUN_004c5c10


float * __thiscall FUN_004c5c10(float *param_1,float *param_2)

{
  *param_1 = *param_1 + *param_2;
  param_1[1] = param_1[1] + param_2[1];
  param_1[2] = param_1[2] + param_2[2];
  param_1[3] = param_1[3] + param_2[3];
  return param_1;
}

