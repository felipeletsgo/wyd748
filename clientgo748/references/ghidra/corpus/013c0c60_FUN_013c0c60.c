// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0c60 | Name: FUN_013c0c60


undefined4 FUN_013c0c60(void)

{
  int unaff_EBP;
  
  if (DAT_013c1f8c != 0) {
    return DAT_013c1f00;
  }
  return *(undefined4 *)(unaff_EBP + -0x458);
}

