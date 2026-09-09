// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00589353 | Name: FUN_00589353


void FUN_00589353(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  byte *pbVar7;
  int iVar8;
  int local_c;
  
  iVar3 = *(int *)(*(int *)(param_1 + 0x1a4) + 0x18);
  iVar4 = *(int *)(param_1 + 0x5c);
  iVar5 = *(int *)(param_1 + 100);
  if (0 < param_4) {
    param_2 = param_2 - (int)param_3;
    local_c = param_4;
    do {
      pbVar7 = *(byte **)(param_2 + (int)param_3);
      pcVar6 = (char *)*param_3;
      for (iVar2 = iVar4; iVar2 != 0; iVar2 = iVar2 + -1) {
        cVar1 = '\0';
        iVar8 = 0;
        if (0 < iVar5) {
          do {
            cVar1 = cVar1 + *(char *)(*(int *)(iVar3 + iVar8 * 4) + (uint)*pbVar7);
            pbVar7 = pbVar7 + 1;
            iVar8 = iVar8 + 1;
          } while (iVar8 < iVar5);
        }
        *pcVar6 = cVar1;
        pcVar6 = pcVar6 + 1;
      }
      param_3 = param_3 + 1;
      local_c = local_c + -1;
    } while (local_c != 0);
  }
  return;
}

