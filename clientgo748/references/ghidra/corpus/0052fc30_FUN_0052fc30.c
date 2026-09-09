// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fc30 | Name: FUN_0052fc30


float * __thiscall FUN_0052fc30(float *param_1,float *param_2,float param_3)

{
  float fVar1;
  
  FUN_00430f10();
  fVar1 = param_1[1];
  *param_2 = *param_1 / param_3;
  param_2[1] = fVar1 / param_3;
  return param_2;
}

