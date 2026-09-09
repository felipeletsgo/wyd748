// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489145 | Name: FUN_00489145


undefined4 FUN_00489145(int param_1)

{
  DAT_005d03f8 = *(int *)(param_1 + 0xc);
  if (*(int *)(param_1 + 0xc) == 0) {
    DAT_005b2980 = 0xffffffff;
  }
  DAT_005b2984 = 0xffffffff;
  return 1;
}

