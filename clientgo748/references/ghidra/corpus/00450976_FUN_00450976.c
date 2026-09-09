// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00450976 | Name: FUN_00450976


void __fastcall FUN_00450976(undefined4 param_1,int param_2)

{
  int unaff_EBP;
  
  if ((param_2 == 1) &&
     ((((*(int *)(*(int *)(*(int *)(unaff_EBP + -4) + 0x4c) + 0x7a0) == 0x1f ||
        (*(int *)(*(int *)(*(int *)(unaff_EBP + -4) + 0x4c) + 0x7a0) == 0x28)) ||
       ((*(int *)(*(int *)(*(int *)(unaff_EBP + -4) + 0x4c) + 0x7a0) == 0x14 &&
        (*(short *)(*(int *)(*(int *)(unaff_EBP + -4) + 0x4c) + 0x1a2) != 7)))) ||
      (*(int *)(*(int *)(*(int *)(unaff_EBP + -4) + 0x4c) + 0x7a0) == 0x27)))) {
    DAT_005d0408 = (uint)(DAT_005d0408 == 0);
    FUN_004431e4(0);
  }
  return;
}

