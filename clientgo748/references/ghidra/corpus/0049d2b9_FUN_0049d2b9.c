// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049d2b9 | Name: FUN_0049d2b9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0049d2b9(int param_1,float param_2,float param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x40) != 0) {
    if ((((*(float *)(*(int *)(param_1 + 0x40) + 0x20a28) <= param_2) &&
         (param_2 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a28) + _DAT_005a44a8)) &&
        (*(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) <= param_3)) &&
       (param_3 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) + _DAT_005a44a8)) {
      iVar1 = FUN_0049c70f(param_2,param_3);
      uVar2 = FUN_0053976b(param_2,param_3,(float)iVar1 * _DAT_005a4214,param_4);
      return uVar2;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) <= param_2)) &&
       ((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) <= param_3 &&
            (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      iVar1 = FUN_0049c70f(param_2,param_3);
      uVar2 = FUN_0053976b(param_2,param_3,(float)iVar1 * _DAT_005a4214,param_4);
      return uVar2;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) <= param_2)) &&
        (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) <= param_3 &&
           (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      iVar1 = FUN_0049c70f(param_2,param_3);
      uVar2 = FUN_0053976b(param_2,param_3,(float)iVar1 * _DAT_005a4214,param_4);
      return uVar2;
    }
    if (((*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) &&
        (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) <= param_2)) &&
       ((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) + _DAT_005a44a8
        && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) <= param_3 &&
            (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) +
                       _DAT_005a44a8)))))) {
      iVar1 = FUN_0049c70f(param_2,param_3);
      uVar2 = FUN_0053976b(param_2,param_3,(float)iVar1 * _DAT_005a4214,param_4);
      return uVar2;
    }
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) &&
         (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) <= param_2)) &&
        (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) + _DAT_005a44a8))
       && ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) <= param_3 &&
           (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) +
                      _DAT_005a44a8)))) {
      iVar1 = FUN_0049c70f(param_2,param_3);
      uVar2 = FUN_0053976b(param_2,param_3,(float)iVar1 * _DAT_005a4214,param_4);
      return uVar2;
    }
  }
  return 0;
}

