// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00589652 | Name: FUN_00589652


void FUN_00589652(int param_1,int param_2,undefined4 *param_3,undefined4 *param_4)

{
  int iVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  short *psVar12;
  char *pcVar13;
  byte *pbVar14;
  int local_28;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int *local_c;
  
  local_28 = (int)param_4;
  iVar3 = *(int *)(param_1 + 100);
  iVar4 = *(int *)(param_1 + 0x5c);
  iVar5 = *(int *)(param_1 + 0x1a4);
  iVar6 = *(int *)(param_1 + 0x11c);
  if (0 < (int)param_4) {
    param_4 = param_3;
    do {
      FUN_00577ec2(*param_4,iVar4);
      param_1 = 0;
      if (0 < iVar3) {
        local_c = (int *)(iVar5 + 0x44);
        do {
          pbVar14 = (byte *)(*(int *)((param_2 - (int)param_3) + (int)param_4) + param_1);
          pcVar13 = (char *)*param_4;
          if (*(char *)(iVar5 + 0x54) == '\0') {
            psVar12 = (short *)*local_c;
            local_14 = 1;
            local_20 = iVar3;
          }
          else {
            local_14 = -1;
            pcVar13 = pcVar13 + iVar4 + -1;
            pbVar14 = pbVar14 + (iVar4 + -1) * iVar3;
            psVar12 = (short *)(*local_c + 2 + iVar4 * 2);
            local_20 = -iVar3;
          }
          iVar7 = *(int *)(param_1 * 4 + *(int *)(iVar5 + 0x18));
          iVar8 = *(int *)(param_1 * 4 + *(int *)(iVar5 + 0x10));
          iVar9 = 0;
          local_18 = 0;
          local_18._0_2_ = 0;
          local_1c = 0;
          for (iVar1 = iVar4; iVar1 != 0; iVar1 = iVar1 + -1) {
            uVar10 = (uint)*(byte *)((uint)*pbVar14 + iVar6 + (psVar12[local_14] + 8 + iVar9 >> 4));
            bVar2 = *(byte *)(uVar10 + iVar7);
            *pcVar13 = *pcVar13 + bVar2;
            pbVar14 = pbVar14 + local_20;
            pcVar13 = pcVar13 + local_14;
            iVar11 = uVar10 - *(byte *)((uint)bVar2 + iVar8);
            *psVar12 = (short)local_18 + (short)iVar11 * 3;
            local_18 = local_1c + iVar11 * 5;
            iVar9 = iVar11 * 7;
            psVar12 = psVar12 + local_14;
            local_1c = iVar11;
          }
          param_1 = param_1 + 1;
          local_c = local_c + 1;
          *psVar12 = (short)local_18;
        } while (param_1 < iVar3);
      }
      param_4 = param_4 + 1;
      local_28 = local_28 + -1;
      *(bool *)(iVar5 + 0x54) = *(char *)(iVar5 + 0x54) == '\0';
    } while (local_28 != 0);
  }
  return;
}

