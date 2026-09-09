// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b71d3 | Name: Catch@004b71d3


code * Catch_004b71d3(void)

{
  undefined4 uVar1;
  int unaff_EBP;
  
  *(undefined4 *)(unaff_EBP + -0x1c) = *(undefined4 *)(unaff_EBP + 8);
  uVar1 = FUN_004b73c0(*(int *)(unaff_EBP + -0x1c) + 2,0);
  *(undefined4 *)(unaff_EBP + -0x18) = uVar1;
  return FUN_004b71f3;
}

