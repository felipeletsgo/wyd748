// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056d19d | Name: FUN_0056d19d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056d19d(int param_1,uint param_2,uint param_3,int param_4)

{
  float fVar1;
  int iVar2;
  int iVar3;
  uint *extraout_EDX;
  uint *puVar4;
  uint local_c;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar2 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  local_c = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar3 = 0;
    puVar4 = extraout_EDX;
    do {
      fVar1 = *(float *)(iVar2 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (local_c & 3) * 4);
      *puVar4 = (((int)ROUND(*(float *)(iVar3 + 8 + param_4) * _DAT_005a698c + fVar1) & 0xffU |
                 (int)ROUND(*(float *)(iVar3 + 0xc + param_4) * _DAT_005a698c + fVar1) << 8) << 8 |
                (int)ROUND(*(float *)(iVar3 + 4 + param_4) * _DAT_005a698c + fVar1) & 0xffU) << 8 |
                (int)ROUND(*(float *)(iVar3 + param_4) * _DAT_005a698c + fVar1) & 0xffU;
      puVar4 = puVar4 + 1;
      local_c = local_c + 1;
      iVar3 = iVar3 + 0x10;
    } while (local_c < *(uint *)(param_1 + 0x1058));
  }
  return;
}

