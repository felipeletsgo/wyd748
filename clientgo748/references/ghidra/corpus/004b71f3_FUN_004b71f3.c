// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b71f3 | Name: FUN_004b71f3


void FUN_004b71f3(void)

{
  undefined1 *puVar1;
  int unaff_EBP;
  
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  if (*(int *)(*(int *)(unaff_EBP + -0x20) + 8) != 0) {
    if (*(uint *)(unaff_EBP + -0x1c) < *(uint *)(*(int *)(unaff_EBP + -0x20) + 8)) {
      *(undefined4 *)(unaff_EBP + -0x28) = *(undefined4 *)(unaff_EBP + -0x1c);
    }
    else {
      *(undefined4 *)(unaff_EBP + -0x28) = *(undefined4 *)(*(int *)(unaff_EBP + -0x20) + 8);
    }
    FUN_004b66f0(*(int *)(unaff_EBP + -0x18) + 1,*(undefined4 *)(*(int *)(unaff_EBP + -0x20) + 4),
                 *(undefined4 *)(unaff_EBP + -0x28));
  }
  *(undefined4 *)(unaff_EBP + -0x14) = *(undefined4 *)(*(int *)(unaff_EBP + -0x20) + 8);
  FUN_004b6980(1);
  *(int *)(*(int *)(unaff_EBP + -0x20) + 4) = *(int *)(unaff_EBP + -0x18) + 1;
  puVar1 = (undefined1 *)FUN_004b6eb0(*(undefined4 *)(*(int *)(unaff_EBP + -0x20) + 4));
  *puVar1 = 0;
  *(undefined4 *)(*(int *)(unaff_EBP + -0x20) + 0xc) = *(undefined4 *)(unaff_EBP + -0x1c);
  if (*(uint *)(unaff_EBP + -0x1c) < *(uint *)(unaff_EBP + -0x14)) {
    *(undefined4 *)(unaff_EBP + -0x2c) = *(undefined4 *)(unaff_EBP + -0x1c);
  }
  else {
    *(undefined4 *)(unaff_EBP + -0x2c) = *(undefined4 *)(unaff_EBP + -0x14);
  }
  FUN_004b6d20(*(undefined4 *)(unaff_EBP + -0x2c));
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

