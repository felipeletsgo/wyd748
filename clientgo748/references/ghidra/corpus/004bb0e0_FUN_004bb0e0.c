// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bb0e0 | Name: FUN_004bb0e0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004bb0e0(int param_1,char param_2,int param_3)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  uint local_18;
  float local_10;
  int local_c;
  uint local_8;
  
  if ((((*(int *)(param_1 + 0x318) != 0) && (*(int *)(param_1 + 0x31c) != 0)) &&
      (iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x14c))
                         (DAT_005ccf9c[0xa8f9],0,*(undefined4 *)(param_1 + 0x318),
                          *(undefined4 *)(param_1 + 0xc)), -1 < iVar2)) &&
     ((iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x154))
                         (DAT_005ccf9c[0xa8f9],*(undefined4 *)(param_1 + 0x31c),0), -1 < iVar2 &&
      (iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))
                         (DAT_005ccf9c[0xa8f9],*(undefined4 *)(param_1 + 0x10)), -1 < iVar2)))) {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x8c,3);
    if ((param_2 != '\0') && (*(int *)(param_1 + 0x10) == 0x212)) {
      uVar3 = (**(code **)(*DAT_0092e654 + 8))();
      if (param_2 == 'd') {
        local_10 = (float)(uVar3 % 8000) / _DAT_005a43fc;
      }
      else {
        local_10 = (float)(uVar3 % 4000) / _DAT_005a4420;
      }
      local_c = 0;
      iVar2 = (**(code **)(**(int **)(param_1 + 0x318) + 0x2c))
                        (*(undefined4 *)(param_1 + 0x318),0,0,&local_c,0);
      if (iVar2 < 0) {
        return 1;
      }
      for (local_18 = 0; local_18 < *(uint *)(param_1 + 0x14); local_18 = local_18 + 1) {
        iVar2 = *(int *)(param_1 + 0x24 + local_18 * 0x14);
        iVar1 = *(int *)(param_1 + 0x28 + local_18 * 0x14);
        local_24 = iVar2;
        if (param_2 == '\x01') {
          for (; local_24 < iVar2 + iVar1; local_24 = local_24 + 1) {
            *(float *)(local_c + 0x20 + local_24 * 0x28) =
                 local_10 + *(float *)(local_c + 0x18 + local_24 * 0x28);
            *(float *)(local_c + 0x24 + local_24 * 0x28) =
                 local_10 + *(float *)(local_c + 0x1c + local_24 * 0x28);
          }
        }
        else {
          local_28 = iVar2;
          if (param_2 == '\x02') {
            for (; local_28 < iVar2 + iVar1; local_28 = local_28 + 1) {
              *(float *)(local_c + 0x20 + local_28 * 0x28) =
                   local_10 + *(float *)(local_c + 0x18 + local_28 * 0x28);
            }
          }
          else {
            local_2c = iVar2;
            if (param_2 == '\x03') {
              for (; local_2c < iVar2 + iVar1; local_2c = local_2c + 1) {
                *(float *)(local_c + 0x24 + local_2c * 0x28) =
                     local_10 + *(float *)(local_c + 0x1c + local_2c * 0x28);
              }
            }
            else {
              local_30 = iVar2;
              if (param_2 == '\x04') {
                for (; local_30 < iVar2 + iVar1; local_30 = local_30 + 1) {
                  *(float *)(local_c + 0x20 + local_30 * 0x28) =
                       *(float *)(local_c + 0x18 + local_30 * 0x28) - local_10;
                }
              }
              else {
                local_34 = iVar2;
                if (param_2 == '\x05') {
                  for (; local_34 < iVar2 + iVar1; local_34 = local_34 + 1) {
                    *(float *)(local_c + 0x24 + local_34 * 0x28) =
                         *(float *)(local_c + 0x1c + local_34 * 0x28) - local_10;
                  }
                }
                else if (param_2 == '\x06') {
                  local_3c = iVar2;
                  local_38 = iVar2;
                  if (*(int *)(param_1 + 0x35c) == 1) {
                    for (; local_38 < iVar2 + iVar1; local_38 = local_38 + 1) {
                      *(undefined4 *)(local_c + 0x24 + local_38 * 0x28) =
                           *(undefined4 *)(local_c + 0x1c + local_38 * 0x28);
                    }
                    *(undefined4 *)(param_1 + 0x35c) = 0;
                  }
                  else {
                    for (; local_3c < iVar2 + iVar1; local_3c = local_3c + 1) {
                      *(float *)(local_c + 0x1c + local_3c * 0x28) =
                           *(float *)(local_c + 0x24 + local_3c * 0x28) - local_10;
                    }
                  }
                }
                else {
                  local_40 = iVar2;
                  if (param_2 == '\a') {
                    for (; local_40 < iVar2 + iVar1; local_40 = local_40 + 1) {
                      *(float *)(local_c + 0x18 + local_40 * 0x28) =
                           *(float *)(local_c + 0x20 + local_40 * 0x28) - local_10 * _DAT_005a40f0;
                    }
                  }
                  else {
                    local_44 = iVar2;
                    if (param_2 == 'd') {
                      for (; local_44 < iVar2 + iVar1; local_44 = local_44 + 1) {
                        *(float *)(local_c + 0x18 + local_44 * 0x28) =
                             *(float *)(local_c + 0x20 + local_44 * 0x28) - (local_10 + local_10);
                      }
                    }
                    else if (param_2 == 'e') {
                      local_4c = iVar2;
                      local_48 = iVar2;
                      if (*(int *)(param_1 + 0x35c) == 1) {
                        for (; local_48 < iVar2 + iVar1; local_48 = local_48 + 1) {
                          *(undefined4 *)(local_c + 0x24 + local_48 * 0x28) =
                               *(undefined4 *)(local_c + 0x1c + local_48 * 0x28);
                        }
                        *(undefined4 *)(param_1 + 0x35c) = 0;
                      }
                      else {
                        for (; local_4c < iVar2 + iVar1; local_4c = local_4c + 1) {
                          *(float *)(local_c + 0x1c + local_4c * 0x28) =
                               *(float *)(local_c + 0x24 + local_4c * 0x28) - local_10;
                          *(float *)(local_c + 0x18 + local_4c * 0x28) =
                               local_10 + *(float *)(local_c + 0x20 + local_4c * 0x28);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      iVar2 = (**(code **)(**(int **)(param_1 + 0x318) + 0x30))(*(undefined4 *)(param_1 + 0x318));
      if (iVar2 < 0) {
        return 1;
      }
    }
    for (local_8 = 0; local_8 < *(uint *)(param_1 + 0x14); local_8 = local_8 + 1) {
      if (*(int *)(param_1 + 4) == 1) {
        if (*(uint *)(param_1 + 0x324) < 0x57e41) {
          uVar4 = FUN_004b925c(*(int *)(param_1 + 0x298 + local_8 * 4) + param_3,360000);
          (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar4);
        }
        else {
          uVar4 = FUN_004b925c(*(int *)(param_1 + 0x298 + local_8 * 4) + param_3,
                               *(undefined4 *)(param_1 + 0x324));
          (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar4);
        }
      }
      else if (*(uint *)(param_1 + 0x324) < 0x2bf21) {
        uVar4 = FUN_004b9a4b(*(int *)(param_1 + 0x298 + local_8 * 4) + param_3,180000);
        (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar4);
      }
      else {
        uVar4 = FUN_004b9a4b(*(int *)(param_1 + 0x298 + local_8 * 4) + param_3,
                             *(undefined4 *)(param_1 + 0x324));
        (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar4);
      }
      iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                        (DAT_005ccf9c[0xa8f9],4,*(undefined4 *)(param_1 + 0x24 + local_8 * 0x14),
                         *(undefined4 *)(param_1 + 0x28 + local_8 * 0x14),
                         *(int *)(param_1 + 0x1c + local_8 * 0x14) * 3,
                         *(undefined4 *)(param_1 + 0x20 + local_8 * 0x14));
      if (iVar2 < 0) {
        return 0;
      }
    }
  }
  return 1;
}

