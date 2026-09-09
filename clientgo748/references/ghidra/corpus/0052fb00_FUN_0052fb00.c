// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fb00 | Name: FUN_0052fb00


float * __thiscall FUN_0052fb00(float *param_1,float *param_2,float param_3)

{
  float fVar1;
  
  FUN_00430f10();
  fVar1 = param_1[1];
  *param_2 = param_3 * *param_1;
  param_2[1] = param_3 * fVar1;
  return param_2;
}

