// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00568480 | Name: FUN_00568480


undefined4 __fastcall FUN_00568480(int *param_1)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  int iVar5;
  ushort *puVar6;
  int iVar7;
  ushort *puVar8;
  ushort *puVar9;
  ushort *puVar10;
  ushort *puVar11;
  ushort *puVar12;
  
  iVar5 = *param_1;
  puVar6 = *(ushort **)(param_1[1] + 0x18);
  iVar7 = *(int *)(iVar5 + 0x1050);
  puVar8 = *(ushort **)(iVar5 + 0x18);
  puVar12 = (ushort *)(*(int *)(iVar5 + 0x105c) * iVar7 + (int)puVar8);
  for (; puVar8 < puVar12; puVar8 = puVar8 + iVar7) {
    iVar5 = *(int *)(iVar5 + 0x1058);
    puVar11 = (ushort *)(iVar7 + (int)puVar8);
    puVar9 = puVar6;
    for (puVar10 = puVar8; puVar10 < puVar8 + iVar5; puVar10 = puVar10 + 2) {
      uVar1 = *puVar10;
      uVar2 = puVar10[1];
      uVar3 = puVar11[1];
      uVar4 = *puVar11;
      puVar11 = puVar11 + 2;
      *puVar9 = (byte)((byte)((uint)(uVar3 & 0xe3) + (uint)(uVar1 & 0xe3) + (uint)(uVar4 & 0xe3) +
                              0x42 + (uint)(uVar2 & 0xe3) >> 2) ^
                      (byte)((uint)(uVar3 & 0xff1c) + (uVar1 & 0xffffff1c) + (uint)(uVar4 & 0xff1c)
                             + 0x208 + (uint)(uVar2 & 0xff1c) >> 2)) & 0xe3 ^
                (ushort)((uVar3 & 0xff1c) + (uVar1 & 0xff1c) + (uVar4 & 0xff1c) + 0x208 +
                         (uVar2 & 0xff1c) >> 2);
      puVar9 = puVar9 + 1;
    }
    puVar6 = (ushort *)((int)puVar6 + *(int *)(param_1[1] + 0x1050));
    iVar5 = *param_1;
    iVar7 = *(int *)(iVar5 + 0x1050);
  }
  return 0;
}

