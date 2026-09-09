// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005682a6 | Name: FUN_005682a6


undefined4 __fastcall FUN_005682a6(int *param_1)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  byte bVar4;
  int iVar5;
  byte *pbVar6;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  byte *pbVar10;
  byte *pbVar11;
  byte *pbVar12;
  
  iVar5 = *param_1;
  pbVar10 = *(byte **)(param_1[1] + 0x18);
  pbVar6 = *(byte **)(iVar5 + 0x18);
  iVar7 = *(int *)(iVar5 + 0x1050);
  pbVar11 = pbVar6 + *(int *)(iVar5 + 0x105c) * iVar7;
  for (; pbVar6 < pbVar11; pbVar6 = pbVar6 + iVar7 * 2) {
    iVar5 = *(int *)(iVar5 + 0x1058);
    pbVar12 = pbVar6 + iVar7;
    pbVar8 = pbVar10;
    for (pbVar9 = pbVar6; pbVar9 < pbVar6 + iVar5; pbVar9 = pbVar9 + 2) {
      bVar1 = *pbVar9;
      bVar2 = pbVar9[1];
      bVar3 = pbVar12[1];
      bVar4 = *pbVar12;
      pbVar12 = pbVar12 + 2;
      *pbVar8 = ((byte)((bVar3 & 0x1c) + (bVar1 & 0x1c) + (bVar4 & 0x1c) + 8 + (bVar2 & 0x1c) >> 2)
                ^ (byte)((bVar3 & 0xffff00e3) + (bVar1 & 0xffff00e3) + (bVar4 & 0xffff00e3) + 0x42 +
                         (bVar2 & 0xffff00e3) >> 2)) & 0x1c ^
                (byte)((uint)(bVar3 & 0xe3) + (uint)(bVar1 & 0xe3) + (uint)(bVar4 & 0xe3) + 0x42 +
                       (uint)(bVar2 & 0xe3) >> 2);
      pbVar8 = pbVar8 + 1;
    }
    pbVar10 = pbVar10 + *(int *)(param_1[1] + 0x1050);
    iVar5 = *param_1;
    iVar7 = *(int *)(iVar5 + 0x1050);
  }
  return 0;
}

