// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598fe8 | Name: FUN_00598fe8


int * FUN_00598fe8(int *param_1)

{
  bool bVar1;
  int iVar2;
  void *pvVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  
  iVar4 = *param_1;
  bVar1 = false;
  iVar2 = FUN_00593d53();
  if (iVar4 < 0) {
    return (int *)0x0;
  }
  if (*(int *)(iVar2 + 0x40) == 0) {
    pvVar3 = _malloc(0x24);
    *(void **)(iVar2 + 0x40) = pvVar3;
    piVar5 = (int *)&DAT_013b8178;
    if (pvVar3 == (void *)0x0) goto LAB_00599022;
  }
  piVar5 = *(int **)(iVar2 + 0x40);
LAB_00599022:
  iVar7 = iVar4 % 0x7861f80;
  iVar4 = (iVar4 / 0x7861f80) * 4;
  iVar2 = iVar4 + 0x46;
  iVar8 = iVar7;
  if (0x1e1337f < iVar7) {
    iVar8 = iVar7 + -0x1e13380;
    iVar2 = iVar4 + 0x47;
    if (0x1e1337f < iVar8) {
      iVar8 = iVar7 + -0x3c26700;
      iVar2 = iVar4 + 0x48;
      if (iVar8 < 0x1e28500) {
        bVar1 = true;
      }
      else {
        iVar2 = iVar4 + 0x49;
        iVar8 = iVar7 + -0x5a4ec00;
      }
    }
  }
  piVar5[5] = iVar2;
  piVar9 = (int *)&DAT_005cc720;
  piVar5[7] = iVar8 / 0x15180;
  if (!bVar1) {
    piVar9 = (int *)&DAT_005cc754;
  }
  iVar4 = 1;
  piVar6 = piVar9;
  while (piVar6 = piVar6 + 1, *piVar6 < piVar5[7]) {
    iVar4 = iVar4 + 1;
  }
  piVar5[4] = iVar4 + -1;
  piVar5[3] = piVar5[7] - piVar9[iVar4 + -1];
  piVar5[6] = (*param_1 / 0x15180 + 4) % 7;
  piVar5[2] = (iVar8 % 0x15180) / 0xe10;
  iVar4 = (iVar8 % 0x15180) % 0xe10;
  piVar5[1] = iVar4 / 0x3c;
  piVar5[8] = 0;
  *piVar5 = iVar4 % 0x3c;
  return piVar5;
}

