// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00589468 | Name: FUN_00589468


void FUN_00589468(int param_1,int param_2,undefined4 *param_3,uint param_4)

{
  int iVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  byte *pbVar9;
  char *pcVar10;
  int local_1c;
  int *local_8;
  
  iVar3 = *(int *)(param_1 + 100);
  iVar4 = *(int *)(param_1 + 0x1a4);
  iVar5 = *(int *)(param_1 + 0x5c);
  if (0 < (int)param_4) {
    param_2 = param_2 - (int)param_3;
    local_1c = param_4;
    do {
      FUN_00577ec2(*param_3,iVar5);
      iVar6 = *(int *)(iVar4 + 0x30);
      param_1 = 0;
      if (0 < iVar3) {
        local_8 = (int *)(iVar4 + 0x34);
        do {
          param_4 = 0;
          pcVar10 = (char *)*param_3;
          pbVar9 = (byte *)(*(int *)(param_2 + (int)param_3) + param_1);
          iVar7 = *(int *)(*(int *)(iVar4 + 0x18) + param_1 * 4);
          iVar8 = *local_8;
          for (iVar1 = iVar5; iVar1 != 0; iVar1 = iVar1 + -1) {
            bVar2 = *pbVar9;
            pbVar9 = pbVar9 + iVar3;
            *pcVar10 = *pcVar10 +
                       *(char *)(*(int *)(iVar8 + iVar6 * 0x40 + param_4 * 4) + (uint)bVar2 + iVar7)
            ;
            pcVar10 = pcVar10 + 1;
            param_4 = param_4 + 1 & 0xf;
          }
          param_1 = param_1 + 1;
          local_8 = local_8 + 1;
        } while (param_1 < iVar3);
      }
      param_3 = param_3 + 1;
      local_1c = local_1c + -1;
      *(uint *)(iVar4 + 0x30) = iVar6 + 1U & 0xf;
    } while (local_1c != 0);
  }
  return;
}

