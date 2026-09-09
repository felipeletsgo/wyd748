// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e2fa | Name: FUN_0059e2fa


undefined4 * FUN_0059e2fa(void)

{
  undefined1 *puVar1;
  undefined4 *extraout_ECX;
  int unaff_EBP;
  
  FUN_00592900();
  *(undefined4 **)(unaff_EBP + -0x14) = extraout_ECX;
  *(undefined **)(unaff_EBP + -0x10) = &DAT_005aa868;
  FUN_0059e74e(unaff_EBP + -0x10);
  puVar1 = *(undefined1 **)(unaff_EBP + 8);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *(undefined1 *)(extraout_ECX + 3) = *puVar1;
  FUN_004b6980(0);
  FUN_004b67e0(puVar1,0,DAT_005a467c);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  *extraout_ECX = &PTR_FUN_005aa85c;
  return extraout_ECX;
}

