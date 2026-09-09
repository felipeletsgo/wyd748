// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574b8a | Name: FUN_00574b8a


void FUN_00574b8a(int *param_1,int param_2,int param_3)

{
  char *pcVar1;
  int iVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  byte *pbVar6;
  byte *pbVar7;
  int iVar8;
  ushort uVar9;
  undefined1 *puVar10;
  undefined1 *puVar11;
  
  iVar2 = *param_1;
  if ((char)param_1[2] == '\0') {
    uVar9 = 0;
    if (param_3 != 0) {
      uVar9 = *(ushort *)(param_3 + 8);
    }
    bVar3 = *(byte *)((int)param_1 + 9);
    if (bVar3 < 8) {
      if (bVar3 == 1) {
        uVar9 = uVar9 * 0xff;
        pcVar1 = (char *)(iVar2 + -1 + param_2);
        iVar5 = 7 - (iVar2 - 1U & 7);
        pbVar6 = (byte *)((iVar2 - 1U >> 3) + param_2);
        for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
          *pcVar1 = -((*pbVar6 >> ((byte)iVar5 & 0x1f) & 1) != 0);
          if (iVar5 == 7) {
            iVar5 = 0;
            pbVar6 = pbVar6 + -1;
          }
          else {
            iVar5 = iVar5 + 1;
          }
          pcVar1 = pcVar1 + -1;
        }
      }
      else if (bVar3 == 2) {
        uVar9 = uVar9 * 0x55;
        pbVar6 = (byte *)(iVar2 + -1 + param_2);
        iVar5 = (3 - (iVar2 - 1U & 3)) * 2;
        pbVar7 = (byte *)((iVar2 - 1U >> 2) + param_2);
        for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
          bVar3 = *pbVar7 >> ((byte)iVar5 & 0x1f) & 3;
          *pbVar6 = ((bVar3 << 2 | bVar3) << 2 | bVar3) << 2 | bVar3;
          if (iVar5 == 6) {
            iVar5 = 0;
            pbVar7 = pbVar7 + -1;
          }
          else {
            iVar5 = iVar5 + 2;
          }
          pbVar6 = pbVar6 + -1;
        }
      }
      else if (bVar3 == 4) {
        uVar9 = uVar9 * 0x11;
        pbVar6 = (byte *)(iVar2 + -1 + param_2);
        pbVar7 = (byte *)((iVar2 - 1U >> 1) + param_2);
        iVar5 = (iVar2 - 1U & 1) * -4 + 4;
        for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
          bVar3 = *pbVar7 >> ((byte)iVar5 & 0x1f);
          *pbVar6 = bVar3 << 4 | bVar3 & 0xf;
          iVar4 = 4;
          if (iVar5 == 4) {
            iVar4 = 0;
            pbVar7 = pbVar7 + -1;
          }
          pbVar6 = pbVar6 + -1;
          iVar5 = iVar4;
        }
      }
      *(undefined1 *)((int)param_1 + 9) = 8;
      *(undefined1 *)((int)param_1 + 0xb) = 8;
      param_1[1] = iVar2;
    }
    if (param_3 == 0) {
      return;
    }
    if (*(char *)((int)param_1 + 9) == '\b') {
      pbVar6 = (byte *)(iVar2 + -1 + param_2);
      puVar10 = (undefined1 *)(param_2 + -1 + iVar2 * 2);
      for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
        if (*pbVar6 == uVar9) {
          *puVar10 = 0;
        }
        else {
          *puVar10 = 0xff;
        }
        puVar10[-1] = *pbVar6;
        puVar10 = puVar10 + -2;
        pbVar6 = pbVar6 + -1;
      }
    }
    else if (*(char *)((int)param_1 + 9) == '\x10') {
      puVar10 = (undefined1 *)(param_1[1] + -1 + param_2);
      puVar11 = (undefined1 *)(param_2 + -1 + param_1[1] * 2);
      for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
        if (CONCAT11(puVar10[-1],*puVar10) == uVar9) {
          *puVar11 = 0;
          puVar11[-1] = 0;
        }
        else {
          *puVar11 = 0xff;
          puVar11[-1] = 0xff;
        }
        puVar11[-2] = *puVar10;
        puVar11[-3] = puVar10[-1];
        puVar11 = puVar11 + -4;
        puVar10 = puVar10 + -2;
      }
    }
    *(undefined1 *)(param_1 + 2) = 4;
    *(undefined1 *)((int)param_1 + 10) = 2;
    bVar3 = *(char *)((int)param_1 + 9) << 1;
  }
  else {
    if ((char)param_1[2] != '\x02') {
      return;
    }
    if (param_3 == 0) {
      return;
    }
    if (*(char *)((int)param_1 + 9) == '\b') {
      pbVar6 = (byte *)(param_1[1] + -1 + param_2);
      puVar10 = (undefined1 *)(param_2 + -1 + iVar2 * 4);
      for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
        if ((((ushort)pbVar6[-2] == *(ushort *)(param_3 + 2)) &&
            ((ushort)pbVar6[-1] == *(ushort *)(param_3 + 4))) &&
           ((ushort)*pbVar6 == *(ushort *)(param_3 + 6))) {
          *puVar10 = 0;
        }
        else {
          *puVar10 = 0xff;
        }
        puVar10[-1] = *pbVar6;
        puVar10[-2] = pbVar6[-1];
        puVar10[-3] = pbVar6[-2];
        puVar10 = puVar10 + -4;
        pbVar6 = pbVar6 + -3;
      }
    }
    else if (*(char *)((int)param_1 + 9) == '\x10') {
      puVar10 = (undefined1 *)(param_1[1] + -1 + param_2);
      puVar11 = (undefined1 *)(param_2 + -1 + iVar2 * 8);
      for (iVar8 = iVar2; iVar8 != 0; iVar8 = iVar8 + -1) {
        if (((CONCAT11(puVar10[-5],puVar10[-4]) == *(short *)(param_3 + 2)) &&
            (CONCAT11(puVar10[-3],puVar10[-2]) == *(short *)(param_3 + 4))) &&
           (CONCAT11(puVar10[-1],*puVar10) == *(short *)(param_3 + 6))) {
          *puVar11 = 0;
          puVar11[-1] = 0;
        }
        else {
          *puVar11 = 0xff;
          puVar11[-1] = 0xff;
        }
        puVar11[-2] = *puVar10;
        puVar11[-3] = puVar10[-1];
        puVar11[-4] = puVar10[-2];
        puVar11[-5] = puVar10[-3];
        puVar11[-6] = puVar10[-4];
        puVar11[-7] = puVar10[-5];
        puVar11 = puVar11 + -8;
        puVar10 = puVar10 + -6;
      }
    }
    *(undefined1 *)(param_1 + 2) = 6;
    *(undefined1 *)((int)param_1 + 10) = 4;
    bVar3 = *(char *)((int)param_1 + 9) << 2;
  }
  *(byte *)((int)param_1 + 0xb) = bVar3;
  param_1[1] = (uint)bVar3 * iVar2 >> 3;
  return;
}

