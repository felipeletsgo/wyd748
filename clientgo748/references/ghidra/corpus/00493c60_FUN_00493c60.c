// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00493c60 | Name: FUN_00493c60


float * __thiscall FUN_00493c60(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  
  FUN_00430f10();
  fVar1 = param_1[1];
  fVar2 = param_3[1];
  *param_2 = *param_1 - *param_3;
  param_2[1] = fVar1 - fVar2;
  return param_2;
}

