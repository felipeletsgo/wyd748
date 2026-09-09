// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005757a0 | Name: FUN_005757a0


int FUN_005757a0(int param_1,uint param_2,uint param_3)

{
  int iVar1;
  uint uVar2;
  byte *pbVar3;
  byte *pbVar4;
  bool bVar5;
  bool bVar6;
  
  uVar2 = 8;
  if (((param_3 < 9) && (uVar2 = param_3, param_3 == 0)) || (7 < param_2)) {
    return 0;
  }
  if (8 < param_2 + uVar2) {
    uVar2 = 8 - param_2;
  }
  bVar5 = false;
  iVar1 = 0;
  bVar6 = true;
  pbVar3 = (byte *)(param_1 + param_2);
  pbVar4 = &DAT_005a8544 + param_2;
  do {
    if (uVar2 == 0) break;
    uVar2 = uVar2 - 1;
    bVar5 = *pbVar3 < *pbVar4;
    bVar6 = *pbVar3 == *pbVar4;
    pbVar3 = pbVar3 + 1;
    pbVar4 = pbVar4 + 1;
  } while (bVar6);
  if (!bVar6) {
    iVar1 = (1 - (uint)bVar5) - (uint)(bVar5 != 0);
  }
  return iVar1;
}

