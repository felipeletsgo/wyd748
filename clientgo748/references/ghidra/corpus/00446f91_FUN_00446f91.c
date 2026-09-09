// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00446f91 | Name: FUN_00446f91


void __fastcall FUN_00446f91(undefined4 param_1,int param_2)

{
  int iVar1;
  int unaff_EBP;
  
  if ((param_2 == 1) &&
     ((((*(int *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x4c) + 0x7a0) == 0x1f ||
        (*(int *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x4c) + 0x7a0) == 0x28)) ||
       ((*(int *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x4c) + 0x7a0) == 0x14 &&
        (*(short *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x4c) + 0x1a2) != 7)))) ||
      (*(int *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x4c) + 0x7a0) == 0x27)))) {
    if (*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) != 0) {
      *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) + 0x2c) = 1;
      *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) + 0x1e8) = DAT_005d0408;
    }
  }
  else {
    if (*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) != 0) {
      *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) + 0x2c) = 0;
    }
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 1) {
      (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x5b0) + 0x27918) + 0x60))(0);
    }
  }
  FUN_0044eca3();
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

