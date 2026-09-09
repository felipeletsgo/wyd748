// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e2a0 | Name: FUN_0059e2a0


void FUN_0059e2a0(void)

{
  size_t sVar1;
  int unaff_EBP;
  
  FUN_00592900();
  *(undefined1 *)(unaff_EBP + -0x20) = *(undefined1 *)(unaff_EBP + -0xd);
  FUN_004b6980(0);
  sVar1 = _strlen("string too long");
  FUN_004b6be0("string too long",sVar1);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  FUN_0059e2fa(unaff_EBP + -0x20);
  *(undefined ***)(unaff_EBP + -0x3c) = &PTR_LAB_005aa83c;
                    /* WARNING: Subroutine does not return */
  __CxxThrowException_8(unaff_EBP + -0x3c,&DAT_005af608);
}

