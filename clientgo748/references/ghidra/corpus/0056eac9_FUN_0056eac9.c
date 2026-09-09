// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056eac9 | Name: FUN_0056eac9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056eac9(int param_1,int param_2,int param_3,float *param_4)

{
  float fVar1;
  ushort *puVar2;
  ushort *puVar3;
  
  fVar1 = _DAT_005a69b4;
  puVar2 = (ushort *)
           (*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
           *(int *)(param_1 + 0x18));
  puVar3 = (ushort *)(*(int *)(param_1 + 0x1064) + (int)puVar2);
  for (; puVar2 < puVar3; puVar2 = puVar2 + 3) {
    *param_4 = (float)puVar2[2] * fVar1;
    param_4[1] = (float)puVar2[1] * fVar1;
    param_4[2] = (float)*puVar2 * fVar1;
    param_4[3] = 1.0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

