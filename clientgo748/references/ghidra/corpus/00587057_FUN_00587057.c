// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587057 | Name: FUN_00587057


void FUN_00587057(int param_1,int param_2,undefined4 *param_3,int *param_4)

{
  int iVar1;
  undefined1 *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  byte *pbVar6;
  int iVar7;
  byte *pbVar8;
  int local_8;
  
  iVar1 = *param_4;
  local_8 = 0;
  if (0 < *(int *)(param_1 + 0x110)) {
    do {
      param_4 = (int *)0x0;
      do {
        pbVar6 = (byte *)*param_3;
        if (param_4 == (int *)0x0) {
          pbVar8 = (byte *)param_3[-1];
        }
        else {
          pbVar8 = (byte *)param_3[1];
        }
        puVar2 = *(undefined1 **)(iVar1 + local_8 * 4);
        local_8 = local_8 + 1;
        iVar5 = (uint)*pbVar6 * 3 + (uint)*pbVar8;
        iVar3 = (uint)pbVar6[1] * 3 + (uint)pbVar8[1];
        *puVar2 = (char)(iVar5 * 4 + 8 >> 4);
        pbVar8 = pbVar8 + 2;
        pbVar6 = pbVar6 + 2;
        puVar2[1] = (char)(iVar5 * 3 + 7 + iVar3 >> 4);
        for (iVar7 = *(int *)(param_2 + 0x28) + -2; puVar2 = puVar2 + 2, iVar7 != 0;
            iVar7 = iVar7 + -1) {
          iVar4 = (uint)*pbVar6 * 3 + (uint)*pbVar8;
          *puVar2 = (char)(iVar3 * 3 + 8 + iVar5 >> 4);
          pbVar8 = pbVar8 + 1;
          pbVar6 = pbVar6 + 1;
          puVar2[1] = (char)(iVar3 * 3 + 7 + iVar4 >> 4);
          iVar5 = iVar3;
          iVar3 = iVar4;
        }
        param_4 = (int *)((int)param_4 + 1);
        *puVar2 = (char)(iVar3 * 3 + 8 + iVar5 >> 4);
        puVar2[1] = (char)(iVar3 * 4 + 7 >> 4);
      } while ((int)param_4 < 2);
      param_3 = param_3 + 1;
    } while (local_8 < *(int *)(param_1 + 0x110));
  }
  return;
}

