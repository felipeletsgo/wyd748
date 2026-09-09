// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00568839 | Name: FUN_00568839


undefined4 __fastcall FUN_00568839(int *param_1)

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
      *pbVar8 = ((byte)((bVar3 & 0xf) + (bVar1 & 0xf) + (bVar4 & 0xf) + 2 + (bVar2 & 0xf) >> 2) ^
                (byte)((bVar3 & 0xffff00f0) + (bVar1 & 0xffff00f0) + (bVar4 & 0xffff00f0) + 0x20 +
                       (bVar2 & 0xffff00f0) >> 2)) & 0xf ^
                (byte)((uint)(bVar3 & 0xf0) + (uint)(bVar1 & 0xf0) + (uint)(bVar4 & 0xf0) + 0x20 +
                       (uint)(bVar2 & 0xf0) >> 2);
      pbVar8 = pbVar8 + 1;
    }
    pbVar10 = pbVar10 + *(int *)(param_1[1] + 0x1050);
    iVar5 = *param_1;
    iVar7 = *(int *)(iVar5 + 0x1050);
  }
  return 0;
}

