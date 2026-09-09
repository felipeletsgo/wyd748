// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049cb58 | Name: FUN_0049cb58


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_0049cb58(int param_1,undefined4 *param_2,float param_3,float param_4)

{
  if (*(int *)(param_1 + 0x40) == 0) {
    *param_2 = 0x3f800000;
    param_2[1] = 0x3f800000;
    param_2[2] = 0x3f800000;
    param_2[3] = 0x3f800000;
    return param_2;
  }
  if (*(int *)(param_1 + 0x40) != 0) {
    if ((((*(float *)(*(int *)(param_1 + 0x40) + 0x20a28) <= param_3) &&
         (param_3 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a28) + _DAT_005a44a8)) &&
        (*(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) <= param_4)) &&
       (param_4 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) + _DAT_005a44a8)) {
      FUN_0053a4de(param_2,param_3,param_4);
      return param_2;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) <= param_3)) &&
       ((param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) <= param_4 &&
            (param_4 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      FUN_0053a4de(param_2,param_3,param_4);
      return param_2;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) <= param_3)) &&
        (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) <= param_4 &&
           (param_4 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      FUN_0053a4de(param_2,param_3,param_4);
      return param_2;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) <= param_3)) &&
       ((param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) <= param_4 &&
            (param_4 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      FUN_0053a4de(param_2,param_3,param_4);
      return param_2;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) <= param_3)) &&
        (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) <= param_4 &&
           (param_4 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      FUN_0053a4de(param_2,param_3,param_4);
      return param_2;
    }
  }
  *param_2 = 0x3f800000;
  param_2[1] = 0x3f800000;
  param_2[2] = 0x3f800000;
  param_2[3] = 0x3f800000;
  return param_2;
}

