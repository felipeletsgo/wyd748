// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00535298 | Name: FUN_00535298


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00535298(int param_1)

{
  float fVar1;
  byte bVar2;
  bool bVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  void *pvVar7;
  uint uVar8;
  float10 fVar9;
  int local_41c;
  int *local_418;
  int *local_414;
  int *local_408;
  uint local_36c;
  int local_368;
  uint local_364;
  int local_360;
  uint local_35c;
  int local_358;
  uint local_354;
  int local_350;
  float local_348;
  float local_1e8;
  float local_1e4;
  float local_1e0;
  int local_1dc;
  float local_1d8;
  float local_1d4;
  float local_1d0 [8];
  int local_1b0;
  int local_1ac;
  int *local_1a8;
  int local_1a4;
  int *local_1a0;
  float local_19c;
  float local_198;
  float local_194;
  int local_190;
  int local_18c;
  uint local_188;
  int local_184;
  int local_180;
  int local_17c;
  float local_178;
  float local_174;
  float local_170;
  float local_16c;
  float local_164;
  float local_160;
  float local_15c;
  float local_154;
  undefined1 local_150 [16];
  float local_140;
  float local_13c;
  int local_138;
  int local_134;
  uint local_130 [4];
  uint local_120;
  uint local_11c;
  int local_118;
  uint local_114;
  uint local_110;
  int local_10c;
  int local_108;
  int local_104;
  float local_100;
  uint local_fc;
  int local_f8;
  int local_f4;
  float local_f0;
  undefined1 local_ec [64];
  undefined1 local_ac [12];
  int local_a0;
  int local_9c;
  int local_98;
  int local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  int local_80;
  undefined1 local_7c [64];
  uint local_3c;
  uint local_38;
  undefined1 local_34 [8];
  float local_2c;
  int local_28;
  uint local_24;
  int local_20;
  uint local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a25db;
  local_10 = ExceptionList;
  if (DAT_005d03f0 == 1) {
    uVar4 = 0;
  }
  else if (*(int *)(param_1 + 0xbc) == 0) {
    uVar4 = 1;
  }
  else {
    local_90 = *(int *)(DAT_013b71e8 + 0x1b084);
    if (_DAT_005a4154 < *(float *)(local_90 + 0x2c)) {
      uVar4 = 1;
    }
    else if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
      uVar4 = 1;
    }
    else {
      ExceptionList = &local_10;
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      thunk_FUN_0056230d(local_ec,0x40000000,0x3dcccccd,0x40000000);
      thunk_FUN_0056239d(local_7c,*(undefined4 *)(param_1 + 0x20a28),0,
                         *(undefined4 *)(param_1 + 0x20a2c));
      thunk_FUN_00561a5b(local_ec,DAT_005ccf9c + 0xa99b,local_ec);
      thunk_FUN_00561a5b(local_ec,local_ec,local_7c);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_ec);
      if (*(int *)(param_1 + 0xc0) == 1) {
        (**(code **)(*DAT_005ccf9c + 0x30))(8,2);
      }
      local_8c = *(undefined4 *)(local_90 + 0x48);
      local_88 = *(undefined4 *)(local_90 + 0x4c);
      local_84 = *(undefined4 *)(local_90 + 0x50);
      local_14 = __ftol();
      iVar5 = __ftol();
      local_14 = (int)local_14 / 2;
      local_24 = iVar5 / 2;
      (**(code **)(*DAT_005ccf9c + 0x30))(0x30,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x8c,3);
      if (DAT_005ccf9c[0xa8e4] == 1) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x8d,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x8d,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
        if (((*(int *)(param_1 + 0xc4) == 0) || (*(int *)(param_1 + 0xc4) == 3)) ||
           (*(int *)(param_1 + 0xc4) == 4)) {
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        }
        (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
      }
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],param_1 + 0x200cc);
      local_20 = 0xf;
      if (*(int *)(DAT_0067cf38 + 0x24) == 0x7535) {
        local_20 = 0x12;
      }
      local_a0 = 0;
      if (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) <= _DAT_005a3534) {
        local_348 = -*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c);
      }
      else {
        local_348 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c);
      }
      if (_DAT_005a3660 < local_348) {
        local_20 = __ftol();
      }
      local_a0 = local_20 / 3;
      local_38 = local_14 - local_a0;
      local_3c = local_24 - local_a0;
      local_98 = local_14 + local_20;
      local_9c = local_24 + local_20;
      if (((-1 < DAT_005ccfa0) || (*(int *)(DAT_0067cf38 + 0x24) == 0x7534)) ||
         (*(int *)(DAT_0067cf38 + 0x24) == 0x7535)) {
        local_104 = __ftol();
        if ((((DAT_005ccfa0 == 2) || (DAT_005ccfa0 == 3)) ||
            ((DAT_005ccfa0 == 4 || ((DAT_005ccfa0 == 0 || (DAT_005ccfa0 == 4)))))) &&
           (_DAT_005a58b8 < *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c))) {
          local_104 = 0xd;
        }
        if (local_104 < 6) {
          local_104 = 6;
        }
        local_104 = local_104 + 2;
        iVar5 = __ftol();
        if ((((iVar5 >> 7 < 0x1b) || (iVar5 = __ftol(), 0x1e < iVar5 >> 7)) ||
            (iVar5 = __ftol(), iVar5 >> 7 < 0x15)) || (iVar5 = __ftol(), 0x18 < iVar5 >> 7)) {
          bVar3 = false;
        }
        else {
          bVar3 = true;
        }
        if (bVar3) {
          if (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) <= _DAT_005a5360) {
            if (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) <= _DAT_005a58b4) {
              if (_DAT_005a58b0 < *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c)) {
                local_104 = local_104 + 3;
              }
            }
            else {
              local_104 = local_104 + 4;
            }
          }
          else {
            local_104 = local_104 + 10;
          }
        }
        else if (_DAT_005a5360 < *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c)) {
          local_104 = local_104 + 3;
        }
        fVar9 = (float10)FUN_00493bf0(*(undefined4 *)(local_90 + 0x28));
        local_100 = (float)fVar9;
        fVar9 = (float10)FUN_00423b00(*(undefined4 *)(local_90 + 0x28));
        local_f0 = (float)fVar9;
        local_110 = local_14;
        local_fc = local_24;
        local_10c = __ftol();
        local_10c = local_110 + local_10c;
        iVar5 = __ftol();
        local_f8 = local_fc + iVar5;
        local_108 = (int)(local_110 + local_10c) / 2;
        local_f4 = (int)(local_fc + local_fc + iVar5) / 2;
        local_38 = local_108 - local_104;
        local_3c = local_f4 - local_104;
        local_98 = local_108 + local_104;
        local_9c = local_f4 + local_104;
      }
      local_80 = 1;
      local_94 = 1;
      for (local_1c = local_3c; (int)local_1c < local_9c; local_1c = local_1c + 1) {
        if ((-1 < (int)local_1c) && ((int)local_1c < 0x40)) {
          for (local_114 = local_38; (int)local_114 < local_98; local_114 = local_114 + 1) {
            if ((-1 < (int)local_114) && ((int)local_114 < 0x40)) {
              local_80 = 1;
              local_94 = 1;
              uVar8 = local_14 & 0x80000001;
              if ((int)uVar8 < 0) {
                uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
              }
              if (uVar8 == 0) {
                if (local_114 == local_14 || (int)(local_114 - local_14) < 0) {
                  local_350 = -(local_114 - local_14);
                }
                else {
                  local_350 = local_114 - local_14;
                }
                if (0x10 < local_350) {
                  if (local_114 == local_14 || (int)(local_114 - local_14) < 0) {
                    local_354 = -(local_114 - local_14);
                  }
                  else {
                    local_354 = local_114 - local_14;
                  }
                  local_354 = local_354 & 0x80000001;
                  if ((int)local_354 < 0) {
                    local_354 = (local_354 - 1 | 0xfffffffe) + 1;
                  }
                  if (local_354 == 0) goto LAB_005359e9;
                  local_80 = 2;
                }
              }
              else {
                if (local_114 == local_14 || (int)(local_114 - local_14) < 0) {
                  local_358 = -(local_114 - local_14);
                }
                else {
                  local_358 = local_114 - local_14;
                }
                if (0x11 < local_358) {
                  if (local_114 == local_14 || (int)(local_114 - local_14) < 0) {
                    local_35c = -(local_114 - local_14);
                  }
                  else {
                    local_35c = local_114 - local_14;
                  }
                  local_35c = local_35c & 0x80000001;
                  if ((int)local_35c < 0) {
                    local_35c = (local_35c - 1 | 0xfffffffe) + 1;
                  }
                  if (local_35c == 1) goto LAB_005359e9;
                  local_80 = 2;
                }
              }
              uVar8 = local_24 & 0x80000001;
              if ((int)uVar8 < 0) {
                uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
              }
              if (uVar8 == 0) {
                if (local_1c == local_24 || (int)(local_1c - local_24) < 0) {
                  local_360 = -(local_1c - local_24);
                }
                else {
                  local_360 = local_1c - local_24;
                }
                if (0x10 < local_360) {
                  if (local_1c == local_24 || (int)(local_1c - local_24) < 0) {
                    local_364 = -(local_1c - local_24);
                  }
                  else {
                    local_364 = local_1c - local_24;
                  }
                  local_364 = local_364 & 0x80000001;
                  if ((int)local_364 < 0) {
                    local_364 = (local_364 - 1 | 0xfffffffe) + 1;
                  }
                  if (local_364 == 0) goto LAB_005359e9;
                  local_94 = 2;
                }
              }
              else {
                if (local_1c == local_24 || (int)(local_1c - local_24) < 0) {
                  local_368 = -(local_1c - local_24);
                }
                else {
                  local_368 = local_1c - local_24;
                }
                if (0x11 < local_368) {
                  if (local_1c == local_24 || (int)(local_1c - local_24) < 0) {
                    local_36c = -(local_1c - local_24);
                  }
                  else {
                    local_36c = local_1c - local_24;
                  }
                  local_36c = local_36c & 0x80000001;
                  if ((int)local_36c < 0) {
                    local_36c = (local_36c - 1 | 0xfffffffe) + 1;
                  }
                  if (local_36c == 1) goto LAB_005359e9;
                  local_94 = 2;
                }
              }
              bVar2 = *(byte *)(param_1 + 0xce + (local_114 + local_1c * 0x40) * 0xc);
              local_120 = CONCAT31(local_120._1_3_,bVar2);
              local_11c = CONCAT31(local_11c._1_3_,
                                   *(undefined1 *)
                                    (param_1 + 0xd0 + (local_114 + local_1c * 0x40) * 0xc));
              local_118 = *(byte *)(param_1 + 0xcd + (local_114 + local_1c * 0x40) * 0xc) + 10;
              if (DAT_005ccf9c[0xa8e4] == 1) {
                for (local_138 = 0; local_138 < 4; local_138 = local_138 + 1) {
                  *(undefined4 *)(param_1 + 0x201d0 + local_138 * 0x18) =
                       *(undefined4 *)(&DAT_005bb918 + local_138 * 8 + (uint)bVar2 * 0x20);
                  *(undefined4 *)(param_1 + 0x201d4 + local_138 * 0x18) =
                       *(undefined4 *)(&DAT_005bb91c + local_138 * 8 + (uint)bVar2 * 0x20);
                }
                local_130[0] = *(uint *)(param_1 + 0xd4 + (local_114 + local_1c * 0x40) * 0xc);
                local_130[1] = *(undefined4 *)
                                (param_1 + 0xd4 + (local_114 + (local_1c + local_94) * 0x40) * 0xc);
                local_130[2] = *(undefined4 *)
                                (param_1 + 0xd4 + (local_114 + local_80 + local_1c * 0x40) * 0xc);
                local_130[3] = *(undefined4 *)
                                (param_1 + 0xd4 +
                                (local_114 + local_80 + (local_1c + local_94) * 0x40) * 0xc);
                for (local_134 = 0; local_134 < 4; local_134 = local_134 + 1) {
                  local_140 = (float)((local_130[local_134] & 0xff0000) >> 0x10) / _DAT_005a41e4;
                  local_154 = (float)((local_130[local_134] & 0xff00) >> 8) / _DAT_005a41e4;
                  local_13c = (float)(local_130[local_134] & 0xff) / _DAT_005a41e4;
                  FUN_00430f10();
                  local_174 = local_140;
                  local_170 = local_154;
                  local_16c = local_13c;
                  FUN_00430f10();
                  local_164 = (float)DAT_005ccf9c[0xa9b9] * _DAT_005a4154;
                  local_160 = (float)DAT_005ccf9c[0xa9ba] * _DAT_005a4154;
                  local_15c = (float)DAT_005ccf9c[0xa9bb] * _DAT_005a4154;
                  FUN_00430f10();
                  FUN_0053a9b0(local_150,&local_174,&local_164,0x3f333333);
                  iVar5 = __ftol();
                  iVar6 = __ftol();
                  uVar8 = __ftol();
                  *(uint *)(param_1 + 0x201cc + local_134 * 0x18) =
                       iVar5 << 0x10 | iVar6 << 8 | uVar8;
                }
                if (((int)local_114 < 0x3f) && ((int)local_1c < 0x3f)) {
                  *(undefined4 *)(param_1 + 0x20128) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + local_1c * 0x40) * 0xc);
                  *(undefined4 *)(param_1 + 0x20154) =
                       *(undefined4 *)
                        (param_1 + 0xd4 + (local_114 + (local_1c + local_94) * 0x40) * 0xc);
                  *(undefined4 *)(param_1 + 0x20180) =
                       *(undefined4 *)
                        (param_1 + 0xd4 + (local_114 + local_80 + local_1c * 0x40) * 0xc);
                  *(undefined4 *)(param_1 + 0x201ac) =
                       *(undefined4 *)
                        (param_1 + 0xd4 +
                        (local_114 + local_80 + (local_1c + local_94) * 0x40) * 0xc);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 + local_1c * 0x40)
                                                            * 0xc),(float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                }
                if ((local_114 == 0x3f) && ((int)local_1c < 0x3f)) {
                  *(undefined4 *)(param_1 + 0x20128) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_1c * 0x40 + 0x3f) * 0xc);
                  *(undefined4 *)(param_1 + 0x20154) =
                       *(undefined4 *)(param_1 + 0xd4 + ((local_1c + local_94) * 0x40 + 0x3f) * 0xc)
                  ;
                  *(undefined4 *)(param_1 + 0x20180) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_1c * 0x40 + 0x3f) * 0xc);
                  *(undefined4 *)(param_1 + 0x201ac) =
                       *(undefined4 *)(param_1 + 0xd4 + ((local_1c + local_94) * 0x40 + 0x3f) * 0xc)
                  ;
                  uVar4 = FUN_00430f50(0x427c0000,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_1c * 0x40 + 0x3f) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                }
                if ((local_1c == 0x3f) && ((int)local_114 < 0x3f)) {
                  *(undefined4 *)(param_1 + 0x20128) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + 0xfc0) * 0xc);
                  *(undefined4 *)(param_1 + 0x20154) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + 0xfc0) * 0xc);
                  *(undefined4 *)(param_1 + 0x20180) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + local_80 + 0xfc0) * 0xc);
                  *(undefined4 *)(param_1 + 0x201ac) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + local_80 + 0xfc0) * 0xc);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + 0xfc0) * 0xc),0x427c0000);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 + local_1c * 0x40)
                                                            * 0xc),(float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 + local_1c * 0x40)
                                                            * 0xc),(float)(int)(local_1c + local_94)
                                      );
                  FUN_004310a0(uVar4);
                }
                if ((*(int *)(param_1 + 0x20a6c) != 0) &&
                   (*(uint *)(param_1 + 0xa8) < *(int *)(param_1 + 0x20a6c) + 2000U)) {
                  local_178 = (_DAT_005a3834 * *(float *)(param_1 + 0x20a60) *
                              (float)((*(int *)(param_1 + 0xa8) - *(int *)(param_1 + 0x20a6c)) +
                                     -2000)) / _DAT_005a441c;
                  for (local_17c = 0; local_17c < 4; local_17c = local_17c + 1) {
                    iVar5 = local_17c * 0x18;
                    fVar9 = (float10)FUN_00493bf0((float)*(uint *)(param_1 + 0xa8) / _DAT_005a43c4 +
                                                  ((*(float *)(param_1 + 0x20a68) -
                                                   *(float *)(param_1 + 0x201c8 + local_17c * 0x18))
                                                  * (*(float *)(param_1 + 0x20a64) -
                                                    *(float *)(param_1 + 0x201c0 + local_17c * 0x18)
                                                    )) / _DAT_005a3670);
                    *(float *)(param_1 + 0x201c4 + local_17c * 0x18) =
                         (float)(fVar9 * (float10)local_178 +
                                (float10)*(float *)(param_1 + 0x201c4 + iVar5));
                  }
                }
                uVar4 = FUN_004ba2cf(local_118,60000);
                (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar4);
                (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
                (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                          (DAT_005ccf9c[0xa8f9],5,2,param_1 + 0x201c0,0x18);
              }
              else {
                local_188 = *(byte *)(param_1 + 0xcd + (local_114 + local_1c * 0x40) * 0xc) + 10;
                for (local_184 = 0; local_184 < 4; local_184 = local_184 + 1) {
                  *(undefined4 *)(param_1 + 0x2012c + local_184 * 0x2c) =
                       *(undefined4 *)(&DAT_005bb918 + local_184 * 8 + (local_120 & 0xff) * 0x20);
                  *(undefined4 *)(param_1 + 0x20130 + local_184 * 0x2c) =
                       *(undefined4 *)(&DAT_005bb91c + local_184 * 8 + (local_120 & 0xff) * 0x20);
                  if (((*(int *)(param_1 + 0xc4) == 0) || (*(int *)(param_1 + 0xc4) == 3)) ||
                     (*(int *)(param_1 + 0xc4) == 4)) {
                    *(undefined4 *)(param_1 + 0x20134 + local_184 * 0x2c) =
                         *(undefined4 *)(&DAT_005bba18 + local_184 * 8 + (local_11c & 0xff) * 0x20);
                    *(undefined4 *)(param_1 + 0x20138 + local_184 * 0x2c) =
                         *(undefined4 *)(&DAT_005bba1c + local_184 * 8 + (local_11c & 0xff) * 0x20);
                  }
                  if (((*(int *)(param_1 + 0xc4) != 0) && (*(int *)(param_1 + 0xc4) != 3)) &&
                     (*(int *)(param_1 + 0xc4) != 4)) {
                    if (((*(int *)(param_1 + 0x20a20) < 0x1a) ||
                        (0x1e < *(int *)(param_1 + 0x20a20))) ||
                       ((*(int *)(param_1 + 0x20a24) < 8 || (0xc < *(int *)(param_1 + 0x20a24))))) {
                      bVar3 = false;
                    }
                    else {
                      bVar3 = true;
                    }
                    if (!bVar3) {
                      if ((local_188 == 0xaa) || (local_188 == 0xab)) {
                        *(float *)(param_1 + 0x2012c + local_184 * 0x2c) =
                             (float)(*(uint *)(param_1 + 0xa8) % 10000) / _DAT_005a420c +
                             *(float *)(param_1 + 0x2012c + local_184 * 0x2c);
                        if (DAT_0092e660 == 0) {
                          local_190 = _rand();
                          FUN_00430f50((float)(int)local_114 + (float)(int)local_114 +
                                       *(float *)(param_1 + 0x20a28) + _DAT_005a34a0,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc) *
                                       _DAT_005a4214 + _DAT_005a430c,
                                       (float)(int)local_1c + (float)(int)local_1c +
                                       *(float *)(param_1 + 0x20a2c) + _DAT_005a34a0);
                          if (local_190 % 200 < 2) {
                            local_1a4 = _rand();
                            local_1a4 = local_1a4 % 10;
                            pvVar7 = operator_new(0x180);
                            local_8 = 0;
                            if (pvVar7 == (void *)0x0) {
                              local_408 = (int *)0x0;
                            }
                            else {
                              local_408 = (int *)FUN_004c8f70(0,1000,(float)local_1a4 *
                                                                     _DAT_005a58ac + _DAT_005a41e8,
                                                              (float)local_1a4 * _DAT_005a414c +
                                                              _DAT_005a41e8,
                                                              (float)local_1a4 * _DAT_005a58ac +
                                                              _DAT_005a41e8,0x38d1b717,1,0x50);
                            }
                            local_8 = 0xffffffff;
                            local_1a0 = local_408;
                            if (local_408 != (int *)0x0) {
                              iVar5 = _rand();
                              local_19c = (float)(iVar5 % 10 + -5) * _DAT_005a41e8 + local_19c;
                              iVar5 = _rand();
                              local_194 = (float)(iVar5 % 10 + -5) * _DAT_005a41e8 + local_194;
                              uVar4 = FUN_004310a0(&local_19c);
                              FUN_004310a0(uVar4);
                              local_1a0[0x1b] = 1;
                              local_1a0[0x40] = 0;
                              local_1a0[0x52] = 1;
                              local_1a0[0x50] = 0x3f333333;
                              (**(code **)(*local_1a0 + 0x3c))(0xffffaa00);
                              FUN_0054ac09(local_1a0);
                            }
                          }
                          if (2000 < (uint)(*(int *)(param_1 + 0xa8) - *(int *)(param_1 + 0xa4))) {
                            uVar8 = local_114 & 0x80000001;
                            if ((int)uVar8 < 0) {
                              uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
                            }
                            if (((uVar8 == 0) && ((int)local_1c % 3 == 0)) && (local_190 % 100 < 1))
                            {
                              local_1ac = _rand();
                              local_1ac = local_1ac % 7;
                              pvVar7 = operator_new(0x180);
                              local_8 = 1;
                              if (pvVar7 == (void *)0x0) {
                                local_414 = (int *)0x0;
                              }
                              else {
                                local_414 = (int *)FUN_004c8f70(0x38,20000,0x3e4ccccd,0x3e4ccccd,
                                                                0x3e4ccccd,0,1,0x50);
                              }
                              local_8 = 0xffffffff;
                              local_1a8 = local_414;
                              if (local_414 != (int *)0x0) {
                                uVar4 = FUN_00430f50(local_19c,
                                                     (float)local_1ac * _DAT_005a4158 +
                                                     local_198 + _DAT_005a430c,local_194);
                                FUN_004310a0(uVar4);
                                FUN_004310a0(local_1a8 + 0x1d);
                                local_1a8[0x51] =
                                     (int)((float)local_1ac * _DAT_005a4214 + _DAT_005a430c);
                                local_1a8[0x4f] =
                                     (int)((float)local_1ac * _DAT_005a40ec + _DAT_005a40f0);
                                local_1a8[0x50] =
                                     (int)((float)local_1ac * _DAT_005a439c + _DAT_005a4158);
                                local_1a8[0x52] = local_1ac % 3 + 6;
                                local_1a8[0x1b] = 1;
                                (**(code **)(*local_1a8 + 0x3c))(0xffffaa00);
                                FUN_0054ac09(local_1a8);
                              }
                              pvVar7 = operator_new(0x180);
                              local_8 = 2;
                              if (pvVar7 == (void *)0x0) {
                                local_418 = (int *)0x0;
                              }
                              else {
                                local_418 = (int *)FUN_004c8f70(0x3c,20000,0x3d8f5c29,0x3d8f5c29,
                                                                0x3d8f5c29,0,1,0x50);
                              }
                              local_8 = 0xffffffff;
                              local_1a8 = local_418;
                              if (local_418 != (int *)0x0) {
                                uVar4 = FUN_00430f50(local_19c,
                                                     (float)local_1ac * _DAT_005a4158 +
                                                     local_198 + _DAT_005a430c,local_194);
                                FUN_004310a0(uVar4);
                                FUN_004310a0(local_1a8 + 0x1d);
                                local_1a8[0x51] =
                                     (int)((float)local_1ac * _DAT_005a4214 + _DAT_005a430c);
                                local_1a8[0x4f] =
                                     (int)((float)local_1ac * _DAT_005a40ec + _DAT_005a40f0);
                                local_1a8[0x50] =
                                     (int)((float)local_1ac * _DAT_005a439c + _DAT_005a4158);
                                local_1a8[0x52] = local_1ac % 3 + 6;
                                local_1a8[0x1b] = 1;
                                (**(code **)(*local_1a8 + 0x3c))(0xffffff00);
                                FUN_0054ac09(local_1a8);
                              }
                              if (local_1ac < 3) {
                                pvVar7 = operator_new(0xa0);
                                local_8 = 3;
                                if (pvVar7 == (void *)0x0) {
                                  local_41c = 0;
                                }
                                else {
                                  local_41c = FUN_004e2a30(local_19c,local_198,local_194,1,0,
                                                           0xffffffff,0x22331100);
                                }
                                local_8 = 0xffffffff;
                                local_1b0 = local_41c;
                                if (local_41c != 0) {
                                  FUN_0054ac09(local_41c);
                                }
                              }
                            }
                          }
                        }
                      }
                      else if ((local_188 == 0x26) || (local_188 == 0x27)) {
                        local_1d0[4] = 0.0;
                        local_1d0[5] = 0.0;
                        local_1d0[6] = 1.0;
                        local_1d0[7] = 1.0;
                        local_1d0[0] = 0.0;
                        local_1d0[1] = 1.0;
                        local_1d0[2] = 0.0;
                        local_1d0[3] = 1.0;
                        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,5);
                        uVar4 = FUN_004ba2cf(0x158,60000);
                        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar4);
                        local_1d4 = (float)(*(uint *)(param_1 + 0xa8) % 10000) / _DAT_005a420c;
                        *(float *)(param_1 + 0x20134 + local_184 * 0x2c) = local_1d0[local_184 + 4];
                        *(float *)(param_1 + 0x20138 + local_184 * 0x2c) =
                             local_1d4 + local_1d0[local_184];
                        local_118 = local_188 + 0x5c;
                      }
                      else if (((int)local_188 < 0x3e) || (0x41 < (int)local_188)) {
                        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
                      }
                      else {
                        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,5);
                        uVar4 = FUN_004ba2cf(local_188 + 0x11e,60000);
                        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar4);
                        *(undefined4 *)(param_1 + 0x20134 + local_184 * 0x2c) =
                             *(undefined4 *)(param_1 + 0x2012c + local_184 * 0x2c);
                        *(undefined4 *)(param_1 + 0x20138 + local_184 * 0x2c) =
                             *(undefined4 *)(param_1 + 0x20130 + local_184 * 0x2c);
                        uVar8 = local_188 & 0x80000001;
                        if ((int)uVar8 < 0) {
                          uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
                        }
                        local_118 = uVar8 + 0x82;
                      }
                    }
                  }
                }
                if (((int)local_114 < 0x3f) && ((int)local_1c < 0x3f)) {
                  *(undefined4 *)(param_1 + 0x20128) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + local_1c * 0x40) * 0xc);
                  *(undefined4 *)(param_1 + 0x20154) =
                       *(undefined4 *)
                        (param_1 + 0xd4 + (local_114 + (local_1c + local_94) * 0x40) * 0xc);
                  *(undefined4 *)(param_1 + 0x20180) =
                       *(undefined4 *)
                        (param_1 + 0xd4 + (local_114 + local_80 + local_1c * 0x40) * 0xc);
                  *(undefined4 *)(param_1 + 0x201ac) =
                       *(undefined4 *)
                        (param_1 + 0xd4 +
                        (local_114 + local_80 + (local_1c + local_94) * 0x40) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + local_1c * 0x40) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + (local_1c + local_94) * 0x40) * 0xc)
                  ;
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + local_80 + local_1c * 0x40) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc +
                               (local_114 + local_80 + (local_1c + local_94) * 0x40) * 0xc);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 + local_1c * 0x40)
                                                            * 0xc),(float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                }
                if ((local_114 == 0x3f) && ((int)local_1c < 0x3f)) {
                  *(undefined4 *)(param_1 + 0x20128) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_1c * 0x40 + 0x3f) * 0xc);
                  *(undefined4 *)(param_1 + 0x20154) =
                       *(undefined4 *)(param_1 + 0xd4 + ((local_1c + local_94) * 0x40 + 0x3f) * 0xc)
                  ;
                  *(undefined4 *)(param_1 + 0x20180) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_1c * 0x40 + 0x3f) * 0xc);
                  *(undefined4 *)(param_1 + 0x201ac) =
                       *(undefined4 *)(param_1 + 0xd4 + ((local_1c + local_94) * 0x40 + 0x3f) * 0xc)
                  ;
                  FUN_004310a0(param_1 + 0x140cc + (local_1c * 0x40 + 0x3f) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + (local_1c + local_94) * 0x40) * 0xc)
                  ;
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + local_1c * 0x40) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + (local_1c + local_94) * 0x40) * 0xc)
                  ;
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 +
                                                            (local_1c + local_94) * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                }
                if ((local_1c == 0x3f) && ((int)local_114 < 0x3f)) {
                  *(undefined4 *)(param_1 + 0x20128) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + 0xfc0) * 0xc);
                  *(undefined4 *)(param_1 + 0x20154) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + 0xfc0) * 0xc);
                  *(undefined4 *)(param_1 + 0x20180) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + local_80 + 0xfc0) * 0xc);
                  *(undefined4 *)(param_1 + 0x201ac) =
                       *(undefined4 *)(param_1 + 0xd4 + (local_114 + local_80 + 0xfc0) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + 0xfc0) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + local_1c * 0x40) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + local_80 + local_1c * 0x40) * 0xc);
                  FUN_004310a0(param_1 + 0x140cc + (local_114 + local_80 + local_1c * 0x40) * 0xc);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)local_114,
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_1c * 0x40) * 0xc),
                                       (float)(int)(local_1c + local_94));
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 + local_1c * 0x40)
                                                            * 0xc),(float)(int)local_1c);
                  FUN_004310a0(uVar4);
                  uVar4 = FUN_00430f50((float)(int)(local_114 + local_80),
                                       (float)(int)*(char *)(param_1 + 0xcc +
                                                            (local_114 + local_80 + local_1c * 0x40)
                                                            * 0xc),(float)(int)(local_1c + local_94)
                                      );
                  FUN_004310a0(uVar4);
                }
                if ((*(int *)(param_1 + 0x20a6c) != 0) &&
                   (*(uint *)(param_1 + 0xa8) < *(int *)(param_1 + 0x20a6c) + 2000U)) {
                  local_1d8 = (_DAT_005a3834 * *(float *)(param_1 + 0x20a60) *
                              (float)((*(int *)(param_1 + 0xa8) - *(int *)(param_1 + 0x20a6c)) +
                                     -2000)) / _DAT_005a441c;
                  for (local_1dc = 0; local_1dc < 4; local_1dc = local_1dc + 1) {
                    iVar5 = local_1dc * 0x2c;
                    fVar9 = (float10)FUN_00493bf0((float)*(uint *)(param_1 + 0xa8) / _DAT_005a43c4 +
                                                  ((*(float *)(param_1 + 0x20a68) -
                                                   *(float *)(param_1 + 0x20118 + local_1dc * 0x2c))
                                                  * (*(float *)(param_1 + 0x20a64) -
                                                    *(float *)(param_1 + 0x20110 + local_1dc * 0x2c)
                                                    )) / _DAT_005a3670);
                    *(float *)(param_1 + 0x20114 + local_1dc * 0x2c) =
                         (float)(fVar9 * (float10)local_1d8 +
                                (float10)*(float *)(param_1 + 0x20114 + iVar5));
                  }
                }
                uVar4 = FUN_004ba2cf(local_118,60000);
                (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar4);
                if (((*(int *)(param_1 + 0xc4) == 0) || (*(int *)(param_1 + 0xc4) == 3)) ||
                   (*(int *)(param_1 + 0xc4) == 4)) {
                  uVar4 = FUN_004ba2cf(*(byte *)(param_1 + 0xcf +
                                                (local_114 + local_1c * 0x40) * 0xc) + 0x100,60000);
                  (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar4);
                }
                (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x252);
                local_18c = 0;
                for (local_180 = 0; local_180 < 4; local_180 = local_180 + 1) {
                  FUN_00430f10();
                  fVar1 = *(float *)(param_1 + 0x20110 + local_180 * 0x2c);
                  local_1e8 = fVar1 + fVar1 + *(float *)(param_1 + 0x20a28);
                  local_1e4 = *(float *)(param_1 + 0x20114 + local_180 * 0x2c) * _DAT_005a4214;
                  fVar1 = *(float *)(param_1 + 0x20118 + local_180 * 0x2c);
                  local_1e0 = fVar1 + fVar1 + *(float *)(param_1 + 0x20a2c);
                  thunk_FUN_00561462(local_ac,&local_1e8,DAT_005ccf9c + 0xa98b);
                  thunk_FUN_00561462(local_34,local_ac,DAT_005ccf9c + 0xa97b);
                  if ((_DAT_005a3534 <= local_2c) && (local_2c < _DAT_005a3660)) {
                    local_18 = __ftol();
                    local_28 = __ftol();
                    if (((_DAT_005a58a8 * DAT_005b490c < (float)local_18) &&
                        (((float)local_18 <
                          _DAT_005a3704 * DAT_005b490c +
                          (float)(DAT_005ccf9c[0xa941] - DAT_005ccf9c[0xa979]) &&
                         (_DAT_005a58a8 * DAT_005b4910 < (float)local_28)))) &&
                       ((float)local_28 <
                        _DAT_005a3704 * DAT_005b4910 +
                        (float)(DAT_005ccf9c[0xa942] - DAT_005ccf9c[0xa97a]))) {
                      local_18c = 1;
                      break;
                    }
                  }
                }
                if (local_18c == 1) {
                  (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                            (DAT_005ccf9c[0xa8f9],5,2,param_1 + 0x20110,0x2c);
                }
              }
            }
LAB_005359e9:
          }
        }
      }
      if (2000 < (uint)(*(int *)(param_1 + 0xa8) - *(int *)(param_1 + 0xa4))) {
        *(undefined4 *)(param_1 + 0xa4) = *(undefined4 *)(param_1 + 0xa8);
      }
      if (DAT_005ccf9c[0xa8e4] == 1) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x8d,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(2,1,1);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x8d,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(2,1,1);
      }
      if (*(int *)(param_1 + 0xc0) == 1) {
        (**(code **)(*DAT_005ccf9c + 0x30))(8,3);
      }
      uVar4 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar4;
}

