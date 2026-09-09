// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00493975 | Name: FUN_00493975


void FUN_00493975(void)

{
  int in_EAX;
  int unaff_EBP;
  
  *(undefined1 *)(in_EAX + 0xef5) = *(undefined1 *)(unaff_EBP + -0x10);
  FUN_0049bcb8(unaff_EBP + -0x38);
  FUN_004595ec(*(undefined4 *)(unaff_EBP + -0x20),*(undefined4 *)(unaff_EBP + -0x24),
               *(undefined4 *)(unaff_EBP + -0x38),*(undefined4 *)(unaff_EBP + -0x34),
               *(undefined4 *)(unaff_EBP + -0x30),*(undefined4 *)(unaff_EBP + -8),1,0);
  FUN_00493a9d();
  return;
}

