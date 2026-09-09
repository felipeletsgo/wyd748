// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c3642 | Name: FUN_004c3642


void FUN_004c3642(void)

{
  int unaff_EBP;
  
  if (*(int *)(*(int *)(unaff_EBP + -0x200) + 0x8c) != 0) {
    FUN_004c21e8(*(int *)(unaff_EBP + -0x200) + 0x48);
  }
  if (*(int *)(*(int *)(unaff_EBP + -0x200) + 0x88) != 0) {
    FUN_004c21e8(*(undefined4 *)(unaff_EBP + 8));
  }
  return;
}

