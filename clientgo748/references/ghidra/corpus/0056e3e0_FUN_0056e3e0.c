// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e3e0 | Name: FUN_0056e3e0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e3e0(int param_1,int param_2,int param_3,float *param_4)

{
  float fVar1;
  byte *pbVar2;
  byte *pbVar3;
  
  pbVar2 = (byte *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  pbVar3 = pbVar2 + *(int *)(param_1 + 0x1058);
  for (; pbVar2 < pbVar3; pbVar2 = pbVar2 + 1) {
    fVar1 = (float)*pbVar2 * _DAT_005a68b0;
    param_4[2] = fVar1;
    param_4[1] = fVar1;
    *param_4 = fVar1;
    param_4[3] = 1.0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

