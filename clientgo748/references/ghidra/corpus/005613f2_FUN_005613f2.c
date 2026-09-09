// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005613f2 | Name: FUN_005613f2


void FUN_005613f2(float *param_1,float *param_2,float *param_3,float *param_4,float param_5,
                 float param_6)

{
  *param_1 = (*param_4 - *param_2) * param_6 + (*param_3 - *param_2) * param_5 + *param_2;
  param_1[1] = (param_4[1] - param_2[1]) * param_6 + (param_3[1] - param_2[1]) * param_5 +
               param_2[1];
  param_1[2] = (param_4[2] - param_2[2]) * param_6 + (param_3[2] - param_2[2]) * param_5 +
               param_2[2];
  return;
}

