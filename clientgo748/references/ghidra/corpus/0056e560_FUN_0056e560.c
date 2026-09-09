// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056e560 | Name: FUN_0056e560


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056e560(int param_1,int param_2,int param_3,float *param_4)

{
  char *pcVar1;
  float fVar2;
  char *pcVar3;
  
  fVar2 = _DAT_005a69bc;
  pcVar3 = (char *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  pcVar1 = pcVar3 + *(int *)(param_1 + 0x1058) * 2;
  for (; pcVar3 < pcVar1; pcVar3 = pcVar3 + 2) {
    *param_4 = (float)(int)*pcVar3 * fVar2;
    param_4[1] = (float)(int)pcVar3[1] * fVar2;
    param_4[2] = 0.0;
    param_4[3] = 1.0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

