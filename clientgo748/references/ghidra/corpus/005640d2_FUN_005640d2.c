// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005640d2 | Name: FUN_005640d2


void FUN_005640d2(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  
  local_10 = *param_3 - *param_2;
  local_c = param_3[1] - param_2[1];
  local_8 = param_3[2] - param_2[2];
  thunk_FUN_005611dd(&local_10,&local_10);
  local_1c = local_8 * param_4[1] - local_c * param_4[2];
  local_18 = local_10 * param_4[2] - local_8 * *param_4;
  local_14 = local_c * *param_4 - local_10 * param_4[1];
  thunk_FUN_005611dd(&local_1c,&local_1c);
  param_1[4] = local_18;
  param_1[8] = local_14;
  fVar4 = local_c * local_14 - local_8 * local_18;
  fVar6 = local_8 * local_1c - local_10 * local_14;
  fVar5 = local_10 * local_18 - local_c * local_1c;
  *param_1 = local_1c;
  param_1[0xc] = -(local_14 * param_2[2] + local_1c * *param_2 + local_18 * param_2[1]);
  param_1[1] = fVar4;
  param_1[5] = fVar6;
  param_1[9] = fVar5;
  fVar1 = param_2[1];
  fVar2 = *param_2;
  fVar3 = param_2[2];
  param_1[2] = local_10;
  param_1[6] = local_c;
  param_1[10] = local_8;
  param_1[0xd] = -(fVar5 * fVar3 + fVar4 * fVar2 + fVar6 * fVar1);
  param_1[0xe] = -(local_8 * param_2[2] + local_10 * *param_2 + local_c * param_2[1]);
  param_1[3] = 0.0;
  param_1[7] = 0.0;
  param_1[0xb] = 0.0;
  param_1[0xf] = 1.0;
  return;
}

