// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00500c02 | Name: FUN_00500c02


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00500c02(int param_1,float param_2,undefined4 param_3,float param_4)

{
  float10 fVar1;
  float local_c;
  float local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    *(float *)(param_1 + 0x28) = param_2;
    *(float *)(param_1 + 0x2c) = param_4;
    *(undefined4 *)(param_1 + 0x30) = param_3;
    local_8 = *(float *)(&DAT_0092e7d0 + *(int *)(param_1 + 0x5c) * 8) * *(float *)(param_1 + 0x38);
    if (*(char *)(param_1 + 0x79c) != '\0') {
      local_8 = *(float *)(&DAT_0092e7d0 + *(int *)(param_1 + 0x7a0) * 8) *
                *(float *)(param_1 + 0x38);
    }
    local_8 = local_8 / _DAT_005a365c;
    fVar1 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x34));
    local_c = (float)(fVar1 * (float10)local_8);
    fVar1 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x34));
    if (*(char *)(param_1 + 0x79c) == '\0') {
      if (*(int *)(param_1 + 0x24c) == 0x2c) {
        local_c = local_c + _DAT_005a34a0;
      }
      if (*(int *)(param_1 + 0x60) != 0) {
        FUN_004be17b(param_2 + local_c,param_3,param_4 - (float)(fVar1 * (float10)local_8));
      }
    }
    else {
      if (*(int *)(param_1 + 0x19c) != 0) {
        FUN_004be17b(param_2 + local_c,param_3,param_4 - (float)(fVar1 * (float10)local_8));
      }
      if (*(int *)(param_1 + 0x60) != 0) {
        FUN_004be17b(0,0,0);
      }
    }
  }
  return;
}

