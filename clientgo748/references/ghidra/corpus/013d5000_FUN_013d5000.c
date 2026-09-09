// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d5000 | Name: FUN_013d5000


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_013d5000(void)

{
  int *piVar1;
  int iVar2;
  int unaff_EBP;
  
  piVar1 = *(int **)(unaff_EBP + -0x214);
  iVar2 = *piVar1;
  if ((iVar2 < 0x2d) || (0x39 < iVar2)) {
    iVar2 = *(int *)(unaff_EBP + -0x214);
    thunk_FUN_00562821(unaff_EBP + -0x140,*(float *)(iVar2 + 0x1c) - _DAT_005a4380,
                       *(float *)(iVar2 + 0x18) - _DAT_005a4380,*(undefined4 *)(iVar2 + 0x20));
  }
  else if (((iVar2 != 0x2d) || (piVar1[0xb4] < 7)) || (9 < piVar1[0xb4])) {
    if (iVar2 == 0x2f) {
      piVar1[9] = 0x3fc00000;
      piVar1[10] = 0x3fc00000;
      piVar1[0xb] = 0x3fc00000;
    }
    thunk_FUN_00562821(unaff_EBP + -0x140,(float)piVar1[7] + _DAT_005a4380,piVar1[6],piVar1[8]);
  }
  FUN_004be3cb();
  return;
}

