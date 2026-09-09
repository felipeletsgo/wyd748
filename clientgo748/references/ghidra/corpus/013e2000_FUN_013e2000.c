// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013e2000 | Name: FUN_013e2000


undefined4 FUN_013e2000(void)

{
  int iVar1;
  undefined4 uVar2;
  int unaff_EBP;
  
  iVar1 = *(int *)(unaff_EBP + 8);
  if (iVar1 == 0x42) {
    *(undefined4 *)(unaff_EBP + -4) = 0x2d;
  }
  else if (iVar1 == 0x43) {
    *(undefined4 *)(unaff_EBP + -4) = 0x2e;
  }
  else if (iVar1 == 0x44) {
    *(undefined4 *)(unaff_EBP + -4) = 0x2f;
  }
  else if (iVar1 == 0x45) {
    *(undefined4 *)(unaff_EBP + -4) = 0x30;
  }
  else if (iVar1 == 0x46) {
    *(undefined4 *)(unaff_EBP + -4) = 0x35;
  }
  else if (iVar1 == 0x47) {
    *(undefined4 *)(unaff_EBP + -4) = 0x36;
  }
  else if (iVar1 == 0x48) {
    *(undefined4 *)(unaff_EBP + -4) = 0x37;
  }
  else if (iVar1 == 0x49) {
    *(undefined4 *)(unaff_EBP + -4) = 0x38;
  }
  else {
    if (iVar1 != 0x4a) {
      *(undefined4 *)(unaff_EBP + -4) = 0;
      uVar2 = FUN_004faeff();
      return uVar2;
    }
    *(undefined4 *)(unaff_EBP + -4) = 0x39;
  }
  return *(undefined4 *)(unaff_EBP + -4);
}

