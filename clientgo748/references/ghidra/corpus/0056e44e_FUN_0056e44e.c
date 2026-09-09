// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e44e | Name: FUN_0056e44e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e44e(int param_1,int param_2,int param_3,float *param_4)

{
  byte *pbVar1;
  float fVar2;
  float fVar3;
  byte *pbVar4;
  
  fVar3 = _DAT_005a68b0;
  pbVar4 = (byte *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  pbVar1 = pbVar4 + *(int *)(param_1 + 0x1058) * 2;
  for (; pbVar4 < pbVar1; pbVar4 = pbVar4 + 2) {
    fVar2 = (float)*pbVar4 * fVar3;
    param_4[2] = fVar2;
    param_4[1] = fVar2;
    *param_4 = fVar2;
    param_4[3] = (float)pbVar4[1] * fVar3;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

