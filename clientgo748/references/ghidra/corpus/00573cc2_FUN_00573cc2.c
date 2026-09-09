// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573cc2 | Name: FUN_00573cc2


undefined4 FUN_00573cc2(int param_1,int param_2,double *param_3)

{
  if ((((param_1 != 0) && (param_2 != 0)) && ((*(byte *)(param_2 + 8) & 1) != 0)) &&
     (param_3 != (double *)0x0)) {
    *param_3 = (double)*(float *)(param_2 + 0x28);
    return 1;
  }
  return 0;
}

