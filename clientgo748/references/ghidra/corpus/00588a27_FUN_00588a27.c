// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00588a27 | Name: FUN_00588a27


void FUN_00588a27(int param_1,int param_2,int *param_3,int param_4)

{
  ushort *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  byte *pbVar12;
  uint uVar13;
  int iVar14;
  int iVar15;
  uint uVar16;
  int iVar17;
  short *psVar18;
  int iVar19;
  uint uVar20;
  int iVar21;
  int local_48;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int *local_28;
  int local_24;
  int local_20;
  int local_1c;
  undefined1 *local_14;
  int local_8;
  
  iVar3 = *(int *)(param_1 + 0x1a4);
  iVar4 = *(int *)(iVar3 + 0x18);
  iVar5 = *(int *)(param_1 + 0x11c);
  iVar6 = *(int *)(iVar3 + 0x28);
  iVar7 = *(int *)(param_1 + 0x5c);
  piVar8 = *(int **)(param_1 + 0x74);
  iVar9 = *piVar8;
  iVar10 = piVar8[1];
  iVar11 = piVar8[2];
  if (0 < param_4) {
    local_28 = param_3;
    local_48 = param_4;
    do {
      pbVar12 = *(byte **)((param_2 - (int)param_3) + (int)local_28);
      local_14 = (undefined1 *)*local_28;
      if (*(char *)(iVar3 + 0x24) == '\0') {
        psVar18 = *(short **)(iVar3 + 0x20);
        local_3c = 1;
        local_8 = 3;
        *(undefined1 *)(iVar3 + 0x24) = 1;
      }
      else {
        local_3c = -1;
        *(undefined1 *)(iVar3 + 0x24) = 0;
        pbVar12 = pbVar12 + iVar7 * 3 + -3;
        local_14 = local_14 + iVar7 + -1;
        psVar18 = (short *)(*(int *)(iVar3 + 0x20) + (iVar7 * 3 + 3) * 2);
        local_8 = -3;
      }
      iVar15 = 0;
      local_24 = 0;
      local_24._0_2_ = 0;
      local_20 = 0;
      local_20._0_2_ = 0;
      local_1c = 0;
      local_1c._0_2_ = 0;
      iVar19 = 0;
      param_4 = 0;
      local_38 = 0;
      local_34 = 0;
      local_30 = 0;
      for (iVar2 = iVar7; iVar2 != 0; iVar2 = iVar2 + -1) {
        uVar20 = (uint)*(byte *)(*(int *)(iVar6 + (psVar18[local_8] + 8 + iVar19 >> 4) * 4) +
                                 (uint)*pbVar12 + iVar5);
        uVar16 = (uint)*(byte *)(*(int *)(iVar6 + (psVar18[local_8 + 1] + 8 + iVar15 >> 4) * 4) +
                                 (uint)pbVar12[1] + iVar5);
        uVar13 = (uint)*(byte *)(*(int *)(iVar6 + (psVar18[local_8 + 2] + 8 + param_4 >> 4) * 4) +
                                 (uint)pbVar12[2] + iVar5);
        iVar19 = (int)uVar13 >> 3;
        iVar15 = (int)uVar20 >> 3;
        puVar1 = (ushort *)(*(int *)(iVar4 + iVar15 * 4) + (((int)uVar16 >> 2) * 0x20 + iVar19) * 2)
        ;
        if (*puVar1 == 0) {
          FUN_005888ae(param_1,iVar15,iVar19);
        }
        iVar19 = *puVar1 - 1;
        *local_14 = (char)iVar19;
        iVar21 = uVar20 - *(byte *)(iVar19 + iVar9);
        iVar17 = uVar16 - *(byte *)(iVar19 + iVar10);
        iVar14 = uVar13 - *(byte *)(iVar19 + iVar11);
        *psVar18 = (short)local_1c + (short)iVar21 * 3;
        local_1c = local_30 + iVar21 * 5;
        iVar19 = iVar21 * 7;
        psVar18[1] = (short)local_20 + (short)iVar17 * 3;
        local_20 = local_34 + iVar17 * 5;
        iVar15 = iVar17 * 7;
        psVar18[2] = (short)local_24 + (short)iVar14 * 3;
        local_24 = local_38 + iVar14 * 5;
        param_4 = iVar14 * 7;
        pbVar12 = pbVar12 + local_8;
        local_14 = local_14 + local_3c;
        psVar18 = psVar18 + local_8;
        local_38 = iVar14;
        local_34 = iVar17;
        local_30 = iVar21;
      }
      local_28 = local_28 + 1;
      local_48 = local_48 + -1;
      *psVar18 = (short)local_1c;
      psVar18[1] = (short)local_20;
      psVar18[2] = (short)local_24;
    } while (local_48 != 0);
  }
  return;
}

