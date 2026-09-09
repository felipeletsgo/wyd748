// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058897e | Name: FUN_0058897e


void FUN_0058897e(int param_1,int param_2,int *param_3,int param_4)

{
  short *psVar1;
  int iVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  byte *pbVar7;
  byte *pbVar8;
  int local_10;
  int *local_8;
  
  iVar4 = *(int *)(*(int *)(param_1 + 0x1a4) + 0x18);
  iVar5 = *(int *)(param_1 + 0x5c);
  if (0 < param_4) {
    local_8 = param_3;
    local_10 = param_4;
    do {
      pbVar8 = *(byte **)((param_2 - (int)param_3) + (int)local_8);
      pcVar6 = (char *)*local_8;
      for (iVar2 = iVar5; iVar2 != 0; iVar2 = iVar2 + -1) {
        bVar3 = *pbVar8;
        pbVar7 = pbVar8 + 2;
        psVar1 = (short *)(*(int *)(iVar4 + (uint)(bVar3 >> 3) * 4) +
                          ((uint)(pbVar8[1] >> 2) * 0x20 + (uint)(*pbVar7 >> 3)) * 2);
        pbVar8 = pbVar8 + 3;
        if (*psVar1 == 0) {
          FUN_005888ae(param_1,(uint)(bVar3 >> 3),(uint)(*pbVar7 >> 3));
        }
        *pcVar6 = (char)*psVar1 + -1;
        pcVar6 = pcVar6 + 1;
      }
      local_8 = local_8 + 1;
      local_10 = local_10 + -1;
    } while (local_10 != 0);
  }
  return;
}

