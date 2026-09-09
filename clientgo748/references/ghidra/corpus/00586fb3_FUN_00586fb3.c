// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586fb3 | Name: FUN_00586fb3


void FUN_00586fb3(int param_1,int param_2,int param_3,undefined4 *param_4)

{
  byte *pbVar1;
  byte bVar2;
  byte *pbVar3;
  byte *pbVar4;
  int iVar5;
  undefined4 *puVar6;
  
  puVar6 = param_4;
  param_4 = (undefined4 *)0x0;
  puVar6 = (undefined4 *)*puVar6;
  if (0 < *(int *)(param_1 + 0x110)) {
    param_3 = param_3 - (int)puVar6;
    do {
      pbVar4 = *(byte **)(param_3 + (int)puVar6);
      bVar2 = *pbVar4;
      pbVar3 = (byte *)*puVar6;
      *pbVar3 = bVar2;
      pbVar4 = pbVar4 + 1;
      pbVar3[1] = (byte)((int)((uint)bVar2 * 3 + 2 + (uint)*pbVar4) >> 2);
      for (iVar5 = *(int *)(param_2 + 0x28) + -2; pbVar3 = pbVar3 + 2, iVar5 != 0;
          iVar5 = iVar5 + -1) {
        bVar2 = *pbVar4;
        pbVar1 = pbVar4 + -1;
        pbVar4 = pbVar4 + 1;
        *pbVar3 = (byte)((int)(*pbVar1 + 1 + (uint)bVar2 * 3) >> 2);
        pbVar3[1] = (byte)((int)(*pbVar4 + 2 + (uint)bVar2 * 3) >> 2);
      }
      bVar2 = *pbVar4;
      param_4 = (undefined4 *)((int)param_4 + 1);
      *pbVar3 = (byte)((int)(pbVar4[-1] + 1 + (uint)bVar2 * 3) >> 2);
      pbVar3[1] = bVar2;
      puVar6 = puVar6 + 1;
    } while ((int)param_4 < *(int *)(param_1 + 0x110));
  }
  return;
}

