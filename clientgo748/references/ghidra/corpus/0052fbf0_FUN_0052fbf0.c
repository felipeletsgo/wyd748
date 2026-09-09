// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fbf0 | Name: FUN_0052fbf0


float * __thiscall FUN_0052fbf0(float *param_1,float *param_2)

{
  *param_1 = *param_1 + *param_2;
  param_1[1] = param_1[1] + param_2[1];
  return param_1;
}

