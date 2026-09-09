// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049da58 | Name: FUN_0049da58


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0049da58(int param_1,float param_2,float param_3,undefined4 param_4)

{
  if (*(int *)(param_1 + 0x40) != 0) {
    if ((((param_2 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a28)) ||
         (*(float *)(*(int *)(param_1 + 0x40) + 0x20a28) + _DAT_005a44a8 <= param_2)) ||
        (param_3 < *(float *)(*(int *)(param_1 + 0x40) + 0x20a2c))) ||
       (*(float *)(*(int *)(param_1 + 0x40) + 0x20a2c) + _DAT_005a44a8 <= param_3)) {
      if (((*(int *)(*(int *)(param_1 + 0x40) + 0xac) == 0) ||
          (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28))) ||
         ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a28) + _DAT_005a44a8 <=
           param_2 ||
          ((param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) ||
           (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xac) + 0x20a2c) + _DAT_005a44a8 <=
            param_3)))))) {
        if (((*(int *)(*(int *)(param_1 + 0x40) + 0xb0) == 0) ||
            (((param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) ||
              (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a28) + _DAT_005a44a8 <=
               param_2)) ||
             (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c))))) ||
           (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb0) + 0x20a2c) + _DAT_005a44a8 <=
            param_3)) {
          if (((*(int *)(*(int *)(param_1 + 0x40) + 0xb4) == 0) ||
              (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28))) ||
             ((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a28) + _DAT_005a44a8 <=
               param_2 ||
              ((param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) ||
               (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb4) + 0x20a2c) + _DAT_005a44a8 <=
                param_3)))))) {
            if ((*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) &&
               ((((*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) <= param_2 &&
                  (param_2 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a28) +
                             _DAT_005a44a8)) &&
                 (*(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) <= param_3)) &&
                (param_3 < *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0xb8) + 0x20a2c) +
                           _DAT_005a44a8)))) {
              FUN_0053a8b9(param_2,param_3,param_4);
            }
          }
          else {
            FUN_0053a8b9(param_2,param_3,param_4);
          }
        }
        else {
          FUN_0053a8b9(param_2,param_3,param_4);
        }
      }
      else {
        FUN_0053a8b9(param_2,param_3,param_4);
      }
    }
    else {
      FUN_0053a8b9(param_2,param_3,param_4);
    }
  }
  return;
}

