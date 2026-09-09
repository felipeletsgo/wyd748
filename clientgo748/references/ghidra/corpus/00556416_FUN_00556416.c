// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00556416 | Name: FUN_00556416


void FUN_00556416(int param_1,int param_2,int *param_3,int *param_4,int param_5,int param_6)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_40;
  int local_38;
  int local_30;
  int local_2c;
  int local_24;
  int local_20;
  int local_18;
  int local_14;
  
  if ((((param_1 != *param_3) || (param_2 != *param_4)) && (param_1 != 0)) &&
     (((param_2 != 0 && (*param_3 != 0)) && (*param_4 != 0)))) {
    if (*param_3 < param_1) {
      local_20 = param_1 - *param_3;
    }
    else {
      local_20 = *param_3 - param_1;
    }
    if (*param_4 < param_2) {
      local_24 = param_2 - *param_4;
    }
    else {
      local_24 = *param_4 - param_2;
    }
    iVar1 = FUN_005563a0(param_1,param_2,*param_3,*param_4);
    if (1 < iVar1) {
      if (iVar1 < 0x1f) {
        if (local_24 < local_20) {
          if (*param_3 != param_1) {
            iVar1 = ((*param_4 - param_2) * 1000) / (*param_3 - param_1);
            iVar4 = param_2 * 1000 - iVar1 * param_1;
            if (param_1 < *param_3) {
              local_18 = 1;
            }
            else {
              local_18 = -1;
            }
            param_1 = param_1 + local_18;
            local_14 = (int)*(char *)(param_5 +
                                     ((((iVar1 * param_1 + iVar4) / 1000 - DAT_005b8928) *
                                       DAT_005b891c + param_1) - DAT_005b8924));
            if (local_14 == 0x7f) {
              *param_3 = 0;
              *param_4 = 0;
            }
            else {
              local_2c = local_20;
              for (local_30 = param_1; local_30 != *param_3; local_30 = local_30 + local_18) {
                if (local_30 != param_1) {
                  iVar2 = (iVar1 * local_30 + iVar4) / 1000;
                  if ((((iVar2 - DAT_005b8928 < 0) || (0xff < iVar2 - DAT_005b8928)) ||
                      (local_30 - DAT_005b8924 < 0)) || (0xff < local_30 - DAT_005b8924)) {
                    *param_3 = 0;
                    *param_4 = 0;
                    return;
                  }
                  iVar5 = (int)*(char *)(param_5 +
                                        (((iVar2 - DAT_005b8928) * DAT_005b891c + local_30) -
                                        DAT_005b8924));
                  if (iVar5 == 0x7f) {
                    *param_3 = 0;
                    *param_4 = 0;
                    return;
                  }
                  if ((local_14 + param_6 < iVar5) ||
                     (iVar3 = local_14 - param_6, local_14 = iVar5, iVar5 < iVar3)) {
                    *param_3 = local_30;
                    *param_4 = iVar2;
                    return;
                  }
                }
                if (local_2c + -1 < 1) {
                  return;
                }
                local_2c = local_2c + -1;
              }
            }
          }
        }
        else if (*param_4 != param_2) {
          iVar1 = ((*param_3 - param_1) * 1000) / (*param_4 - param_2);
          iVar4 = param_1 * 1000 - iVar1 * param_2;
          if (param_2 < *param_4) {
            local_18 = 1;
          }
          else {
            local_18 = -1;
          }
          param_2 = param_2 + local_18;
          local_14 = (int)*(char *)(param_5 +
                                   (((param_2 - DAT_005b8928) * DAT_005b891c +
                                    (iVar1 * param_2 + iVar4) / 1000) - DAT_005b8924));
          if (local_14 == 0x7f) {
            *param_3 = 0;
            *param_4 = 0;
          }
          else {
            local_38 = local_24;
            for (local_40 = param_2; local_40 != *param_4; local_40 = local_40 + local_18) {
              if (local_40 != param_2) {
                iVar2 = (iVar1 * local_40 + iVar4) / 1000;
                if (((local_40 - DAT_005b8928 < 0) || (0xff < local_40 - DAT_005b8928)) ||
                   ((iVar2 - DAT_005b8924 < 0 || (0xff < iVar2 - DAT_005b8924)))) {
                  *param_3 = 0;
                  *param_4 = 0;
                  return;
                }
                iVar5 = (int)*(char *)(param_5 +
                                      (((local_40 - DAT_005b8928) * DAT_005b891c + iVar2) -
                                      DAT_005b8924));
                if (iVar5 == 0x7f) {
                  *param_3 = 0;
                  *param_4 = 0;
                  return;
                }
                if ((local_14 + param_6 < iVar5) ||
                   (iVar3 = local_14 - param_6, local_14 = iVar5, iVar5 < iVar3)) {
                  *param_3 = iVar2;
                  *param_4 = local_40;
                  return;
                }
              }
              if (local_38 + -1 < 1) {
                return;
              }
              local_38 = local_38 + -1;
            }
          }
        }
      }
      else {
        *param_3 = 0;
        *param_4 = 0;
      }
    }
  }
  return;
}

