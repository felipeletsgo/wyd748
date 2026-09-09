// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587ba7 | Name: FUN_00587ba7


void FUN_00587ba7(int param_1,int *param_2,int param_3,undefined4 *param_4)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  undefined1 *puVar11;
  byte *pbVar12;
  uint uVar13;
  uint uVar14;
  byte *pbVar15;
  int iVar16;
  byte *pbVar17;
  
  iVar3 = *(int *)(param_1 + 0x19c);
  iVar4 = *(int *)(iVar3 + 0x10);
  iVar5 = *(int *)(param_1 + 0x11c);
  iVar6 = *(int *)(iVar3 + 0x14);
  iVar7 = *(int *)(iVar3 + 0x18);
  iVar3 = *(int *)(iVar3 + 0x1c);
  param_3 = param_3 * 4;
  pbVar17 = *(byte **)(param_3 + *param_2);
  pbVar12 = *(byte **)(param_3 + param_2[1]);
  pbVar15 = *(byte **)(param_3 + param_2[2]);
  puVar11 = (undefined1 *)*param_4;
  for (uVar14 = *(uint *)(param_1 + 0x5c) >> 1; uVar14 != 0; uVar14 = uVar14 - 1) {
    bVar1 = *pbVar12;
    pbVar12 = pbVar12 + 1;
    bVar2 = *pbVar15;
    pbVar15 = pbVar15 + 1;
    iVar8 = *(int *)((uint)bVar2 * 4 + iVar4);
    iVar16 = *(int *)((uint)bVar1 * 4 + iVar3);
    iVar9 = *(int *)((uint)bVar2 * 4 + iVar7);
    iVar10 = *(int *)((uint)bVar1 * 4 + iVar6);
    uVar13 = (uint)*pbVar17;
    *puVar11 = *(undefined1 *)(iVar8 + uVar13 + iVar5);
    iVar16 = iVar16 + iVar9 >> 0x10;
    puVar11[1] = *(undefined1 *)(uVar13 + iVar16 + iVar5);
    puVar11[2] = *(undefined1 *)(uVar13 + iVar10 + iVar5);
    uVar13 = (uint)pbVar17[1];
    puVar11[3] = *(undefined1 *)(iVar8 + uVar13 + iVar5);
    puVar11[4] = *(undefined1 *)(iVar16 + uVar13 + iVar5);
    puVar11[5] = *(undefined1 *)(uVar13 + iVar10 + iVar5);
    pbVar17 = pbVar17 + 2;
    puVar11 = puVar11 + 6;
  }
  if ((*(byte *)(param_1 + 0x5c) & 1) != 0) {
    uVar14 = (uint)*pbVar17;
    iVar3 = *(int *)((uint)*pbVar12 * 4 + iVar3);
    iVar7 = *(int *)((uint)*pbVar15 * 4 + iVar7);
    iVar6 = *(int *)((uint)*pbVar12 * 4 + iVar6);
    *puVar11 = *(undefined1 *)(*(int *)((uint)*pbVar15 * 4 + iVar4) + uVar14 + iVar5);
    puVar11[1] = *(undefined1 *)(uVar14 + (iVar3 + iVar7 >> 0x10) + iVar5);
    puVar11[2] = *(undefined1 *)(uVar14 + iVar6 + iVar5);
  }
  return;
}

