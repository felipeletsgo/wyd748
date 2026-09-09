// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587cfd | Name: FUN_00587cfd


void FUN_00587cfd(int param_1,int *param_2,int param_3,undefined4 *param_4)

{
  undefined4 *puVar1;
  byte bVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  undefined1 *puVar13;
  byte *pbVar14;
  uint uVar15;
  uint uVar16;
  undefined1 *puVar17;
  byte *pbVar18;
  int iVar19;
  int iVar20;
  
  iVar20 = *(int *)(param_1 + 0x19c);
  iVar4 = *(int *)(iVar20 + 0x10);
  iVar5 = *(int *)(param_1 + 0x11c);
  iVar6 = *(int *)(iVar20 + 0x14);
  iVar7 = *(int *)(iVar20 + 0x18);
  iVar20 = *(int *)(iVar20 + 0x1c);
  puVar1 = (undefined4 *)(*param_2 + param_3 * 8);
  pbVar8 = (byte *)*puVar1;
  pbVar9 = (byte *)puVar1[1];
  pbVar14 = *(byte **)(param_3 * 4 + param_2[1]);
  pbVar18 = *(byte **)(param_3 * 4 + param_2[2]);
  puVar13 = (undefined1 *)*param_4;
  puVar17 = (undefined1 *)param_4[1];
  for (uVar16 = *(uint *)(param_1 + 0x5c) >> 1; uVar16 != 0; uVar16 = uVar16 - 1) {
    bVar2 = *pbVar14;
    pbVar14 = pbVar14 + 1;
    bVar3 = *pbVar18;
    pbVar18 = pbVar18 + 1;
    iVar10 = *(int *)((uint)bVar3 * 4 + iVar4);
    iVar19 = *(int *)((uint)bVar2 * 4 + iVar20);
    iVar11 = *(int *)((uint)bVar3 * 4 + iVar7);
    iVar12 = *(int *)((uint)bVar2 * 4 + iVar6);
    uVar15 = (uint)*pbVar8;
    *puVar13 = *(undefined1 *)(iVar10 + uVar15 + iVar5);
    iVar19 = iVar19 + iVar11 >> 0x10;
    puVar13[1] = *(undefined1 *)(uVar15 + iVar19 + iVar5);
    puVar13[2] = *(undefined1 *)(uVar15 + iVar12 + iVar5);
    uVar15 = (uint)pbVar8[1];
    puVar13[3] = *(undefined1 *)(iVar10 + uVar15 + iVar5);
    puVar13[4] = *(undefined1 *)(uVar15 + iVar19 + iVar5);
    puVar13[5] = *(undefined1 *)(uVar15 + iVar12 + iVar5);
    uVar15 = (uint)*pbVar9;
    pbVar8 = pbVar8 + 2;
    *puVar17 = *(undefined1 *)(iVar10 + uVar15 + iVar5);
    puVar17[1] = *(undefined1 *)(uVar15 + iVar19 + iVar5);
    puVar17[2] = *(undefined1 *)(uVar15 + iVar12 + iVar5);
    puVar13 = puVar13 + 6;
    uVar15 = (uint)pbVar9[1];
    pbVar9 = pbVar9 + 2;
    puVar17[3] = *(undefined1 *)(iVar10 + uVar15 + iVar5);
    puVar17[4] = *(undefined1 *)(iVar19 + uVar15 + iVar5);
    puVar17[5] = *(undefined1 *)(uVar15 + iVar12 + iVar5);
    puVar17 = puVar17 + 6;
  }
  if ((*(byte *)(param_1 + 0x5c) & 1) != 0) {
    iVar4 = *(int *)((uint)*pbVar18 * 4 + iVar4);
    iVar20 = *(int *)((uint)*pbVar14 * 4 + iVar20);
    iVar7 = *(int *)((uint)*pbVar18 * 4 + iVar7);
    iVar6 = *(int *)((uint)*pbVar14 * 4 + iVar6);
    uVar16 = (uint)*pbVar8;
    *puVar13 = *(undefined1 *)(iVar4 + uVar16 + iVar5);
    iVar20 = iVar20 + iVar7 >> 0x10;
    puVar13[1] = *(undefined1 *)(uVar16 + iVar20 + iVar5);
    puVar13[2] = *(undefined1 *)(uVar16 + iVar6 + iVar5);
    uVar16 = (uint)*pbVar9;
    *puVar17 = *(undefined1 *)(iVar4 + uVar16 + iVar5);
    puVar17[1] = *(undefined1 *)(uVar16 + iVar20 + iVar5);
    puVar17[2] = *(undefined1 *)(uVar16 + iVar6 + iVar5);
  }
  return;
}

