// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057aeb0 | Name: FUN_0057aeb0


float * FUN_0057aeb0(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  
  fVar3 = param_3[1];
  fVar4 = param_3[2];
  fVar5 = param_3[3];
  fVar1 = *param_2;
  fVar6 = param_3[5];
  fVar7 = param_3[6];
  fVar8 = param_3[7];
  fVar2 = param_2[1];
  fVar9 = param_3[0xd];
  fVar10 = param_3[0xe];
  fVar11 = param_3[0xf];
  *param_1 = fVar1 * *param_3 + param_3[0xc] + fVar2 * param_3[4];
  param_1[1] = fVar1 * fVar3 + fVar9 + fVar2 * fVar6;
  param_1[2] = fVar1 * fVar4 + fVar10 + fVar2 * fVar7;
  param_1[3] = fVar1 * fVar5 + fVar11 + fVar2 * fVar8;
  return param_1;
}

