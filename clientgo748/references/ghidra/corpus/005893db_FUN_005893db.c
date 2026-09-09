// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005893db | Name: FUN_005893db


void FUN_005893db(int param_1,int param_2,int *param_3,int param_4)

{
  int iVar1;
  byte bVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  byte *pbVar10;
  char *pcVar11;
  
  piVar3 = *(int **)(*(int *)(param_1 + 0x1a4) + 0x18);
  iVar4 = *piVar3;
  iVar5 = piVar3[1];
  iVar6 = piVar3[2];
  iVar7 = *(int *)(param_1 + 0x5c);
  if (0 < param_4) {
    param_2 = param_2 - (int)param_3;
    do {
      pbVar10 = *(byte **)(param_2 + (int)param_3);
      pcVar11 = (char *)*param_3;
      for (iVar1 = iVar7; iVar1 != 0; iVar1 = iVar1 + -1) {
        bVar2 = *pbVar10;
        pbVar8 = pbVar10 + 1;
        pbVar9 = pbVar10 + 2;
        pbVar10 = pbVar10 + 3;
        *pcVar11 = *(char *)((uint)bVar2 + iVar4) + *(char *)((uint)*pbVar8 + iVar5) +
                   *(char *)((uint)*pbVar9 + iVar6);
        pcVar11 = pcVar11 + 1;
      }
      param_3 = param_3 + 1;
      param_4 = param_4 + -1;
    } while (param_4 != 0);
  }
  return;
}

