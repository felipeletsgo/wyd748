// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0f8f | Name: FUN_013c0f8f


int FUN_013c0f8f(void)

{
  int unaff_EBP;
  
  if (DAT_013c1f8c != 0) {
    return DAT_013c1f04;
  }
  return (int)*(short *)(*(int *)(unaff_EBP + -0x1f8) + 0x44);
}

