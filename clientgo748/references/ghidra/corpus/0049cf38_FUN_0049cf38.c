// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049cf38 | Name: FUN_0049cf38


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0049cf38(int param_1,float param_2,float param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x40) != 0) {
    if ((((*(float *)(*(int *)(param_1 + 0x40) + 0x20a28) <= param_2) &&
         (param_2 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a28) + _DAT_005a44a8)) &&
        (*(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) <= param_3)) &&
       (param_3 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) + _DAT_005a44a8)) {
      uVar1 = FUN_0053976b(param_2,param_3,param_4,param_5);
      return uVar1;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) <= param_2)) &&
       ((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) <= param_3 &&
            (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      uVar1 = FUN_0053976b(param_2,param_3,param_4,param_5);
      return uVar1;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) <= param_2)) &&
        (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) <= param_3 &&
           (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      uVar1 = FUN_0053976b(param_2,param_3,param_4,param_5);
      return uVar1;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) <= param_2)) &&
       ((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) <= param_3 &&
            (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      uVar1 = FUN_0053976b(param_2,param_3,param_4,param_5);
      return uVar1;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) <= param_2)) &&
        (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) <= param_3 &&
           (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      uVar1 = FUN_0053976b(param_2,param_3,param_4,param_5);
      return uVar1;
    }
  }
  return 0;
}

