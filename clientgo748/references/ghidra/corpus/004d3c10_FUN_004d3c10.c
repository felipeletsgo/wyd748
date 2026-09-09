// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d3c10 | Name: FUN_004d3c10


float * __thiscall FUN_004d3c10(float *param_1,uint param_2)

{
  *param_1 = (float)(param_2 >> 0x10 & 0xff) * 0.003921569;
  param_1[1] = (float)(param_2 >> 8 & 0xff) * 0.003921569;
  param_1[2] = (float)(param_2 & 0xff) * 0.003921569;
  param_1[3] = (float)(param_2 >> 0x18) * 0.003921569;
  return param_1;
}

