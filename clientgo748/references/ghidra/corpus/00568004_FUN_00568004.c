// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00568004 | Name: FUN_00568004


undefined4 __fastcall FUN_00568004(int *param_1)

{
  ushort *puVar1;
  int iVar2;
  ushort *puVar3;
  int iVar4;
  ushort *puVar5;
  uint uVar6;
  uint uVar7;
  ushort *puVar8;
  ushort *puVar9;
  uint uVar10;
  ushort *puVar11;
  uint uVar12;
  
  puVar1 = *(ushort **)(param_1[1] + 0x18);
  iVar2 = *param_1;
  puVar3 = *(ushort **)(iVar2 + 0x18);
  iVar4 = *(int *)(iVar2 + 0x1050);
  puVar9 = (ushort *)(*(int *)(iVar2 + 0x105c) * iVar4 + (int)puVar3);
  for (; puVar3 < puVar9; puVar3 = puVar3 + iVar4) {
    iVar2 = *(int *)(iVar2 + 0x1058);
    puVar11 = (ushort *)(iVar4 + (int)puVar3);
    puVar5 = puVar1;
    for (puVar8 = puVar3; puVar8 < puVar3 + iVar2; puVar8 = puVar8 + 2) {
      uVar6 = (uint)*puVar8;
      uVar10 = (uint)puVar8[1];
      uVar7 = (uint)puVar11[1];
      uVar12 = (uint)*puVar11;
      *puVar5 = ((ushort)((uVar7 & 0x83e0) + (uVar6 & 0x83e0) + (uVar12 & 0x83e0) + 0x10040 +
                          (uVar10 & 0x83e0) >> 2) ^
                (ushort)((uVar10 & 0x7c1f) + 0x802 +
                         (uVar7 & 0x7c1f) + (uVar6 & 0x7c1f) + (uVar12 & 0x7c1f) >> 2)) & 0x7c1f ^
                (ushort)((uVar7 & 0x83e0) + (uVar6 & 0x83e0) + (uVar12 & 0x83e0) + 0x10040 +
                         (uVar10 & 0x83e0) >> 2);
      puVar11 = puVar11 + 2;
      puVar5 = puVar5 + 1;
    }
    puVar1 = (ushort *)((int)puVar1 + *(int *)(param_1[1] + 0x1050));
    iVar2 = *param_1;
    iVar4 = *(int *)(iVar2 + 0x1050);
  }
  return 0;
}

