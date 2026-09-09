// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c8c60 | Name: FUN_004c8c60


float * __thiscall FUN_004c8c60(float *param_1,float *param_2,float param_3)

{
  float fVar1;
  float fVar2;
  
  CSimpleArray<>();
  fVar1 = param_1[1];
  fVar2 = param_1[2];
  *param_2 = param_3 * *param_1;
  param_2[1] = param_3 * fVar1;
  param_2[2] = param_3 * fVar2;
  return param_2;
}

