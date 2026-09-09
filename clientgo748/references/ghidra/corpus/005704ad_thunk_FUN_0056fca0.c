// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005704ad | Name: thunk_FUN_0056fca0


void thunk_FUN_0056fca0(void)

{
  undefined4 *extraout_ECX;
  int unaff_EBP;
  
  FUN_00592900();
  *(undefined4 **)(unaff_EBP + -0x10) = extraout_ECX;
  *extraout_ECX = &PTR_LAB_005a69d0;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  FUN_0056ed49();
  if (extraout_ECX[0x41b] != 0) {
    FUN_0058ea3a(extraout_ECX[0x41b]);
  }
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  FUN_0056b13c();
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

