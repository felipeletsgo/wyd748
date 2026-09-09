// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fdab | Name: FUN_0055fdab


int FUN_0055fdab(void)

{
  int unaff_EBP;
  int iVar1;
  
  FUN_00592900();
  FUN_00564a94();
  *(undefined4 *)(unaff_EBP + -4) = 0;
  FUN_0056a92f();
  *(undefined1 *)(unaff_EBP + -4) = 1;
  if ((*(int *)(unaff_EBP + 8) == 0) || (*(int *)(unaff_EBP + 0x10) == 0)) {
    iVar1 = -0x7789f794;
  }
  else {
    iVar1 = FUN_0056a9c9(unaff_EBP + -0xbc,*(undefined4 *)(unaff_EBP + 0x10),
                         *(undefined4 *)(unaff_EBP + 0x14),*(undefined4 *)(unaff_EBP + 0x18),0,1);
    if (((-1 < iVar1) && (iVar1 = FUN_00564b22(unaff_EBP + -0xbc), -1 < iVar1)) &&
       (iVar1 = FUN_00566b8c(*(undefined4 *)(unaff_EBP + 8),*(undefined4 *)(unaff_EBP + 0xc),
                             *(undefined4 *)(unaff_EBP + 0x1c)), -1 < iVar1)) {
      iVar1 = 0;
    }
  }
  *(undefined1 *)(unaff_EBP + -4) = 0;
  FUN_0056a9c4();
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  FUN_00564aad();
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar1;
}

