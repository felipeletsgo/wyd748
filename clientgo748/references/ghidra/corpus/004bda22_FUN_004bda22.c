// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bda22 | Name: FUN_004bda22


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004bda22(int *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  float *pfVar3;
  undefined4 *puVar4;
  float *pfVar5;
  float *local_134;
  int local_130;
  float *local_128;
  float *local_124;
  int local_120;
  undefined1 local_11c [16];
  int local_10c;
  undefined4 local_108 [12];
  float local_d8 [4];
  float local_c8 [12];
  float local_98 [4];
  int local_88;
  int local_84;
  int local_80;
  float *local_7c;
  float local_78 [16];
  uint local_38;
  float *local_34;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  uint local_10;
  int local_c;
  float *local_8;
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  if (param_1[0xb9] == 0) {
    param_1[0xb9] = 0x1e;
  }
  if ((*param_1 < 0x65) && (-1 < *param_1)) {
    local_28 = *(int *)(&DAT_0065a4a8 + param_1[0xb4] * 4 + *param_1 * 900);
    if (local_28 != 0) {
      local_10 = ((uint)((iVar1 + param_1[0xb7]) - param_1[0xba]) / (uint)param_1[0xb9]) %
                 (uint)(local_28 << 2);
      param_1[0xb8] = (int)(local_10 + ((int)local_10 >> 0x1f & 3U)) >> 2;
      local_18 = param_1[0xb3] + param_1[0xb8];
      local_14 = *(int *)(&DAT_0065a790 + *param_1 * 900);
      local_1c = (param_1[0xb3] + param_1[0xb8]) * *(int *)(&DAT_0065a790 + *param_1 * 900);
      local_c = *(int *)(&DAT_0065a790 + *param_1 * 900);
      if ((local_c < 0x65) && (-1 < local_c)) {
        if ((local_28 == 1) ||
           ((DAT_005bab20 == 0 || (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a3670)))) {
          for (local_2c = 0; local_2c < local_c; local_2c = local_2c + 1) {
            if (param_1[local_2c + 0x2e] != 0) {
              puVar2 = (undefined4 *)
                       (*(int *)(&DAT_0065a794 + *param_1 * 900) + (local_1c + local_2c) * 0x40);
              puVar4 = (undefined4 *)(param_1[local_2c + 0x2e] + 8);
              for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                *puVar4 = *puVar2;
                puVar2 = puVar2 + 1;
                puVar4 = puVar4 + 1;
              }
            }
          }
          param_1[1] = 1;
        }
        else {
          local_24 = local_28 * 4 + -3;
          for (local_20 = 0; local_20 < local_c; local_20 = local_20 + 1) {
            if (param_1[local_20 + 0x2e] != 0) {
              local_34 = (float *)(*(int *)(&DAT_0065a794 + *param_1 * 900) +
                                  (local_1c + local_20) * 0x40);
              if ((param_1[0xb5] == 0) || (9 < (int)local_10)) {
                param_1[0xb5] = 0;
                local_38 = local_10 & 0x80000003;
                if ((int)local_38 < 0) {
                  local_38 = (local_38 - 1 | 0xfffffffc) + 1;
                }
                if (local_38 == 0) {
                  pfVar3 = (float *)(param_1[local_20 + 0x2e] + 8);
                  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                    *pfVar3 = *local_34;
                    local_34 = local_34 + 1;
                    pfVar3 = pfVar3 + 1;
                  }
                }
                else {
                  pfVar3 = local_34;
                  pfVar5 = local_78;
                  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                    *pfVar5 = *pfVar3;
                    pfVar3 = pfVar3 + 1;
                    pfVar5 = pfVar5 + 1;
                  }
                  if ((int)local_10 < local_24) {
                    local_8 = (float *)(*(int *)(&DAT_0065a794 + *param_1 * 900) +
                                       (local_1c + local_20 + local_14) * 0x40);
                  }
                  else {
                    local_8 = (float *)(*(int *)(&DAT_0065a794 + *param_1 * 900) +
                                       (param_1[0xb3] * local_14 + local_20) * 0x40);
                  }
                  local_7c = local_78;
                  if (local_38 == 1) {
                    for (local_80 = 0; local_80 < 0x10; local_80 = local_80 + 1) {
                      *local_7c = (*local_7c + *local_7c + *local_7c + *local_8) / _DAT_005a3834;
                      local_7c = local_7c + 1;
                      local_8 = local_8 + 1;
                    }
                  }
                  else if (local_38 == 2) {
                    for (local_84 = 0; local_84 < 0x10; local_84 = local_84 + 1) {
                      *local_7c = (*local_7c + *local_8) / _DAT_005a365c;
                      local_7c = local_7c + 1;
                      local_8 = local_8 + 1;
                    }
                  }
                  else if (local_38 == 3) {
                    for (local_88 = 0; local_88 < 0x10; local_88 = local_88 + 1) {
                      *local_7c = (*local_7c + *local_8 + *local_8 + *local_8) / _DAT_005a3834;
                      local_7c = local_7c + 1;
                      local_8 = local_8 + 1;
                    }
                  }
                  pfVar3 = local_78;
                  pfVar5 = (float *)(param_1[local_20 + 0x2e] + 8);
                  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                    *pfVar5 = *pfVar3;
                    pfVar3 = pfVar3 + 1;
                    pfVar5 = pfVar5 + 1;
                  }
                }
              }
              else {
                local_8 = (float *)(*(int *)(&DAT_0065a794 + *param_1 * 900) +
                                   (param_1[0xb6] + local_20) * 0x40);
                pfVar3 = local_34;
                pfVar5 = local_c8;
                for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                  *pfVar5 = *pfVar3;
                  pfVar3 = pfVar3 + 1;
                  pfVar5 = pfVar5 + 1;
                }
                if ((*param_1 == 1) || (*param_1 == 0)) {
                  local_10c = 10 - local_10;
                  FUN_004c5b40();
                  FUN_00430f10();
                  thunk_FUN_005630fc(local_11c,
                                     *(int *)(&DAT_0065a798 + *param_1 * 900) +
                                     (local_1c + local_20) * 0x10,
                                     *(int *)(&DAT_0065a798 + *param_1 * 900) +
                                     (param_1[0xb6] + local_20) * 0x10,
                                     (float)local_10c / _DAT_005a3670);
                  thunk_FUN_0056272c(local_108,local_11c);
                  local_124 = local_98;
                  local_8 = local_8 + 0xc;
                  local_128 = local_d8;
                  for (local_120 = 0; local_120 < 3; local_120 = local_120 + 1) {
                    *local_128 = ((float)local_10c * *local_8 + (float)(int)local_10 * *local_124) /
                                 _DAT_005a3670;
                    local_124 = local_124 + 1;
                    local_8 = local_8 + 1;
                    local_128 = local_128 + 1;
                  }
                  puVar2 = local_108;
                  puVar4 = (undefined4 *)(param_1[local_20 + 0x2e] + 8);
                  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                    *puVar4 = *puVar2;
                    puVar2 = puVar2 + 1;
                    puVar4 = puVar4 + 1;
                  }
                }
                else {
                  local_134 = local_c8;
                  iVar1 = 10 - local_10;
                  for (local_130 = 0; local_130 < 0x10; local_130 = local_130 + 1) {
                    *local_134 = ((float)iVar1 * *local_8 + (float)(int)local_10 * *local_134) /
                                 _DAT_005a3670;
                    local_134 = local_134 + 1;
                    local_8 = local_8 + 1;
                  }
                  pfVar3 = local_c8;
                  pfVar5 = (float *)(param_1[local_20 + 0x2e] + 8);
                  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
                    *pfVar5 = *pfVar3;
                    pfVar3 = pfVar3 + 1;
                    pfVar5 = pfVar5 + 1;
                  }
                }
              }
            }
          }
          param_1[1] = 1;
        }
      }
    }
  }
  return;
}

