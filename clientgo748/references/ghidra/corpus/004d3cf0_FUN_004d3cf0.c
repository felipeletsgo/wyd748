// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d3cf0 | Name: FUN_004d3cf0


float10 FUN_004d3cf0(float *param_1,float *param_2)

{
  return (float10)param_1[2] * (float10)param_2[2] +
         (float10)param_1[1] * (float10)param_2[1] + (float10)*param_1 * (float10)*param_2;
}

