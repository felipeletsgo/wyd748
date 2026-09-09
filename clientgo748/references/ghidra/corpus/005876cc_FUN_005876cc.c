// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005876cc | Name: FUN_005876cc


void FUN_005876cc(int param_1,int *param_2,int param_3,int *param_4,int param_5)

{
  int *piVar1;
  int *piVar2;
  byte bVar3;
  byte bVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  byte *pbVar13;
  undefined1 *puVar14;
  uint uVar15;
  int iVar16;
  int iVar17;
  uint uVar18;
  uint local_1c;
  int local_18;
  
  iVar5 = *(int *)(param_1 + 0x1a0);
  iVar6 = *(int *)(param_1 + 0x11c);
  uVar7 = *(uint *)(param_1 + 0x5c);
  iVar8 = *(int *)(iVar5 + 8);
  iVar9 = *(int *)(iVar5 + 0xc);
  iVar10 = *(int *)(iVar5 + 0x10);
  iVar5 = *(int *)(iVar5 + 0x14);
  uVar18 = uVar7 & 0xfffffff8;
  if (DAT_005c7da8 == '\0') {
    if (-1 < param_5 + -1) {
      param_3 = param_3 << 2;
      local_18 = param_5;
      do {
        piVar1 = (int *)(param_3 + *param_2);
        pbVar13 = *(byte **)(param_3 + param_2[1]);
        piVar2 = (int *)(param_3 + param_2[2]);
        param_3 = param_3 + 4;
        if (uVar7 != 0) {
          iVar16 = *piVar1 - (int)pbVar13;
          iVar17 = *piVar2 - (int)pbVar13;
          puVar14 = (undefined1 *)*param_4;
          local_1c = uVar7;
          do {
            bVar3 = pbVar13[iVar17];
            uVar18 = (uint)pbVar13[iVar16];
            bVar4 = *pbVar13;
            *puVar14 = *(undefined1 *)(*(int *)((uint)bVar3 * 4 + iVar8) + uVar18 + iVar6);
            puVar14[1] = *(undefined1 *)
                          ((*(int *)((uint)bVar4 * 4 + iVar5) + *(int *)((uint)bVar3 * 4 + iVar10)
                           >> 0x10) + uVar18 + iVar6);
            pbVar13 = pbVar13 + 1;
            local_1c = local_1c - 1;
            puVar14[2] = *(undefined1 *)(*(int *)((uint)bVar4 * 4 + iVar9) + uVar18 + iVar6);
            puVar14 = puVar14 + 3;
          } while (local_1c != 0);
        }
        local_18 = local_18 + -1;
        param_4 = param_4 + 1;
      } while (local_18 != 0);
    }
  }
  else if (-1 < param_5 + -1) {
    param_3 = param_3 << 2;
    local_1c = param_5;
    do {
      iVar16 = *(int *)(param_3 + *param_2);
      iVar17 = *(int *)(param_3 + param_2[1]);
      iVar11 = *(int *)(param_3 + param_2[2]);
      param_3 = param_3 + 4;
      iVar12 = *param_4;
      FUN_00587590(uVar18,iVar16,iVar17,iVar11,iVar12);
      if (uVar18 < uVar7) {
        pbVar13 = (byte *)(iVar17 + uVar18);
        local_18 = uVar7 - uVar18;
        puVar14 = (undefined1 *)(iVar12 + uVar18 * 3);
        do {
          bVar3 = pbVar13[iVar11 - iVar17];
          uVar15 = (uint)pbVar13[iVar16 - iVar17];
          bVar4 = *pbVar13;
          *puVar14 = *(undefined1 *)(*(int *)((uint)bVar3 * 4 + iVar8) + uVar15 + iVar6);
          puVar14[1] = *(undefined1 *)
                        ((*(int *)((uint)bVar4 * 4 + iVar5) + *(int *)((uint)bVar3 * 4 + iVar10) >>
                         0x10) + uVar15 + iVar6);
          pbVar13 = pbVar13 + 1;
          local_18 = local_18 + -1;
          puVar14[2] = *(undefined1 *)(*(int *)((uint)bVar4 * 4 + iVar9) + uVar15 + iVar6);
          puVar14 = puVar14 + 3;
        } while (local_18 != 0);
      }
      local_1c = local_1c + -1;
      param_4 = param_4 + 1;
    } while (local_1c != 0);
  }
  return;
}

