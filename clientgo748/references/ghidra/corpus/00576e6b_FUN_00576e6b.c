// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00576e6b | Name: FUN_00576e6b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00576e6b(undefined4 *param_1,int param_2)

{
  uint *puVar1;
  float fVar2;
  undefined4 *puVar3;
  int iVar4;
  uint extraout_EDX;
  uint uVar5;
  float *pfVar6;
  float local_54 [16];
  int local_14;
  int local_10;
  float local_c;
  float *local_8;
  
  puVar3 = param_1;
  *param_1 = 0;
  param_1[1] = 0;
  pfVar6 = local_54;
  for (iVar4 = 0x10; iVar4 != 0; iVar4 = iVar4 + -1) {
    *pfVar6 = 0.0;
    pfVar6 = pfVar6 + 1;
  }
  FUN_005759c5();
  local_8 = (float *)(param_2 + 0xc);
  param_1 = (undefined4 *)extraout_EDX;
  do {
    local_c = local_54[(int)param_1] + *local_8;
    local_14 = (int)ROUND(local_c * _DAT_005a6978 + _DAT_005a67d4);
    puVar1 = puVar3 + ((uint)param_1 >> 3);
    local_10 = local_14;
    fVar2 = (float)local_14;
    *puVar1 = *puVar1 >> 4 | local_14 << 0x1c;
    if (local_14 < 0) {
      fVar2 = fVar2 + _DAT_005a67dc;
    }
    uVar5 = (uint)param_1 & 3;
    fVar2 = local_c - fVar2 * _DAT_005a69a8;
    if (uVar5 != 3) {
      local_54[(int)param_1 + 1] = _DAT_005a862c * fVar2 + local_54[(int)param_1 + 1];
    }
    if (param_1 < 0xc) {
      if (uVar5 != 0) {
        local_54[(int)param_1 + 3] = _DAT_005a8628 * fVar2 + local_54[(int)param_1 + 3];
      }
      local_54[(int)param_1 + 4] = _DAT_005a8624 * fVar2 + local_54[(int)param_1 + 4];
      if (uVar5 != 3) {
        local_54[(int)param_1 + 5] = fVar2 * _DAT_005a69c0 + local_54[(int)param_1 + 5];
      }
    }
    param_1 = (undefined4 *)((int)param_1 + 1);
    local_8 = local_8 + 4;
  } while (param_1 < 0x10);
  FUN_00576352(puVar3 + 2,0);
  return;
}

