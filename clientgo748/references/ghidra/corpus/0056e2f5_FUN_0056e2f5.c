// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e2f5 | Name: FUN_0056e2f5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e2f5(int param_1,int param_2,int param_3,undefined4 *param_4)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined4 *puVar3;
  
  pbVar2 = (byte *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  pbVar1 = pbVar2 + *(int *)(param_1 + 0x1058) * 2;
  for (; pbVar2 < pbVar1; pbVar2 = pbVar2 + 2) {
    puVar3 = (undefined4 *)((*pbVar2 + 3) * 0x10 + param_1);
    *param_4 = *puVar3;
    param_4[1] = puVar3[1];
    param_4[2] = puVar3[2];
    param_4[3] = puVar3[3];
    param_4[3] = (float)pbVar2[1] * _DAT_005a68b0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

