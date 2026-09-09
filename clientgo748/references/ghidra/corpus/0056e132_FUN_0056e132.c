// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e132 | Name: FUN_0056e132


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e132(int param_1,int param_2,int param_3,undefined4 *param_4)

{
  byte *pbVar1;
  byte *pbVar2;
  
  pbVar1 = (byte *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  pbVar2 = pbVar1 + *(int *)(param_1 + 0x1058);
  for (; pbVar1 < pbVar2; pbVar1 = pbVar1 + 1) {
    *param_4 = 0;
    param_4[1] = 0;
    param_4[2] = 0;
    param_4[3] = (float)*pbVar1 * _DAT_005a68b0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

