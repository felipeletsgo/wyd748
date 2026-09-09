// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005685cc | Name: FUN_005685cc


undefined4 __fastcall FUN_005685cc(int *param_1)

{
  ushort *puVar1;
  ushort uVar2;
  int iVar3;
  ushort *puVar4;
  int iVar5;
  ushort *puVar6;
  ushort *puVar7;
  ushort *puVar8;
  ushort *puVar9;
  ushort *puVar10;
  
  puVar9 = *(ushort **)(param_1[1] + 0x18);
  iVar3 = *param_1;
  puVar4 = *(ushort **)(iVar3 + 0x18);
  iVar5 = *(int *)(iVar3 + 0x1050);
  puVar7 = (ushort *)(*(int *)(iVar3 + 0x105c) * iVar5 + (int)puVar4);
  for (; puVar4 < puVar7; puVar4 = puVar4 + iVar5) {
    iVar3 = *(int *)(iVar3 + 0x1058);
    puVar10 = (ushort *)(iVar5 + (int)puVar4);
    puVar6 = puVar9;
    for (puVar8 = puVar4; puVar8 < puVar4 + iVar3; puVar8 = puVar8 + 2) {
      puVar1 = puVar10 + 1;
      uVar2 = *puVar10;
      puVar10 = puVar10 + 2;
      *puVar6 = (ushort)((*puVar1 & 0xffff00f0) + (*puVar8 & 0xffff00f0) + (uVar2 & 0xffff00f0) +
                         0x20 + (puVar8[1] & 0xffff00f0) >> 2) & 0xf0 |
                (ushort)((uint)(*puVar1 & 0xf0f) + (uint)(*puVar8 & 0xf0f) + (uint)(uVar2 & 0xf0f) +
                         0x202 + (uint)(puVar8[1] & 0xf0f) >> 2) & 0xf0f;
      puVar6 = puVar6 + 1;
    }
    puVar9 = (ushort *)((int)puVar9 + *(int *)(param_1[1] + 0x1050));
    iVar3 = *param_1;
    iVar5 = *(int *)(iVar3 + 0x1050);
  }
  return 0;
}

