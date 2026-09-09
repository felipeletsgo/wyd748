// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a1a7 | Name: FUN_0059a1a7


undefined4 FUN_0059a1a7(int param_1)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  code *pcVar5;
  undefined4 *puVar6;
  undefined4 local_10;
  undefined4 local_c;
  
  bVar1 = false;
  if (param_1 == 2) {
    puVar6 = &DAT_013b82ac;
    pcVar5 = DAT_013b82ac;
LAB_0059a22d:
    bVar1 = true;
    FUN_00596aad(1);
    iVar2 = param_1;
  }
  else {
    if (((param_1 != 4) && (param_1 != 8)) && (param_1 != 0xb)) {
      if (param_1 == 0xf) {
        puVar6 = &DAT_013b82b8;
        pcVar5 = DAT_013b82b8;
      }
      else if (param_1 == 0x15) {
        puVar6 = &DAT_013b82b0;
        pcVar5 = DAT_013b82b0;
      }
      else {
        if (param_1 != 0x16) {
          return 0xffffffff;
        }
        puVar6 = &DAT_013b82b4;
        pcVar5 = DAT_013b82b4;
      }
      goto LAB_0059a22d;
    }
    iVar2 = FUN_00593d53();
    iVar4 = FUN_0059a329(param_1,*(undefined4 *)(iVar2 + 0x50));
    puVar6 = (undefined4 *)(iVar4 + 8);
    pcVar5 = (code *)*puVar6;
  }
  if (pcVar5 == (code *)0x1) {
    if (!bVar1) {
      return 0;
    }
    FUN_00596b0e(1);
    return 0;
  }
  if (pcVar5 == (code *)0x0) {
    if (bVar1) {
      FUN_00596b0e(1);
    }
                    /* WARNING: Subroutine does not return */
    __exit(3);
  }
  if (((param_1 == 8) || (param_1 == 0xb)) || (param_1 == 4)) {
    local_c = *(undefined4 *)(iVar2 + 0x54);
    *(undefined4 *)(iVar2 + 0x54) = 0;
    if (param_1 == 8) {
      local_10 = *(undefined4 *)(iVar2 + 0x58);
      *(undefined4 *)(iVar2 + 0x58) = 0x8c;
      goto LAB_0059a2a1;
    }
  }
  else {
LAB_0059a2a1:
    if (param_1 == 8) {
      if (DAT_005cc0a8 < DAT_005cc0ac + DAT_005cc0a8) {
        iVar3 = DAT_005cc0a8 * 0xc;
        iVar4 = DAT_005cc0a8;
        do {
          iVar3 = iVar3 + 0xc;
          *(undefined4 *)(*(int *)(iVar2 + 0x50) + -4 + iVar3) = 0;
          iVar4 = iVar4 + 1;
        } while (iVar4 < DAT_005cc0ac + DAT_005cc0a8);
      }
      goto LAB_0059a2df;
    }
  }
  *puVar6 = 0;
LAB_0059a2df:
  if (bVar1) {
    FUN_00596b0e(1);
  }
  if (param_1 == 8) {
    (*pcVar5)(8,*(undefined4 *)(iVar2 + 0x58));
  }
  else {
    (*pcVar5)(param_1);
    if ((param_1 != 0xb) && (param_1 != 4)) {
      return 0;
    }
  }
  *(undefined4 *)(iVar2 + 0x54) = local_c;
  if (param_1 == 8) {
    *(undefined4 *)(iVar2 + 0x58) = local_10;
  }
  return 0;
}

