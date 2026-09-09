// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e9cb | Name: FUN_0056e9cb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e9cb(int param_1,int param_2,int param_3,float *param_4)

{
  ushort *puVar1;
  float fVar2;
  ushort *puVar3;
  
  puVar3 = (ushort *)
           (*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
           *(int *)(param_1 + 0x18));
  puVar1 = puVar3 + *(int *)(param_1 + 0x1058);
  for (; puVar3 < puVar1; puVar3 = puVar3 + 1) {
    fVar2 = (float)*puVar3 * _DAT_005a69b4;
    param_4[2] = fVar2;
    param_4[1] = fVar2;
    *param_4 = fVar2;
    param_4[3] = 1.0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

