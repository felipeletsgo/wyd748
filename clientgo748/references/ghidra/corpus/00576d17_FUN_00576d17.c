// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00576d17 | Name: FUN_00576d17


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00576d17(int param_1,byte *param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  uint uVar4;
  float *pfVar5;
  uint uVar6;
  float local_24 [8];
  
  iVar3 = FUN_00576b56(param_1,param_2 + 8);
  if (-1 < iVar3) {
    local_24[0] = (float)*param_2 * _DAT_005a68b0;
    local_24[1] = (float)param_2[1] * _DAT_005a68b0;
    if (param_2[1] < *param_2) {
      uVar4 = 1;
      do {
        fVar1 = (float)(int)(7 - uVar4);
        if ((int)(7 - uVar4) < 0) {
          fVar1 = fVar1 + _DAT_005a67dc;
        }
        fVar2 = (float)(int)uVar4;
        if ((int)uVar4 < 0) {
          fVar2 = fVar2 + _DAT_005a67dc;
        }
        uVar4 = uVar4 + 1;
        local_24[uVar4] = (fVar2 * local_24[1] + fVar1 * local_24[0]) * _DAT_005a69b8;
      } while (uVar4 < 7);
    }
    else {
      uVar4 = 1;
      do {
        fVar1 = (float)(int)(5 - uVar4);
        if ((int)(5 - uVar4) < 0) {
          fVar1 = fVar1 + _DAT_005a67dc;
        }
        fVar2 = (float)(int)uVar4;
        if ((int)uVar4 < 0) {
          fVar2 = fVar2 + _DAT_005a67dc;
        }
        uVar4 = uVar4 + 1;
        local_24[uVar4] = (fVar2 * local_24[1] + fVar1 * local_24[0]) * _DAT_005a8630;
      } while (uVar4 < 5);
      local_24[6] = 0.0;
      local_24[7] = 1.0;
    }
    iVar3 = 8;
    uVar4 = (uint)*(uint3 *)(param_2 + 2);
    pfVar5 = (float *)(param_1 + 0xc);
    do {
      uVar6 = uVar4 & 7;
      uVar4 = uVar4 >> 3;
      *pfVar5 = local_24[uVar6];
      pfVar5 = pfVar5 + 4;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    iVar3 = 8;
    uVar4 = (uint)*(uint3 *)(param_2 + 5);
    pfVar5 = (float *)(param_1 + 0x8c);
    do {
      uVar6 = uVar4 & 7;
      uVar4 = uVar4 >> 3;
      *pfVar5 = local_24[uVar6];
      pfVar5 = pfVar5 + 4;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    iVar3 = 0;
  }
  return iVar3;
}

