// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005a0499 | Name: Unwind@005a0499


void Unwind_005a0499(void)

{
  int unaff_EBP;
  
  if ((*(uint *)(unaff_EBP + -0x28) & 1) != 0) {
    FUN_004b6310();
  }
  return;
}

