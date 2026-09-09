// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1c72 | Name: FUN_004b1c72


undefined4 FUN_004b1c72(undefined4 param_1,float *param_2,float *param_3)

{
  float local_10;
  float local_c;
  float local_8;
  
  CSimpleArray<>();
  local_10 = param_2[1] * param_3[2] - param_2[2] * param_3[1];
  local_c = param_2[2] * *param_3 - *param_2 * param_3[2];
  local_8 = *param_2 * param_3[1] - param_2[1] * *param_3;
  FUN_004310a0(&local_10);
  return param_1;
}

