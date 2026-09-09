// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00493db0 | Name: FUN_00493db0


float * __thiscall FUN_00493db0(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
  CSimpleArray<>();
  fVar1 = param_1[1];
  fVar2 = param_3[1];
  fVar3 = param_1[2];
  fVar4 = param_3[2];
  *param_2 = *param_1 - *param_3;
  param_2[1] = fVar1 - fVar2;
  param_2[2] = fVar3 - fVar4;
  return param_2;
}

