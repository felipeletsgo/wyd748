// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0dd1 | Name: FUN_013c0dd1


int FUN_013c0dd1(void)

{
  int unaff_EBP;
  
  if (DAT_013c1f8c != 0) {
    return DAT_013c1f14;
  }
  return (int)*(short *)(*(int *)(unaff_EBP + -0x1f8) + 0x4c);
}

