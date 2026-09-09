// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00538691 | Name: FUN_00538691


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * __thiscall FUN_00538691(int param_1,float *param_2)

{
  float *pfVar1;
  undefined4 *puVar2;
  int iVar3;
  float local_1bc;
  float local_190;
  float local_128;
  float local_124;
  float local_120;
  int local_11c;
  int local_118;
  float local_114;
  float local_110;
  float local_10c;
  int local_108;
  int local_104;
  float local_100 [4];
  float local_f0;
  undefined4 local_ec;
  undefined4 local_e8;
  float local_e4;
  undefined4 local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  int local_d0;
  int local_cc;
  int local_c8;
  int local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  float local_9c;
  float local_98;
  float local_94;
  int local_90;
  int local_8c;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  undefined4 local_70;
  undefined1 local_6c [12];
  int local_60;
  int local_5c;
  int local_58;
  undefined4 local_54;
  undefined4 local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  undefined1 local_3c [8];
  float local_34;
  int local_30;
  int local_2c;
  undefined1 local_28 [12];
  float local_1c;
  int local_18;
  float local_14;
  undefined1 local_10 [12];
  
  if ((DAT_0092ecd4 & 1) == 0) {
    DAT_0092ecd4 = DAT_0092ecd4 | 1;
    FUN_00430f50(0,0xc61c4000,0);
    FUN_00591f89(FUN_005396d7);
  }
  FUN_00430f10();
  FUN_00430f10();
  (**(code **)(*DAT_005ccf9c + 0x50))(local_10,local_28);
  thunk_FUN_005611dd(local_28,local_28);
  local_14 = 0.0;
  local_1c = 0.0;
  local_70 = 0;
  local_58 = *(int *)(DAT_013b71e8 + 0x1b084);
  FUN_00430f10();
  if (*(int *)(local_58 + 0x54) == 0) {
    local_54 = *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x48);
    local_50 = *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x50);
  }
  else {
    FUN_0040bd30(*(int *)(local_58 + 0x54) + 0x28);
  }
  local_18 = __ftol();
  local_44 = __ftol();
  local_78 = 0x19;
  local_8c = 0;
  if (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) <= _DAT_005a3534) {
    local_190 = -*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c);
  }
  else {
    local_190 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c);
  }
  if (_DAT_005a3660 < local_190) {
    local_78 = __ftol();
  }
  local_8c = local_78 / 2;
  local_7c = local_18 - local_8c;
  local_84 = local_18 + local_78;
  local_88 = local_44 + local_78;
  local_80 = local_44 - local_8c;
  for (local_74 = local_80; local_74 < local_88; local_74 = local_74 + 1) {
    if ((-1 < local_74) && (local_74 < 0x80)) {
      for (local_90 = local_7c; local_90 < local_84; local_90 = local_90 + 1) {
        if ((-1 < local_90) && (local_90 < 0x80)) {
          local_c4 = (int)*(char *)(param_1 + 0xc0cc + local_90 + local_74 * 0x80);
          if (0x7f < local_c4) {
            local_c4 = 0;
          }
          if (local_c4 == 0x7f) {
            local_c4 = 400;
          }
          FUN_00421910(&local_c0,0xc,4,FUN_00430f10);
          pfVar1 = (float *)FUN_00430f50((float)local_90 + *(float *)(param_1 + 0x20a28),
                                         (float)local_c4 * _DAT_005a4214,
                                         (float)local_74 + *(float *)(param_1 + 0x20a2c));
          local_c0 = *pfVar1;
          local_bc = pfVar1[1];
          local_b8 = pfVar1[2];
          puVar2 = (undefined4 *)
                   FUN_00430f50((float)local_90 + *(float *)(param_1 + 0x20a28),
                                (float)local_c4 * _DAT_005a4214,
                                (float)local_74 + *(float *)(param_1 + 0x20a2c) + _DAT_005a3660);
          local_b4 = *puVar2;
          local_b0 = puVar2[1];
          local_ac = puVar2[2];
          puVar2 = (undefined4 *)
                   FUN_00430f50((float)local_90 + *(float *)(param_1 + 0x20a28) + _DAT_005a3660,
                                (float)local_c4 * _DAT_005a4214,
                                (float)local_74 + *(float *)(param_1 + 0x20a2c));
          local_a8 = *puVar2;
          local_a4 = puVar2[1];
          local_a0 = puVar2[2];
          pfVar1 = (float *)FUN_00430f50((float)local_90 + *(float *)(param_1 + 0x20a28) +
                                         _DAT_005a3660,(float)local_c4 * _DAT_005a4214,
                                         (float)local_74 + *(float *)(param_1 + 0x20a2c) +
                                         _DAT_005a3660);
          local_9c = *pfVar1;
          local_98 = pfVar1[1];
          local_94 = pfVar1[2];
          iVar3 = FUN_005649c7(&local_c0,&local_b4,&local_a8,local_10,local_28,&local_14,&local_1c,
                               &local_70);
          if (iVar3 == 1) {
            DAT_0092eccc = local_bc;
            DAT_0092ecc8 = local_c0 + local_1c;
            DAT_0092ecd0 = local_b8 + local_14;
            *param_2 = DAT_0092ecc8;
            param_2[1] = DAT_0092eccc;
            param_2[2] = DAT_0092ecd0;
            return param_2;
          }
          iVar3 = FUN_005649c7(&local_9c,&local_a8,&local_b4,local_10,local_28,&local_14,&local_1c,
                               &local_70);
          if (iVar3 == 1) {
            DAT_0092eccc = local_98;
            DAT_0092ecc8 = local_9c - local_1c;
            DAT_0092ecd0 = local_94 - local_14;
            *param_2 = DAT_0092ecc8;
            param_2[1] = DAT_0092eccc;
            param_2[2] = DAT_0092ecd0;
            return param_2;
          }
        }
      }
    }
  }
  if ((_DAT_005a4158 < *(float *)(local_58 + 0x2c)) || (*(int *)(DAT_0067cf38 + 0x200a0) == 1)) {
    *param_2 = DAT_0092ecc8;
    param_2[1] = DAT_0092eccc;
    param_2[2] = DAT_0092ecd0;
  }
  else {
    local_2c = 0;
    local_40 = 0;
    FUN_00430f10();
    FUN_00430f10();
    local_48 = 0;
    local_4c = 0;
    local_5c = 0x40;
    local_60 = 0x40;
    local_c8 = __ftol();
    local_cc = __ftol();
    local_48 = local_c8 + -0xc;
    local_4c = local_cc + -0xc;
    local_5c = local_c8 + 0xc;
    local_60 = local_cc + 0xc;
    if (local_48 < 0) {
      local_48 = 0;
    }
    if (local_4c < 0) {
      local_4c = 0;
    }
    if (0x40 < local_5c) {
      local_5c = 0x3f;
    }
    if (0x40 < local_60) {
      local_60 = 0x3f;
    }
    local_18 = local_18 / 2;
    local_44 = local_44 / 2;
    for (local_30 = local_4c; local_30 < local_60; local_30 = local_30 + 1) {
      if ((-1 < local_30) && (local_30 < 0x41)) {
        for (local_d0 = local_48; local_d0 < local_5c; local_d0 = local_d0 + 1) {
          if ((-1 < local_d0) && (local_d0 < 0x41)) {
            FUN_00421910(local_100,0xc,4,FUN_00430f10);
            if ((local_d0 < 0x40) && (local_30 < 0x40)) {
              pfVar1 = (float *)FUN_00430f50((float)local_d0 + (float)local_d0 +
                                             *(float *)(param_1 + 0x20a28),
                                             (float)(int)*(char *)(param_1 + 0xcc +
                                                                  (local_d0 + local_30 * 0x40) * 0xc
                                                                  ) * _DAT_005a4214,
                                             (float)local_30 + (float)local_30 +
                                             *(float *)(param_1 + 0x20a2c));
              local_100[0] = *pfVar1;
              local_100[1] = pfVar1[1];
              local_100[2] = pfVar1[2];
              puVar2 = (undefined4 *)
                       FUN_00430f50((float)local_d0 + (float)local_d0 +
                                    *(float *)(param_1 + 0x20a28),
                                    (float)(int)*(char *)(param_1 + 0xcc +
                                                         (local_d0 + (local_30 + 1) * 0x40) * 0xc) *
                                    _DAT_005a4214,
                                    (float)(local_30 + 1) + (float)(local_30 + 1) +
                                    *(float *)(param_1 + 0x20a2c));
              local_100[3] = (float)*puVar2;
              local_f0 = (float)puVar2[1];
              local_ec = puVar2[2];
              puVar2 = (undefined4 *)
                       FUN_00430f50((float)(local_d0 + 1) + (float)(local_d0 + 1) +
                                    *(float *)(param_1 + 0x20a28),
                                    (float)(int)*(char *)(param_1 + 0xcc +
                                                         (local_d0 + 1 + local_30 * 0x40) * 0xc) *
                                    _DAT_005a4214,
                                    (float)local_30 + (float)local_30 +
                                    *(float *)(param_1 + 0x20a2c));
              local_e8 = *puVar2;
              local_e4 = (float)puVar2[1];
              local_e0 = puVar2[2];
              pfVar1 = (float *)FUN_00430f50((float)(local_d0 + 1) + (float)(local_d0 + 1) +
                                             *(float *)(param_1 + 0x20a28),
                                             (float)(int)*(char *)(param_1 + 0xcc +
                                                                  (local_d0 + 1 +
                                                                  (local_30 + 1) * 0x40) * 0xc) *
                                             _DAT_005a4214,
                                             (float)(local_30 + 1) + (float)(local_30 + 1) +
                                             *(float *)(param_1 + 0x20a2c));
              local_dc = *pfVar1;
              local_d8 = pfVar1[1];
              local_d4 = pfVar1[2];
              if ((float)(int)*(char *)(param_1 + 0xc0cc + local_d0 + local_30 * 0x80) *
                  _DAT_005a4214 - (local_100[1] + local_f0 + local_e4 + local_d8) / _DAT_005a3834 <=
                  _DAT_005a3534) {
                local_1bc = -((float)(int)*(char *)(param_1 + 0xc0cc + local_d0 + local_30 * 0x80) *
                              _DAT_005a4214 -
                             (local_100[1] + local_f0 + local_e4 + local_d8) / _DAT_005a3834);
              }
              else {
                local_1bc = (float)(int)*(char *)(param_1 + 0xc0cc + local_d0 + local_30 * 0x80) *
                            _DAT_005a4214 -
                            (local_100[1] + local_f0 + local_e4 + local_d8) / _DAT_005a3834;
              }
              if (_DAT_005a3660 < local_1bc) goto LAB_00538d60;
            }
            iVar3 = FUN_005649c7(local_100,local_100 + 3,&local_e8,local_10,local_28,&local_14,
                                 &local_1c,&local_70);
            if (iVar3 == 1) {
              local_108 = 0;
              for (local_104 = 0; local_104 < 3; local_104 = local_104 + 1) {
                FUN_00430f10();
                local_114 = local_100[local_104 * 3];
                local_110 = local_100[local_104 * 3 + 1];
                local_10c = local_100[local_104 * 3 + 2];
                thunk_FUN_00561462(local_6c,&local_114,DAT_005ccf9c + 0xa98b);
                thunk_FUN_00561462(local_3c,local_6c,DAT_005ccf9c + 0xa97b);
                if ((_DAT_005a3534 <= local_34) && (local_34 < _DAT_005a3660)) {
                  local_2c = __ftol();
                  local_40 = __ftol();
                  if ((_DAT_005a44b0 * DAT_005b490c < (float)local_2c) &&
                     ((((float)local_2c <
                        _DAT_005a40f8 * DAT_005b490c +
                        (float)(DAT_005ccf9c[0xa941] - DAT_005ccf9c[0xa979]) &&
                       (_DAT_005a44b0 * DAT_005b4910 < (float)local_40)) &&
                      ((float)local_40 <
                       _DAT_005a40f8 * DAT_005b4910 +
                       (float)(DAT_005ccf9c[0xa942] - DAT_005ccf9c[0xa97a]))))) {
                    local_108 = 1;
                    break;
                  }
                }
              }
              if (local_108 != 0) {
                DAT_0092eccc = local_100[1];
                DAT_0092ecc8 = local_1c + local_1c + local_100[0];
                DAT_0092ecd0 = local_14 + local_14 + local_100[2];
                *param_2 = DAT_0092ecc8;
                param_2[1] = DAT_0092eccc;
                param_2[2] = DAT_0092ecd0;
                return param_2;
              }
            }
            iVar3 = FUN_005649c7(&local_dc,&local_e8,local_100 + 3,local_10,local_28,&local_14,
                                 &local_1c,&local_70);
            if (iVar3 == 1) {
              local_11c = 0;
              for (local_118 = 1; local_118 < 4; local_118 = local_118 + 1) {
                FUN_00430f10();
                local_128 = local_100[local_118 * 3];
                local_124 = local_100[local_118 * 3 + 1];
                local_120 = local_100[local_118 * 3 + 2];
                thunk_FUN_00561462(local_6c,&local_128,DAT_005ccf9c + 0xa98b);
                thunk_FUN_00561462(local_3c,local_6c,DAT_005ccf9c + 0xa97b);
                if ((_DAT_005a3534 <= local_34) && (local_34 < _DAT_005a3660)) {
                  local_2c = __ftol();
                  local_40 = __ftol();
                  if (((_DAT_005a44b0 * DAT_005b490c < (float)local_2c) &&
                      (((float)local_2c <
                        _DAT_005a40f8 * DAT_005b490c +
                        (float)(DAT_005ccf9c[0xa941] - DAT_005ccf9c[0xa979]) &&
                       (_DAT_005a44b0 * DAT_005b4910 < (float)local_40)))) &&
                     ((float)local_40 <
                      _DAT_005a40f8 * DAT_005b4910 +
                      (float)(DAT_005ccf9c[0xa942] - DAT_005ccf9c[0xa97a]))) {
                    local_11c = 1;
                    break;
                  }
                }
              }
              if (local_11c != 0) {
                DAT_0092eccc = local_d8;
                DAT_0092ecc8 = local_dc - (local_1c + local_1c);
                DAT_0092ecd0 = local_d4 - (local_14 + local_14);
                *param_2 = DAT_0092ecc8;
                param_2[1] = DAT_0092eccc;
                param_2[2] = DAT_0092ecd0;
                return param_2;
              }
            }
          }
LAB_00538d60:
        }
      }
    }
    FUN_00430f50(0,0xc61c4000,0);
  }
  return param_2;
}

