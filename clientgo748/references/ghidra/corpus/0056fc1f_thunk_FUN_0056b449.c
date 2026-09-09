// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056fc1f | Name: thunk_FUN_0056b449


void thunk_FUN_0056b449(void)

{
  undefined4 *extraout_ECX;
  uint uVar1;
  int unaff_EBP;
  uint uVar2;
  
  FUN_00592900();
  *(undefined4 **)(unaff_EBP + -0x14) = extraout_ECX;
  *extraout_ECX = &PTR_FUN_005a695c;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if ((extraout_ECX[0x42f] != 0) && (extraout_ECX[0x430] != 0)) {
    uVar1 = extraout_ECX[0x427];
    uVar2 = extraout_ECX[0x428];
    *(undefined4 *)(unaff_EBP + -0x10) = extraout_ECX[0x430];
    if (uVar1 < uVar2) {
      do {
        for (uVar2 = extraout_ECX[0x424]; uVar2 < (uint)extraout_ECX[0x426]; uVar2 = uVar2 + 4) {
          FUN_0058ea3a(*(undefined4 *)(*(int *)(unaff_EBP + -0x10) + 4));
          *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 8;
        }
        uVar1 = uVar1 + 1;
      } while (uVar1 < (uint)extraout_ECX[0x428]);
    }
  }
  FUN_0058ea3a(extraout_ECX[0x42e]);
  FUN_0058ea3a(extraout_ECX[0x430]);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  FUN_0056b13c();
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

