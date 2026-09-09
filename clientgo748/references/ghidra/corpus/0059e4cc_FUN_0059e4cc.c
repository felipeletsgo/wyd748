// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e4cc | Name: FUN_0059e4cc


void FUN_0059e4cc(void)

{
  size_t sVar1;
  int unaff_EBP;
  
  FUN_00592900();
  *(undefined1 *)(unaff_EBP + -0x20) = *(undefined1 *)(unaff_EBP + -0xd);
  FUN_004b6980(0);
  sVar1 = _strlen("invalid string position");
  FUN_004b6be0("invalid string position",sVar1);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  FUN_0059e2fa(unaff_EBP + -0x20);
  *(undefined ***)(unaff_EBP + -0x3c) = &PTR_LAB_005aa870;
                    /* WARNING: Subroutine does not return */
  __CxxThrowException_8(unaff_EBP + -0x3c,&DAT_005af730);
}

