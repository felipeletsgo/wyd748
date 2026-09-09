// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00561095 | Name: FUN_00561095


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00561095(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5,
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
  *param_1 = ((fVar2 - fVar1) * *param_5 + param_6 * *param_4 + fVar3 * *param_3 + fVar4 * *param_2)
             * _DAT_005a67d4;
  param_1[1] = (fVar4 * param_2[1] +
               fVar3 * param_3[1] + param_6 * param_4[1] + (fVar2 - fVar1) * param_5[1]) * fVar5;
  return;
}

