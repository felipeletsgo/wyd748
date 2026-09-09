// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00521d53 | Name: FUN_00521d53


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00521d53(int param_1,int param_2,int param_3,char param_4)

{
  undefined4 uVar1;
  float10 fVar2;
  int local_18;
  undefined4 local_10;
  undefined4 local_c;
  float local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    uVar1 = FUN_00430f20(0,0);
    FUN_0040bd30(uVar1);
    if (param_3 != 0) {
      FUN_00493c60(&local_10,param_1 + 0x28);
      *(undefined4 *)(param_1 + 0x7c8) = 0x3f000000;
      local_8 = *(float *)(&DAT_0092e7d0 + *(int *)(param_3 + 0x5c) * 8);
      fVar2 = (float10)FUN_004d3cd0(*(float *)(&DAT_0092e7d4 + *(int *)(param_3 + 0x5c) * 8) *
                                    *(float *)(&DAT_0092e7d4 + *(int *)(param_3 + 0x5c) * 8) +
                                    local_8 * local_8);
      *(float *)(param_1 + 0x7c8) =
           (float)(fVar2 * (float10)*(float *)(param_3 + 0x38) * (float10)_DAT_005a40ec);
      if (_DAT_005a365c < *(float *)(param_1 + 0x7c8)) {
        *(undefined4 *)(param_1 + 0x7c8) = 0x40000000;
      }
      if ((param_3 != param_1) && (*(int *)(param_1 + 0x24c) != 0x2c)) {
        fVar2 = (float10)FUN_00493c30(local_10,local_c);
        *(float *)(param_1 + 0x22c) = (float)(fVar2 + (float10)_DAT_005a4380);
      }
      *(int *)(param_1 + 0x75c) = (int)param_4;
      if ((param_4 < '\0') || ('g' < param_4)) {
        if (param_2 != *(int *)(param_1 + 0x740)) {
          if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
            FUN_0051dba4(param_2,0);
          }
          else {
            FUN_0051dba4(param_2,0);
          }
          *(undefined4 *)(param_1 + 0x75c) = 0xffffffff;
        }
        if ((*(int *)(param_3 + 0x24c) == 0x38) && (*(short *)(param_3 + 0x1d2) == 0)) {
          uVar1 = FUN_00430f20(*(float *)(param_3 + 0x28) * _DAT_005a4154 +
                               *(float *)(param_1 + 0x28) * _DAT_005a414c,
                               *(float *)(param_3 + 0x2c) * _DAT_005a4154 +
                               *(float *)(param_1 + 0x2c) * _DAT_005a414c);
          FUN_0040bd30(uVar1);
        }
        else {
          uVar1 = FUN_00430f20(*(float *)(param_3 + 0x28) * _DAT_005a415c +
                               *(float *)(param_1 + 0x28) * _DAT_005a4158,
                               *(float *)(param_3 + 0x2c) * _DAT_005a415c +
                               *(float *)(param_1 + 0x2c) * _DAT_005a4158);
          FUN_0040bd30(uVar1);
        }
      }
      else {
        *(undefined4 *)(param_1 + 0x760) = 1;
        *(undefined4 *)(param_1 + 0x754) = 0;
        *(undefined4 *)(param_1 + 0x758) = 0;
        for (local_18 = 0; local_18 < 4; local_18 = local_18 + 1) {
          if (*(int *)(param_1 + 0x5c) == 1) {
            if (*(char *)(param_1 + 0x79c) == '\x01') {
              *(uint *)(param_1 + 0x744 + local_18 * 4) =
                   (byte)(&DAT_0092ff33)[local_18 + param_4 * 0x60] - 1;
            }
            else {
              *(uint *)(param_1 + 0x744 + local_18 * 4) =
                   (byte)(&DAT_0092ff30)[local_18 + param_4 * 0x60] - 1;
            }
            if (0 < *(int *)(param_1 + 0x744 + local_18 * 4)) {
              *(int *)(param_1 + 0x758) = *(int *)(param_1 + 0x758) + 1;
            }
          }
          else {
            if (*(char *)(param_1 + 0x79c) == '\x01') {
              *(uint *)(param_1 + 0x744 + local_18 * 4) =
                   (byte)(&DAT_0092ff2b)[local_18 + param_4 * 0x60] - 1;
            }
            else {
              *(uint *)(param_1 + 0x744 + local_18 * 4) =
                   (byte)(&DAT_0092ff28)[local_18 + param_4 * 0x60] - 1;
            }
            if (0 < *(int *)(param_1 + 0x744 + local_18 * 4)) {
              *(int *)(param_1 + 0x758) = *(int *)(param_1 + 0x758) + 1;
            }
          }
        }
        *(undefined4 *)(param_1 + 0x750) = 0xffffffff;
        FUN_0051dba4(1,1);
        FUN_0051dba4(*(undefined4 *)(param_1 + 0x744 + *(int *)(param_1 + 0x754) * 4),0);
        *(undefined4 *)(param_1 + 0x75c) = 0xffffffff;
      }
    }
  }
  return;
}

