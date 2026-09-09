// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595c0c | Name: FUN_00595c0c


int * FUN_00595c0c(uint *param_1)

{
  char *pcVar1;
  int *piVar2;
  char cVar3;
  int *piVar4;
  undefined4 uVar5;
  byte bVar6;
  uint uVar7;
  int iVar8;
  uint *puVar9;
  int iVar10;
  int *piVar11;
  uint *puVar12;
  uint *puVar13;
  uint uVar14;
  int iVar15;
  uint local_10;
  uint local_c;
  int local_8;
  
  puVar9 = DAT_013b85fc + DAT_013b85f8 * 5;
  uVar7 = (int)param_1 + 0x17U & 0xfffffff0;
  iVar8 = ((int)((int)param_1 + 0x17U) >> 4) + -1;
  bVar6 = (byte)iVar8;
  param_1 = DAT_013b85f0;
  if (iVar8 < 0x20) {
    local_10 = 0xffffffff >> (bVar6 & 0x1f);
    local_c = 0xffffffff;
  }
  else {
    local_c = 0xffffffff >> (bVar6 - 0x20 & 0x1f);
    local_10 = 0;
  }
  for (; (param_1 < puVar9 && ((param_1[1] & local_c) == 0 && (*param_1 & local_10) == 0));
      param_1 = param_1 + 5) {
  }
  puVar12 = DAT_013b85fc;
  if (param_1 == puVar9) {
    for (; (puVar12 < DAT_013b85f0 && ((puVar12[1] & local_c) == 0 && (*puVar12 & local_10) == 0));
        puVar12 = puVar12 + 5) {
    }
    param_1 = puVar12;
    if (puVar12 == DAT_013b85f0) {
      for (; (puVar12 < puVar9 && (puVar12[2] == 0)); puVar12 = puVar12 + 5) {
      }
      puVar13 = DAT_013b85fc;
      param_1 = puVar12;
      if (puVar12 == puVar9) {
        for (; (puVar13 < DAT_013b85f0 && (puVar13[2] == 0)); puVar13 = puVar13 + 5) {
        }
        param_1 = puVar13;
        if ((puVar13 == DAT_013b85f0) && (param_1 = (uint *)FUN_00595f15(), param_1 == (uint *)0x0))
        {
          return (int *)0x0;
        }
      }
      uVar5 = FUN_00595fc6(param_1);
      *(undefined4 *)param_1[4] = uVar5;
      if (*(int *)param_1[4] == -1) {
        return (int *)0x0;
      }
    }
  }
  piVar4 = (int *)param_1[4];
  local_8 = *piVar4;
  if ((local_8 == -1) ||
     ((piVar4[local_8 + 0x31] & local_c) == 0 && (piVar4[local_8 + 0x11] & local_10) == 0)) {
    local_8 = 0;
    puVar9 = (uint *)(piVar4 + 0x11);
    if ((piVar4[0x31] & local_c) == 0 && (piVar4[0x11] & local_10) == 0) {
      do {
        puVar12 = puVar9 + 0x21;
        local_8 = local_8 + 1;
        puVar9 = puVar9 + 1;
      } while ((*puVar12 & local_c) == 0 && (local_10 & *puVar9) == 0);
    }
  }
  iVar8 = 0;
  piVar2 = piVar4 + local_8 * 0x81 + 0x51;
  local_10 = piVar4[local_8 + 0x11] & local_10;
  if (local_10 == 0) {
    local_10 = piVar4[local_8 + 0x31] & local_c;
    iVar8 = 0x20;
  }
  for (; -1 < (int)local_10; local_10 = local_10 << 1) {
    iVar8 = iVar8 + 1;
  }
  piVar11 = (int *)piVar2[iVar8 * 2 + 1];
  iVar10 = *piVar11 - uVar7;
  iVar15 = (iVar10 >> 4) + -1;
  if (0x3f < iVar15) {
    iVar15 = 0x3f;
  }
  DAT_013b85f0 = param_1;
  if (iVar15 != iVar8) {
    if (piVar11[1] == piVar11[2]) {
      if (iVar8 < 0x20) {
        pcVar1 = (char *)((int)piVar4 + iVar8 + 4);
        uVar14 = ~(0x80000000U >> ((byte)iVar8 & 0x1f));
        piVar4[local_8 + 0x11] = uVar14 & piVar4[local_8 + 0x11];
        *pcVar1 = *pcVar1 + -1;
        if (*pcVar1 == '\0') {
          *param_1 = *param_1 & uVar14;
        }
      }
      else {
        pcVar1 = (char *)((int)piVar4 + iVar8 + 4);
        uVar14 = ~(0x80000000U >> ((byte)iVar8 - 0x20 & 0x1f));
        piVar4[local_8 + 0x31] = piVar4[local_8 + 0x31] & uVar14;
        *pcVar1 = *pcVar1 + -1;
        if (*pcVar1 == '\0') {
          param_1[1] = param_1[1] & uVar14;
        }
      }
    }
    *(int *)(piVar11[2] + 4) = piVar11[1];
    *(int *)(piVar11[1] + 8) = piVar11[2];
    if (iVar10 == 0) goto LAB_00595ed2;
    piVar11[1] = piVar2[iVar15 * 2 + 1];
    piVar11[2] = (int)(piVar2 + iVar15 * 2);
    (piVar2 + iVar15 * 2)[1] = (int)piVar11;
    *(int **)(piVar11[1] + 8) = piVar11;
    if (piVar11[1] == piVar11[2]) {
      cVar3 = *(char *)(iVar15 + 4 + (int)piVar4);
      bVar6 = (byte)iVar15;
      if (iVar15 < 0x20) {
        *(char *)(iVar15 + 4 + (int)piVar4) = cVar3 + '\x01';
        if (cVar3 == '\0') {
          *param_1 = *param_1 | 0x80000000U >> (bVar6 & 0x1f);
        }
        piVar4[local_8 + 0x11] = piVar4[local_8 + 0x11] | 0x80000000U >> (bVar6 & 0x1f);
      }
      else {
        *(char *)(iVar15 + 4 + (int)piVar4) = cVar3 + '\x01';
        if (cVar3 == '\0') {
          param_1[1] = param_1[1] | 0x80000000U >> (bVar6 - 0x20 & 0x1f);
        }
        piVar4[local_8 + 0x31] = piVar4[local_8 + 0x31] | 0x80000000U >> (bVar6 - 0x20 & 0x1f);
      }
    }
  }
  if (iVar10 != 0) {
    *piVar11 = iVar10;
    *(int *)(iVar10 + -4 + (int)piVar11) = iVar10;
  }
LAB_00595ed2:
  piVar11 = (int *)((int)piVar11 + iVar10);
  *piVar11 = uVar7 + 1;
  *(uint *)((int)piVar11 + (uVar7 - 4)) = uVar7 + 1;
  iVar8 = *piVar2;
  *piVar2 = iVar8 + 1;
  if (((iVar8 == 0) && (param_1 == DAT_013b85f4)) && (local_8 == DAT_013b85ec)) {
    DAT_013b85f4 = (uint *)0x0;
  }
  *piVar4 = local_8;
  return piVar11 + 1;
}

