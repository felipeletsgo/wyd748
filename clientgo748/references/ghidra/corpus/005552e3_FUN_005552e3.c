// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005552e3 | Name: FUN_005552e3


undefined4
FUN_005552e3(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7,
            int *param_8)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int local_18;
  int local_10;
  int local_c;
  int local_8;
  
  *param_8 = 0;
  local_8 = 0;
  if ((param_1 < 10) && (-1 < param_1)) {
    if ((param_2 == 1) && ((param_3 == 3 || (param_3 == 2)))) {
      local_8 = 1;
    }
    if ((param_3 == 1) && ((param_2 == 3 || (param_2 == 2)))) {
      local_8 = -1;
    }
    if (local_8 == 0) {
      uVar3 = 0;
    }
    else {
      for (local_c = 0; local_c < 6; local_c = local_c + 1) {
        for (local_10 = 0; local_10 < 6; local_10 = local_10 + 1) {
          if (*(int *)(&DAT_005bed50 +
                      local_10 * 4 + local_c * 0x18 + param_7 * 0x90 + param_1 * 0x240) != 0) {
            iVar1 = param_4 + -2 + local_10;
            iVar2 = param_5 + -2 + local_c;
            if ((((iVar1 - DAT_005b8924 < 1) || (iVar2 - DAT_005b8928 < 1)) ||
                (DAT_005b891c + -2 < iVar1 - DAT_005b8924)) ||
               (DAT_005b8920 + -2 < iVar2 - DAT_005b8928)) break;
            local_18 = (int)*(char *)(param_6 +
                                     (((iVar2 - DAT_005b8928) * DAT_005b891c + iVar1) - DAT_005b8924
                                     )) +
                       *(int *)(&DAT_005bed50 +
                               local_10 * 4 + local_c * 0x18 + param_7 * 0x90 + param_1 * 0x240) *
                       local_8;
            if (0x7f < local_18) {
              local_18 = 0x7f;
            }
            if (local_18 < -0x80) {
              local_18 = -0x80;
            }
            if (*(int *)(&DAT_005bed50 +
                        local_10 * 4 + local_c * 0x18 + param_7 * 0x90 + param_1 * 0x240) != 0) {
              *param_8 = local_18;
            }
            *(undefined1 *)
             (param_6 + (((iVar2 - DAT_005b8928) * DAT_005b891c + iVar1) - DAT_005b8924)) =
                 (undefined1)local_18;
          }
        }
      }
      uVar3 = 1;
    }
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

