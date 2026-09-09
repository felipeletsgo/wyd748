// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056ca35 | Name: FUN_0056ca35


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056ca35(int param_1,uint param_2,uint param_3,float *param_4)

{
  int iVar1;
  int iVar2;
  float *pfVar3;
  ushort *extraout_EDX;
  ushort *puVar4;
  uint uVar5;
  uint local_14;
  int local_10;
  uint local_c;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = (float *)FUN_0056b152(param_4);
  }
  iVar2 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar5 = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    local_10 = 0;
    puVar4 = extraout_EDX;
    do {
      local_14 = 0;
      local_c = 0;
      pfVar3 = (float *)(param_1 + 0x38);
      do {
        if ((*param_4 - pfVar3[-2]) * (*param_4 - pfVar3[-2]) +
            (param_4[1] - pfVar3[-1]) * (param_4[1] - pfVar3[-1]) +
            (param_4[2] - *pfVar3) * (param_4[2] - *pfVar3) < _DAT_005a6988) {
          local_14 = local_c;
        }
        local_c = local_c + 1;
        pfVar3 = pfVar3 + 4;
      } while (local_c < 0x100);
      iVar1 = local_10 + 0xc;
      local_10 = local_10 + 0x10;
      *puVar4 = (ushort)(byte)(int)ROUND(*(float *)(iVar1 + (int)param_4) * _DAT_005a696c +
                                         *(float *)(iVar2 + ((param_3 & 3) + (param_2 & 3) * 8) * 4
                                                   + (uVar5 & 3) * 4)) << 8 | (ushort)local_14;
      puVar4 = puVar4 + 1;
      uVar5 = uVar5 + 1;
    } while (uVar5 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

