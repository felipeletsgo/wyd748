// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00522126 | Name: FUN_00522126


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00522126(int param_1,int param_2,float param_3,float param_4,char param_5)

{
  undefined4 uVar1;
  float10 fVar2;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    uVar1 = FUN_00430f20(0,0);
    FUN_0040bd30(uVar1);
    *(undefined4 *)(param_1 + 0x7c8) = 0x3f000000;
    FUN_00493c60(&local_c,param_1 + 0x28);
    if (*(int *)(param_1 + 0x24c) != 0x2c) {
      fVar2 = (float10)FUN_00493c30(local_c,local_8);
      *(float *)(param_1 + 0x22c) = (float)(fVar2 + (float10)_DAT_005a4380);
    }
    if ((param_5 < '\0') || ('g' < param_5)) {
      if (param_2 != *(int *)(param_1 + 0x740)) {
        if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
          FUN_0051dba4(param_2,0);
        }
        else {
          FUN_0051dba4(param_2,0);
        }
      }
      uVar1 = FUN_00430f20(param_3 * _DAT_005a415c + *(float *)(param_1 + 0x28) * _DAT_005a4158,
                           param_4 * _DAT_005a415c + *(float *)(param_1 + 0x2c) * _DAT_005a4158);
      FUN_0040bd30(uVar1);
    }
    else {
      *(undefined4 *)(param_1 + 0x760) = 1;
      *(undefined4 *)(param_1 + 0x754) = 0;
      for (local_10 = 0; local_10 < 4; local_10 = local_10 + 1) {
        if (*(int *)(param_1 + 0x5c) == 1) {
          if (*(char *)(param_1 + 0x79c) == '\x01') {
            *(uint *)(param_1 + 0x744 + local_10 * 4) =
                 (byte)(&DAT_0092ff33)[local_10 + param_5 * 0x60] - 1;
          }
          else {
            *(uint *)(param_1 + 0x744 + local_10 * 4) =
                 (byte)(&DAT_0092ff30)[local_10 + param_5 * 0x60] - 1;
          }
        }
        else if (*(char *)(param_1 + 0x79c) == '\x01') {
          *(uint *)(param_1 + 0x744 + local_10 * 4) =
               (byte)(&DAT_0092ff2b)[local_10 + param_5 * 0x60] - 1;
        }
        else {
          *(uint *)(param_1 + 0x744 + local_10 * 4) =
               (byte)(&DAT_0092ff28)[local_10 + param_5 * 0x60] - 1;
        }
      }
      *(undefined4 *)(param_1 + 0x750) = 0xffffffff;
      FUN_0051dba4(*(undefined4 *)(param_1 + 0x744 + *(int *)(param_1 + 0x754) * 4),0);
    }
  }
  return;
}

