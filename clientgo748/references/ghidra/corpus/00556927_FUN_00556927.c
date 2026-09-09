// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00556927 | Name: FUN_00556927


void FUN_00556927(int param_1,int param_2,int *param_3,int *param_4,int param_5,int param_6)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_38;
  int local_34;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  
  if ((((param_1 != *param_3) || (param_2 != *param_4)) && (param_1 != 0)) &&
     (((param_2 != 0 && (*param_3 != 0)) && (*param_4 != 0)))) {
    if (*param_3 < param_1) {
      local_1c = param_1 - *param_3;
    }
    else {
      local_1c = *param_3 - param_1;
    }
    if (*param_4 < param_2) {
      local_20 = param_2 - *param_4;
    }
    else {
      local_20 = *param_4 - param_2;
    }
    local_14 = (int)*(char *)(param_5 +
                             (((param_2 - DAT_005b8928) * DAT_005b891c + param_1) - DAT_005b8924));
    if (local_20 < local_1c) {
      if (param_1 < *param_3) {
        local_18 = 1;
      }
      else {
        local_18 = -1;
      }
      iVar1 = ((*param_4 - param_2) * 1000) / (*param_3 - param_1);
      local_24 = local_1c;
      for (local_28 = param_1; local_28 - local_18 != *param_3; local_28 = local_28 + local_18) {
        if (local_28 != param_1) {
          iVar2 = (iVar1 * local_28 + (param_2 * 1000 - iVar1 * param_1)) / 1000;
          if ((((iVar2 - DAT_005b8928 < 0) || (0xff < iVar2 - DAT_005b8928)) ||
              (local_28 - DAT_005b8924 < 0)) || (0xff < local_28 - DAT_005b8924)) {
            *param_3 = 0;
            *param_4 = 0;
            return;
          }
          iVar2 = (int)*(char *)(param_5 +
                                (((iVar2 - DAT_005b8928) * DAT_005b891c + local_28) - DAT_005b8924))
          ;
          if ((local_14 + param_6 < iVar2) ||
             (iVar3 = local_14 - param_6, local_14 = iVar2, iVar2 < iVar3)) {
            *param_3 = 0;
            *param_4 = 0;
            return;
          }
        }
        if (local_24 + -1 < 0) {
          return;
        }
        local_24 = local_24 + -1;
      }
    }
    else {
      if (param_2 < *param_4) {
        local_18 = 1;
      }
      else {
        local_18 = -1;
      }
      iVar1 = ((*param_3 - param_1) * 1000) / (*param_4 - param_2);
      local_34 = local_20;
      for (local_38 = param_2; local_38 - local_18 != *param_4; local_38 = local_38 + local_18) {
        if (local_38 == param_2) {
joined_r0x00556cba:
          iVar2 = local_14;
          if (local_34 + -1 < 0) {
            return;
          }
        }
        else {
          iVar2 = (iVar1 * local_38 + (param_1 * 1000 - iVar1 * param_2)) / 1000;
          if (((local_38 - DAT_005b8928 < 0) || (0xff < local_38 - DAT_005b8928)) ||
             ((iVar2 - DAT_005b8924 < 0 || (0xff < iVar2 - DAT_005b8924)))) {
            *param_3 = 0;
            *param_4 = 0;
            return;
          }
          iVar2 = (int)*(char *)(param_5 +
                                (((local_38 - DAT_005b8928) * DAT_005b891c + iVar2) - DAT_005b8924))
          ;
          if (local_38 != param_2) {
            if ((local_14 + param_6 < iVar2) ||
               (iVar3 = local_14 - param_6, local_14 = iVar2, iVar2 < iVar3)) {
              *param_3 = 0;
              *param_4 = 0;
              return;
            }
            goto joined_r0x00556cba;
          }
        }
        local_14 = iVar2;
        local_34 = local_34 + -1;
      }
    }
  }
  return;
}

