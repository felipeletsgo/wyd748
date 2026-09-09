// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005638af | Name: FUN_005638af


void FUN_005638af(float *param_1,float *param_2,float *param_3)

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
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  
  fVar1 = param_3[9];
  fVar2 = param_2[2];
  fVar3 = param_3[5];
  fVar4 = param_2[1];
  fVar5 = param_3[1];
  fVar6 = *param_2;
  fVar7 = param_3[0xd];
  fVar8 = param_3[10];
  fVar9 = param_2[2];
  fVar10 = param_3[6];
  fVar11 = param_2[1];
  fVar12 = param_3[2];
  fVar13 = *param_2;
  fVar14 = param_3[0xe];
  fVar15 = param_3[0xb];
  fVar16 = param_2[2];
  fVar17 = param_3[7];
  fVar18 = param_2[1];
  fVar19 = param_3[3];
  fVar20 = *param_2;
  fVar21 = param_3[0xf];
  *param_1 = *param_2 * *param_3 + param_3[4] * param_2[1] + param_3[8] * param_2[2] + param_3[0xc];
  param_1[1] = fVar5 * fVar6 + fVar3 * fVar4 + fVar1 * fVar2 + fVar7;
  param_1[2] = fVar12 * fVar13 + fVar10 * fVar11 + fVar8 * fVar9 + fVar14;
  param_1[3] = fVar19 * fVar20 + fVar17 * fVar18 + fVar15 * fVar16 + fVar21;
  return;
}

