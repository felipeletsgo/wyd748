// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056d4b7 | Name: FUN_0056d4b7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056d4b7(int param_1,uint param_2,uint param_3,int param_4)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  uint *extraout_EDX;
  uint *puVar5;
  uint local_c;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar3 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  local_c = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar4 = 0;
    puVar5 = extraout_EDX;
    do {
      fVar1 = *(float *)(iVar3 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (local_c & 3) * 4);
      fVar2 = *(float *)(iVar4 + 0xc + param_4);
      *puVar5 = (((int)ROUND(*(float *)(iVar4 + 8 + param_4) * _DAT_005a699c + fVar1) & 0x3ffU |
                 (int)ROUND(fVar2 + fVar2 + fVar1) << 10) << 10 |
                (int)ROUND(*(float *)(iVar4 + 4 + param_4) * _DAT_005a699c + fVar1) & 0x3ffU) << 10
                | (int)ROUND(*(float *)(iVar4 + param_4) * _DAT_005a699c + fVar1) & 0x3ffU;
      puVar5 = puVar5 + 1;
      local_c = local_c + 1;
      iVar4 = iVar4 + 0x10;
    } while (local_c < *(uint *)(param_1 + 0x1058));
  }
  return;
}

