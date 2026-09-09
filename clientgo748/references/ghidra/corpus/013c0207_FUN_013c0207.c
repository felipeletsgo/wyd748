// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0207 | Name: FUN_013c0207


void __fastcall FUN_013c0207(uint param_1)

{
  int iVar1;
  int unaff_EBP;
  
  iVar1 = *(int *)(unaff_EBP + -8);
  if ((*(byte *)((int)&PTR_DAT_013c0237 + ((int)param_1 >> 3)) >> (param_1 & 7) & 1) != 0) {
    iVar1 = (iVar1 - *(int *)(unaff_EBP + -0x40)) + 150000;
  }
  *(int *)(&DAT_0092eaf8 + param_1 * 4) = iVar1;
  FUN_00493975();
  return;
}

