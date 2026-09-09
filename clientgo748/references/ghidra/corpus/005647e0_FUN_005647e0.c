// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005647e0 | Name: FUN_005647e0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_005647e0(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5,
            float *param_6,float *param_7,float *param_8)

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
  float local_48;
  float local_44;
  float local_40;
  
  fVar1 = *param_2 - *param_1;
  fVar2 = param_2[1] - param_1[1];
  fVar3 = param_2[2] - param_1[2];
  fVar4 = *param_3 - *param_1;
  fVar5 = param_3[1] - param_1[1];
  fVar6 = param_3[2] - param_1[2];
  fVar7 = fVar6 * param_5[1] - fVar5 * param_5[2];
  fVar8 = fVar4 * param_5[2] - fVar6 * *param_5;
  fVar9 = fVar5 * *param_5 - fVar4 * param_5[1];
  fVar10 = fVar7 * fVar1 + fVar8 * fVar2 + fVar3 * fVar9;
  fVar12 = (float)_DAT_005a68b8;
  if (fVar10 <= fVar12) {
    if (fVar12 <= fVar10) {
      return 0;
    }
    local_48 = *param_1 - *param_4;
    local_44 = param_1[1] - param_4[1];
    local_40 = param_1[2] - param_4[2];
    fVar10 = -fVar10;
  }
  else {
    local_48 = *param_4 - *param_1;
    local_44 = param_4[1] - param_1[1];
    local_40 = param_4[2] - param_1[2];
  }
  fVar11 = 1.0 / fVar10;
  fVar7 = local_48 * fVar7 + local_44 * fVar8 + local_40 * fVar9;
  if ((fVar12 <= fVar7) && (fVar7 <= fVar10)) {
    fVar8 = local_44 * fVar3 - local_40 * fVar2;
    fVar3 = local_40 * fVar1 - fVar3 * local_48;
    fVar2 = local_48 * fVar2 - local_44 * fVar1;
    fVar1 = fVar8 * *param_5 + fVar3 * param_5[1] + fVar2 * param_5[2];
    if ((fVar12 <= fVar1) &&
       ((fVar1 + fVar7 <= fVar10 &&
        (fVar2 = fVar8 * fVar4 + fVar5 * fVar3 + fVar6 * fVar2, (float)_DAT_005a68b8 <= fVar2)))) {
      *param_6 = fVar7 * fVar11;
      *param_7 = fVar1 * fVar11;
      *param_8 = fVar2 * fVar11;
      return 1;
    }
  }
  return 0;
}

