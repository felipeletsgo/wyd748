// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049d6eb | Name: FUN_0049d6eb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall FUN_0049d6eb(int param_1,float param_2,float param_3,undefined4 param_4)

{
  float10 fVar1;
  
  if (*(int *)(param_1 + 0x40) != 0) {
    if ((((*(float *)(*(int *)(param_1 + 0x40) + 0x20a28) <= param_2) &&
         (param_2 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a28) + _DAT_005a44a8)) &&
        (*(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) <= param_3)) &&
       (param_3 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) + _DAT_005a44a8)) {
      fVar1 = (float10)FUN_005396dc(param_2,param_3,param_4);
      return fVar1;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) <= param_2)) &&
       ((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) <= param_3 &&
            (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      fVar1 = (float10)FUN_005396dc(param_2,param_3,param_4);
      return fVar1;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) <= param_2)) &&
        (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) <= param_3 &&
           (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      fVar1 = (float10)FUN_005396dc(param_2,param_3,param_4);
      return fVar1;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) <= param_2)) &&
       ((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) <= param_3 &&
            (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      fVar1 = (float10)FUN_005396dc(param_2,param_3,param_4);
      return fVar1;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) <= param_2)) &&
        (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) <= param_3 &&
           (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      fVar1 = (float10)FUN_005396dc(param_2,param_3,param_4);
      return fVar1;
    }
  }
  return (float10)_DAT_005a44b0;
}

