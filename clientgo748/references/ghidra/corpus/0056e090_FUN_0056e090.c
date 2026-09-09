// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e090 | Name: FUN_0056e090


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e090(int param_1,int param_2,int param_3,float *param_4)

{
  float fVar1;
  byte *pbVar2;
  byte *pbVar3;
  
  fVar1 = _DAT_005a69b8;
  pbVar2 = (byte *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  pbVar3 = pbVar2 + *(int *)(param_1 + 0x1058);
  for (; pbVar2 < pbVar3; pbVar2 = pbVar2 + 1) {
    *param_4 = (float)(*pbVar2 >> 5) * fVar1;
    param_4[1] = (float)(*pbVar2 >> 2 & 7) * fVar1;
    param_4[2] = (float)(*pbVar2 & 3) * _DAT_005a69ac;
    param_4[3] = 1.0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

