// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fb40 | Name: FUN_0052fb40


float * FUN_0052fb40(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  
  FUN_00430f10();
  fVar1 = param_2[2];
  fVar2 = *param_3;
  fVar3 = *param_2;
  fVar4 = param_3[2];
  fVar5 = *param_2;
  fVar6 = param_3[1];
  fVar7 = param_2[1];
  fVar8 = *param_3;
  *param_1 = param_2[1] * param_3[2] - param_2[2] * param_3[1];
  param_1[1] = fVar1 * fVar2 - fVar3 * fVar4;
  param_1[2] = fVar5 * fVar6 - fVar7 * fVar8;
  return param_1;
}

