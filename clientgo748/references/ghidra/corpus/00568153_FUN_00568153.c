// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00568153 | Name: FUN_00568153


undefined4 __fastcall FUN_00568153(int *param_1)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  ushort *puVar5;
  int iVar6;
  int iVar7;
  ushort *puVar8;
  ushort *puVar9;
  ushort *puVar10;
  ushort *puVar11;
  ushort *puVar12;
  
  puVar5 = *(ushort **)(param_1[1] + 0x18);
  iVar6 = *param_1;
  iVar7 = *(int *)(iVar6 + 0x1050);
  puVar8 = *(ushort **)(iVar6 + 0x18);
  puVar11 = (ushort *)(*(int *)(iVar6 + 0x105c) * iVar7 + (int)puVar8);
  for (; puVar8 < puVar11; puVar8 = puVar8 + iVar7) {
    iVar6 = *(int *)(iVar6 + 0x1058);
    puVar12 = (ushort *)(iVar7 + (int)puVar8);
    puVar9 = puVar5;
    for (puVar10 = puVar8; puVar10 < puVar8 + iVar6; puVar10 = puVar10 + 2) {
      uVar1 = *puVar10;
      uVar2 = puVar10[1];
      uVar3 = puVar12[1];
      uVar4 = *puVar12;
      *puVar9 = ((ushort)((uint)(uVar3 & 0xf0f0) + (uint)(uVar1 & 0xf0f0) + (uint)(uVar4 & 0xf0f0) +
                          0x2020 + (uint)(uVar2 & 0xf0f0) >> 2) ^
                (ushort)((uVar2 & 0xffff0f0f) + 0x202 +
                         (uVar3 & 0xffff0f0f) + (uVar1 & 0xffff0f0f) + (uVar4 & 0xffff0f0f) >> 2)) &
                0xf0f ^ (ushort)((uVar3 & 0xf0f0) + (uVar1 & 0xf0f0) + (uVar4 & 0xf0f0) + 0x2020 +
                                 (uVar2 & 0xf0f0) >> 2);
      puVar12 = puVar12 + 2;
      puVar9 = puVar9 + 1;
    }
    puVar5 = (ushort *)((int)puVar5 + *(int *)(param_1[1] + 0x1050));
    iVar6 = *param_1;
    iVar7 = *(int *)(iVar6 + 0x1050);
  }
  return 0;
}

