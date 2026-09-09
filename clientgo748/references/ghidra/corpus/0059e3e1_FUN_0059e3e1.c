// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e3e1 | Name: FUN_0059e3e1


undefined4 * FUN_0059e3e1(void)

{
  undefined1 uVar1;
  int iVar2;
  undefined4 *extraout_ECX;
  int unaff_EBP;
  
  FUN_00592900();
  iVar2 = *(int *)(unaff_EBP + 8);
  *(undefined4 **)(unaff_EBP + -0x10) = extraout_ECX;
  FUN_0059e78b(iVar2);
  uVar1 = *(undefined1 *)(iVar2 + 0xc);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *(undefined1 *)(extraout_ECX + 3) = uVar1;
  FUN_004b6980(0);
  FUN_004b67e0(iVar2 + 0xc,0,DAT_005a467c);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  *extraout_ECX = &PTR_FUN_005aa85c;
  return extraout_ECX;
}

