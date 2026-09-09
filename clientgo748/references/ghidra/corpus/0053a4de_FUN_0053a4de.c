// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053a4de | Name: FUN_0053a4de


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * __thiscall FUN_0053a4de(int param_1,float *param_2,float param_3,float param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  int local_78;
  float local_54 [6];
  float local_3c;
  float local_34;
  float local_30;
  float local_2c;
  float local_24;
  float local_20;
  float local_1c;
  uint local_14 [4];
  
  iVar5 = __ftol();
  iVar6 = __ftol();
  iVar5 = iVar5 / 2;
  iVar6 = iVar6 / 2;
  if ((((iVar5 < 0) || (0x3e < iVar5)) || (iVar6 < 0)) || (0x3e < iVar6)) {
    if (iVar5 == 0x3f) {
      local_14[0] = *(uint *)(param_1 + 0xd4 + (iVar6 * 0x40 + 0x3f) * 0xc);
      local_14[1] = *(undefined4 *)(param_1 + 0xd4 + (iVar6 * 0x40 + 0x3f) * 0xc);
      local_14[2] = *(undefined4 *)(param_1 + 0xd4 + ((iVar6 + 1) * 0x40 + 0x3f) * 0xc);
      local_14[3] = *(undefined4 *)(param_1 + 0xd4 + ((iVar6 + 1) * 0x40 + 0x3f) * 0xc);
    }
    else if (iVar6 == 0x3f) {
      local_14[0] = *(uint *)(param_1 + 0xd4 + (iVar5 + 0xfc0) * 0xc);
      local_14[1] = *(undefined4 *)(param_1 + 0xd4 + (iVar5 + 0xfc1) * 0xc);
      local_14[2] = *(undefined4 *)(param_1 + 0xd4 + (iVar5 + 0xfc0) * 0xc);
      local_14[3] = *(undefined4 *)(param_1 + 0xd4 + (iVar5 + 0xfc1) * 0xc);
    }
  }
  else {
    local_14[0] = *(uint *)(param_1 + 0xd4 + (iVar5 + iVar6 * 0x40) * 0xc);
    local_14[1] = *(undefined4 *)(param_1 + 0xd4 + (iVar5 + 1 + iVar6 * 0x40) * 0xc);
    local_14[2] = *(undefined4 *)(param_1 + 0xd4 + (iVar5 + (iVar6 + 1) * 0x40) * 0xc);
    local_14[3] = *(undefined4 *)(param_1 + 0xd4 + (iVar5 + 1 + (iVar6 + 1) * 0x40) * 0xc);
  }
  for (local_78 = 0; local_78 < 4; local_78 = local_78 + 1) {
    local_54[local_78 * 4] = (float)((local_14[local_78] & 0xff0000) >> 0x10) / _DAT_005a41e4;
    local_54[local_78 * 4 + 1] = (float)((local_14[local_78] & 0xff00) >> 8) / _DAT_005a41e4;
    local_54[local_78 * 4 + 2] = (float)(local_14[local_78] & 0xff) / _DAT_005a41e4;
  }
  fVar1 = ((float)iVar5 + (float)iVar5) - (param_3 - *(float *)(param_1 + 0x20a28));
  fVar2 = ((float)iVar6 + (float)iVar6) - (param_4 - *(float *)(param_1 + 0x20a2c));
  fVar3 = (((_DAT_005a365c - fVar1) + fVar2) * local_30 +
          ((_DAT_005a365c + fVar1) - fVar2) * local_54[5] +
          ((_DAT_005a3834 - fVar1) - fVar2) * local_54[1] + (fVar1 + fVar2) * local_20) /
          _DAT_005a3e9c;
  fVar4 = (((_DAT_005a365c - fVar1) + fVar2) * local_2c +
          ((_DAT_005a365c + fVar1) - fVar2) * local_3c +
          ((_DAT_005a3834 - fVar1) - fVar2) * local_54[2] + (fVar1 + fVar2) * local_1c) /
          _DAT_005a3e9c;
  *param_2 = (((_DAT_005a365c - fVar1) + fVar2) * local_34 +
             ((_DAT_005a365c + fVar1) - fVar2) * local_54[4] +
             ((_DAT_005a3834 - fVar1) - fVar2) * local_54[0] + (fVar1 + fVar2) * local_24) /
             _DAT_005a3e9c;
  param_2[1] = fVar3;
  param_2[2] = fVar4;
  param_2[3] = 1.0;
  return param_2;
}

