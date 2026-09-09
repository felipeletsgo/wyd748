// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00589553 | Name: FUN_00589553


void FUN_00589553(uint param_1,int param_2,int *param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  char *pcVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  byte *pbVar13;
  int iVar14;
  int local_14;
  int *local_c;
  
  iVar2 = *(int *)(param_1 + 0x1a4);
  piVar3 = *(int **)(iVar2 + 0x18);
  iVar4 = *piVar3;
  iVar5 = piVar3[1];
  iVar6 = piVar3[2];
  iVar7 = *(int *)(param_1 + 0x5c);
  if (0 < param_4) {
    local_c = param_3;
    local_14 = param_4;
    do {
      pbVar13 = *(byte **)((param_2 - (int)param_3) + (int)local_c);
      pcVar8 = (char *)*local_c;
      iVar9 = *(int *)(iVar2 + 0x30);
      iVar10 = *(int *)(iVar2 + 0x34);
      iVar11 = *(int *)(iVar2 + 0x3c);
      param_1 = 0;
      iVar14 = iVar9 * 0x40;
      iVar12 = *(int *)(iVar2 + 0x38);
      for (iVar1 = iVar7; iVar1 != 0; iVar1 = iVar1 + -1) {
        *pcVar8 = *(char *)(*(int *)(iVar10 + iVar14 + param_1 * 4) + (uint)*pbVar13 + iVar4) +
                  *(char *)(*(int *)(iVar12 + iVar14 + param_1 * 4) + (uint)pbVar13[1] + iVar5) +
                  *(char *)(*(int *)(iVar11 + iVar14 + param_1 * 4) + (uint)pbVar13[2] + iVar6);
        pbVar13 = pbVar13 + 3;
        pcVar8 = pcVar8 + 1;
        param_1 = param_1 + 1 & 0xf;
      }
      local_c = local_c + 1;
      local_14 = local_14 + -1;
      *(uint *)(iVar2 + 0x30) = iVar9 + 1U & 0xf;
    } while (local_14 != 0);
  }
  return;
}

