// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e5e2 | Name: FUN_0056e5e2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e5e2(int param_1,int param_2,int param_3,float *param_4)

{
  ushort *puVar1;
  ushort uVar2;
  float fVar3;
  ushort *puVar4;
  
  fVar3 = _DAT_005a69c0;
  puVar4 = (ushort *)
           (*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
           *(int *)(param_1 + 0x18));
  puVar1 = puVar4 + *(int *)(param_1 + 0x1058);
  for (; puVar4 < puVar1; puVar4 = puVar4 + 1) {
    uVar2 = *puVar4;
    *param_4 = (float)(int)((char)((char)*puVar4 << 3) >> 3) * fVar3;
    param_4[1] = (float)(int)((char)((char)(uVar2 >> 5) << 3) >> 3) * fVar3;
    param_4[2] = 0.0;
    param_4[3] = (float)(*puVar4 >> 10) * _DAT_005a69a0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

