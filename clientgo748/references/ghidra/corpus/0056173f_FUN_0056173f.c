// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056173f | Name: FUN_0056173f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0056173f(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5,
                 float param_6)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  
  fVar5 = _DAT_005a67d4;
  fVar1 = param_6 * param_6;
  fVar2 = param_6 * fVar1;
  fVar4 = (fVar1 * _DAT_005a680c - fVar2) - param_6;
  fVar3 = (fVar2 * _DAT_005a6800 - fVar1 * _DAT_005a6808) + _DAT_005a680c;
  param_6 = (fVar1 * _DAT_005a6804 - fVar2 * _DAT_005a6800) + param_6;
  fVar2 = fVar2 - fVar1;
  *param_1 = (fVar2 * *param_5 + param_6 * *param_4 + fVar3 * *param_3 + fVar4 * *param_2) *
             _DAT_005a67d4;
  param_1[1] = (fVar4 * param_2[1] + fVar3 * param_3[1] + param_6 * param_4[1] + fVar2 * param_5[1])
               * fVar5;
  param_1[2] = (fVar4 * param_2[2] + fVar3 * param_3[2] + param_6 * param_4[2] + fVar2 * param_5[2])
               * fVar5;
  param_1[3] = (fVar4 * param_2[3] + fVar3 * param_3[3] + param_6 * param_4[3] + fVar2 * param_5[3])
               * fVar5;
  return;
}

