// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00493e10 | Name: FUN_00493e10


float * __thiscall FUN_00493e10(float *param_1,float *param_2,float param_3)

{
  float fVar1;
  float fVar2;
  
  CSimpleArray<>();
  fVar1 = param_1[1];
  fVar2 = param_1[2];
  *param_2 = *param_1 / param_3;
  param_2[1] = fVar1 / param_3;
  param_2[2] = fVar2 / param_3;
  return param_2;
}

