// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004da607 | Name: FUN_004da607


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004da607(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  void *pvVar8;
  undefined4 *puVar9;
  float fVar10;
  undefined1 *puVar11;
  float fVar12;
  int local_5f0;
  int *local_594;
  undefined4 local_590;
  int *local_584;
  int *local_580;
  int *local_57c;
  int *local_570;
  int *local_56c;
  int *local_560;
  int *local_55c;
  int *local_554;
  int *local_54c;
  int *local_544;
  undefined1 local_518 [64];
  undefined1 local_4d8 [64];
  undefined1 local_498 [64];
  undefined1 local_458 [64];
  undefined1 local_418 [64];
  undefined1 local_3d8 [64];
  undefined1 local_398 [64];
  undefined1 local_358 [76];
  void *local_30c;
  int *local_308;
  void *local_2f8;
  undefined4 local_2f4;
  void *local_2d8;
  int *local_2d4;
  void *local_2c4;
  int *local_2c0;
  void *local_2b0;
  int *local_2ac;
  void *local_29c;
  int *local_298;
  void *local_288;
  int *local_284;
  void *local_274;
  int *local_270;
  void *local_260;
  int *local_25c;
  void *local_24c;
  int *local_248;
  void *local_238;
  int *local_234;
  void *local_230;
  int *local_22c;
  uint local_228 [5];
  undefined4 local_214;
  undefined4 local_210;
  undefined *local_20c;
  undefined *local_208;
  uint local_204;
  uint local_200;
  int local_1fc;
  uint local_1f8;
  uint local_1f4;
  int local_1f0;
  undefined4 local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  float local_1e0;
  undefined1 local_1dc [12];
  undefined1 local_1d0 [12];
  undefined1 local_1c4 [16];
  undefined1 local_1b4 [12];
  float local_1a8;
  undefined4 local_1a4 [12];
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  uint local_164;
  uint local_160;
  uint local_15c;
  uint local_158;
  uint local_154;
  undefined1 local_150 [12];
  undefined1 local_144 [12];
  undefined1 local_138 [12];
  undefined1 local_12c [12];
  undefined1 local_120 [16];
  undefined1 local_110 [16];
  undefined1 local_100 [16];
  undefined1 local_f0 [16];
  undefined1 local_e0 [16];
  int local_d0;
  int *local_cc;
  int local_c8;
  int *local_c4;
  undefined1 local_c0 [12];
  float local_b4;
  int *local_b0;
  int *local_ac;
  float local_a8;
  int local_a4;
  undefined1 local_a0 [12];
  float local_94;
  int *local_90;
  int *local_8c;
  float local_88;
  int local_84;
  float local_80 [4];
  int *local_70;
  int *local_6c;
  float local_68;
  int *local_64;
  float local_60;
  int *local_5c;
  float local_58;
  int *local_54;
  undefined4 local_50;
  uint local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  undefined1 local_34 [12];
  float local_28;
  float local_24;
  float local_20;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0ca9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  if (*(int *)(param_1 + 0x84) == 0) {
    uVar2 = 0;
  }
  else if (*(int *)(*(int *)(param_1 + 0x84) + 0x2f4) == 0) {
    uVar2 = 0;
  }
  else if (*(int *)(*(int *)(*(int *)(param_1 + 0x84) + 0x2f4) + 0x48) == 0) {
    uVar2 = 0;
  }
  else {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    FUN_00430f50(0,0,*(float *)(param_1 + 0x28e8) * _DAT_005a430c);
    thunk_FUN_0056144f(&local_28,local_34,param_1 + 0x2c);
    uVar2 = FUN_00430f50(local_28,local_24,local_20);
    FUN_004310a0(uVar2);
    FUN_004310a0(param_1 + 0x8c);
    if ((*(int *)(param_1 + 0x28f8) == 1) && (DAT_0092e660 == 0)) {
      if (100 < (uint)(iVar1 - *(int *)(param_1 + 0x2914))) {
        iVar3 = _rand();
        local_58 = (float)(iVar3 % 5);
        local_230 = operator_new(0x180);
        local_8 = 0;
        if (local_230 == (void *)0x0) {
          local_544 = (int *)0x0;
        }
        else {
          local_544 = (int *)FUN_004c8f70(0xb,1000,_DAT_005a40ec * local_58 + _DAT_005a4214,
                                          _DAT_005a442c * local_58,
                                          _DAT_005a40ec * local_58 + _DAT_005a4214,0,8,0x50);
        }
        local_22c = local_544;
        local_8 = 0xffffffff;
        local_54 = local_544;
        if (local_544 != (int *)0x0) {
          (**(code **)(*local_544 + 0x3c))(0xff111105);
          local_54[0x1b] = 1;
          local_54[0x41] = 1;
          FUN_004310a0(param_1 + 0x8c);
          FUN_0054ac09(local_54);
        }
      }
    }
    else if ((*(int *)(param_1 + 0x28f8) == 4) &&
            ((DAT_0092e660 == 0 && (0x1e < (uint)(iVar1 - *(int *)(param_1 + 0x2914)))))) {
      thunk_FUN_0056144f(&local_28,&local_44,param_1 + 0x2c);
      if (DAT_0092e660 == 0) {
        iVar3 = _rand();
        local_60 = (float)(iVar3 % 5);
        local_238 = operator_new(0x180);
        local_8 = 1;
        if (local_238 == (void *)0x0) {
          local_54c = (int *)0x0;
        }
        else {
          local_54c = (int *)FUN_004c8f70(0xb,1000,_DAT_005a4214 * local_60 + _DAT_005a415c,
                                          _DAT_005a4158 * local_60 + _DAT_005a415c,
                                          _DAT_005a4214 * local_60 + _DAT_005a415c,0,8,0x50);
        }
        local_234 = local_54c;
        local_8 = 0xffffffff;
        local_5c = local_54c;
        if (local_54c != (int *)0x0) {
          (**(code **)(*local_54c + 0x3c))(0xff0055ff);
          local_5c[0x1b] = 1;
          local_5c[0x41] = 0;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(local_5c);
        }
      }
      *(int *)(param_1 + 0x2914) = iVar1;
    }
    if ((*(char *)(param_1 + 0x28f0) == '\x01') && (DAT_0092e660 == 0)) {
      thunk_FUN_0056144f(&local_28,&local_44,param_1 + 0x2c);
      if (DAT_0092e660 == 0) {
        iVar3 = _rand();
        local_68 = (float)(iVar3 % 5);
        local_24c = operator_new(0x180);
        local_8 = 2;
        if (local_24c == (void *)0x0) {
          local_554 = (int *)0x0;
        }
        else {
          local_554 = (int *)FUN_004c8f70(0xb,1000,_DAT_005a4214 * local_68 + _DAT_005a415c,
                                          _DAT_005a4158 * local_68 + _DAT_005a415c,
                                          _DAT_005a4214 * local_68 + _DAT_005a415c,0,8,0x50);
        }
        local_248 = local_554;
        local_8 = 0xffffffff;
        local_64 = local_554;
        if (local_554 != (int *)0x0) {
          (**(code **)(*local_554 + 0x3c))(0xffffffff);
          local_64[0x1b] = 1;
          local_64[0x41] = 0;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(local_64);
        }
      }
      *(int *)(param_1 + 0x2914) = iVar1;
    }
    if ((((*(char *)(param_1 + 0x28f3) == '\x01') && (DAT_0092e660 == 0)) && (DAT_0092e664 == 0)) &&
       (0x32 < (uint)(iVar1 - *(int *)(param_1 + 0x2914)))) {
      iVar3 = _rand();
      local_80[3] = (float)(iVar3 % 5);
      local_260 = operator_new(0x180);
      local_8 = 3;
      if (local_260 == (void *)0x0) {
        local_55c = (int *)0x0;
      }
      else {
        local_55c = (int *)FUN_004c8f70(0x38,500,0x3e99999a,0x3e99999a,0x3e99999a,0x3b03126f,1,0x50)
        ;
      }
      local_25c = local_55c;
      local_8 = 0xffffffff;
      local_6c = local_55c;
      if (local_55c != (int *)0x0) {
        (**(code **)(*local_55c + 0x3c))(0xff550000);
        local_6c[0x1b] = 1;
        local_6c[0x41] = 1;
        uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
        FUN_004310a0(uVar2);
        FUN_0054ac09(local_6c);
      }
      local_274 = operator_new(0x180);
      local_8 = 4;
      if (local_274 == (void *)0x0) {
        local_560 = (int *)0x0;
      }
      else {
        local_560 = (int *)FUN_004c8f70(0x3c,500,local_80[3] * _DAT_005a45d4 + _DAT_005a414c,
                                        local_80[3] * _DAT_005a45d4 + _DAT_005a40ec,
                                        local_80[3] * _DAT_005a45d4 + _DAT_005a414c,0x3a03126f,1,
                                        0x50);
      }
      local_270 = local_560;
      local_8 = 0xffffffff;
      local_70 = local_560;
      if (local_560 != (int *)0x0) {
        (**(code **)(*local_560 + 0x3c))(0xff553333);
        local_70[0x45] = (int)((_DAT_005a4148 * local_80[3]) / _DAT_005a40f0);
        local_70[0x1b] = 1;
        local_70[0x41] = 1;
        uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
        FUN_004310a0(uVar2);
        FUN_0054ac09(local_70);
      }
      *(int *)(param_1 + 0x2914) = iVar1;
    }
    if (((*(char *)(param_1 + 0x28f1) == '\x01') && (DAT_0092e660 == 0)) &&
       (0x32 < (uint)(iVar1 - *(int *)(param_1 + 0x2914)))) {
      local_80[0] = -2.8312832e+38;
      local_80[1] = -2.835669e+38;
      local_80[2] = -1.7058253e+38;
      for (local_84 = 0; local_84 < 3; local_84 = local_84 + 1) {
        local_88 = *(float *)(param_1 + 0x28ec) / _DAT_005a3670;
        iVar3 = _rand();
        FUN_00430f50(0,0,*(float *)(param_1 + 0x28ec) - (float)(iVar3 % 10) * local_88);
        thunk_FUN_0056144f(&local_28,local_a0,param_1 + 0x2c);
        iVar3 = _rand();
        local_94 = (float)(iVar3 % 5);
        local_288 = operator_new(0x180);
        local_8 = 5;
        if (local_288 == (void *)0x0) {
          local_56c = (int *)0x0;
        }
        else {
          local_56c = (int *)FUN_004c8f70(0x38,500,_DAT_005a4158 - (float)local_84 * _DAT_005a45d4,
                                          _DAT_005a4158 - (float)local_84 * _DAT_005a45d4,
                                          _DAT_005a4158 - (float)local_84 * _DAT_005a45d4,0x3b03126f
                                          ,1,0x50);
        }
        local_284 = local_56c;
        local_8 = 0xffffffff;
        local_8c = local_56c;
        if (local_56c != (int *)0x0) {
          (**(code **)(*local_56c + 0x3c))(local_80[local_84]);
          local_8c[0x1b] = 1;
          local_8c[0x41] = 1;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(local_8c);
        }
        local_29c = operator_new(0x180);
        local_8 = 6;
        if (local_29c == (void *)0x0) {
          local_570 = (int *)0x0;
        }
        else {
          local_570 = (int *)FUN_004c8f70(0x3c,500,local_94 * _DAT_005a45d4 + _DAT_005a4158,
                                          local_94 * _DAT_005a45d4 + _DAT_005a4158,
                                          local_94 * _DAT_005a45d4 + _DAT_005a4158,0x3a03126f,1,0x50
                                         );
        }
        local_298 = local_570;
        local_8 = 0xffffffff;
        local_90 = local_570;
        if (local_570 != (int *)0x0) {
          (**(code **)(*local_570 + 0x3c))(0xff333355);
          local_90[0x45] = (int)((_DAT_005a4148 * local_94) / _DAT_005a40f0);
          local_90[0x1b] = 1;
          local_90[0x41] = 1;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(local_90);
        }
      }
      *(int *)(param_1 + 0x2914) = iVar1;
    }
    if (((*(char *)(param_1 + 0x28f5) == '\x01') && (DAT_0092e660 == 0)) &&
       (100 < (uint)(iVar1 - *(int *)(param_1 + 0x2914)))) {
      for (local_a4 = 0; local_a4 < 1; local_a4 = local_a4 + 1) {
        local_a8 = *(float *)(param_1 + 0x28ec) / _DAT_005a3670;
        iVar3 = _rand();
        FUN_00430f50(0,0,*(float *)(param_1 + 0x28ec) - (float)(iVar3 % 10) * local_a8);
        thunk_FUN_0056144f(&local_28,local_c0,param_1 + 0x2c);
        iVar3 = _rand();
        local_b4 = (float)(iVar3 % 5);
        local_2b0 = operator_new(0x180);
        local_8 = 7;
        if (local_2b0 == (void *)0x0) {
          local_57c = (int *)0x0;
        }
        else {
          local_57c = (int *)FUN_004c8f70(0x65,500,_DAT_005a4be4 - (float)local_a4 * _DAT_005a45d4,
                                          _DAT_005a4158 - (float)local_a4 * _DAT_005a45d4,
                                          _DAT_005a4158 - (float)local_a4 * _DAT_005a45d4,0x3b03126f
                                          ,8,0x50);
        }
        local_2ac = local_57c;
        local_8 = 0xffffffff;
        local_ac = local_57c;
        if (local_57c != (int *)0x0) {
          (**(code **)(*local_57c + 0x3c))(0xff5500ff);
          local_ac[0x1b] = 1;
          local_ac[0x41] = 1;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(local_ac);
        }
        local_2c4 = operator_new(0x180);
        local_8 = 8;
        if (local_2c4 == (void *)0x0) {
          local_580 = (int *)0x0;
        }
        else {
          local_580 = (int *)FUN_004c8f70(0x65,500,local_b4 * _DAT_005a45d4 + _DAT_005a4214,
                                          local_b4 * _DAT_005a45d4 + _DAT_005a4214,
                                          local_b4 * _DAT_005a45d4 + _DAT_005a4214,0x3a03126f,8,0x50
                                         );
        }
        local_2c0 = local_580;
        local_8 = 0xffffffff;
        local_b0 = local_580;
        if (local_580 != (int *)0x0) {
          (**(code **)(*local_580 + 0x3c))(0xffffffff);
          local_b0[0x45] = (int)((_DAT_005a4148 * local_b4) / _DAT_005a40f0);
          local_b0[0x1b] = 1;
          local_b0[0x41] = 1;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(local_b0);
        }
      }
      *(int *)(param_1 + 0x2914) = iVar1;
    }
    if (*(char *)(param_1 + 0x28f2) == '\0') {
      if (*(int *)(param_1 + 0x88) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x88));
        *(undefined4 *)(param_1 + 0x88) = 0;
      }
    }
    else {
      if (DAT_0092e660 == 0) {
        local_c8 = _rand();
        local_c8 = local_c8 % 5;
        local_2d8 = operator_new(0x180);
        local_8 = 9;
        if (local_2d8 == (void *)0x0) {
          local_584 = (int *)0x0;
        }
        else {
          local_584 = (int *)FUN_004c8f70(0,0x5dc,(float)local_c8 * _DAT_005a45d4 + _DAT_005a41e8,
                                          (float)local_c8 * _DAT_005a439c + _DAT_005a41e8,
                                          (float)local_c8 * _DAT_005a45d4 + _DAT_005a41e8,0x38d1b717
                                          ,1,0x50);
        }
        local_2d4 = local_584;
        local_8 = 0xffffffff;
        local_c4 = local_584;
        if (local_584 != (int *)0x0) {
          iVar3 = _rand();
          fVar12 = (float)(iVar3 % 10 + -5) * _DAT_005a41e8 + local_20;
          fVar10 = local_24;
          iVar3 = _rand();
          uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a41e8 + local_28,fVar10,fVar12);
          uVar2 = FUN_004310a0(uVar2);
          FUN_004310a0(uVar2);
          local_c4[0x1b] = 1;
          local_c4[0x40] = 0;
          local_c4[0x52] = 1;
          local_c4[0x50] = 0x3f800000;
          (**(code **)(*local_c4 + 0x3c))(0xff55aaff);
          FUN_0054ac09(local_c4);
        }
      }
      puVar4 = (undefined4 *)FUN_00430f50(0,0,*(undefined4 *)(param_1 + 0x28e8));
      local_44 = *puVar4;
      local_40 = puVar4[1];
      local_3c = puVar4[2];
      thunk_FUN_0056144f(&local_28,local_34,param_1 + 0x2c);
      if (*(int *)(param_1 + 0x88) == 0) {
        local_2f8 = operator_new(0x180);
        local_8 = 10;
        if (local_2f8 == (void *)0x0) {
          local_590 = 0;
        }
        else {
          local_590 = FUN_004c8f70(0x38,0,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
        }
        local_2f4 = local_590;
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x88) = local_590;
        if (*(int *)(param_1 + 0x88) != 0) {
          (**(code **)(**(int **)(param_1 + 0x88) + 0x3c))(0xff55aaff);
          *(undefined4 *)(*(int *)(param_1 + 0x88) + 0x6c) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x88) + 0x104) = 0;
          uVar2 = FUN_00430f50(local_28,local_24 + _DAT_005a4158,local_20);
          FUN_004310a0(uVar2);
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x88));
        }
      }
    }
    if ((*(char *)(param_1 + 0x28f4) == '\x01') && (DAT_0092e660 == 0)) {
      local_d0 = _rand();
      local_d0 = local_d0 % 5;
      local_30c = operator_new(0x180);
      local_8 = 0xb;
      if (local_30c == (void *)0x0) {
        local_594 = (int *)0x0;
      }
      else {
        local_594 = (int *)FUN_004c8f70(0,0x5dc,(float)local_d0 * _DAT_005a45d4 + _DAT_005a45d4,
                                        (float)local_d0 * _DAT_005a4ae0 + _DAT_005a45d4,
                                        (float)local_d0 * _DAT_005a45d4 + _DAT_005a45d4,0x38d1b717,1
                                        ,0x50);
      }
      local_308 = local_594;
      local_8 = 0xffffffff;
      local_cc = local_594;
      if (local_594 != (int *)0x0) {
        iVar3 = _rand();
        fVar12 = (float)(iVar3 % 10 + -5) * _DAT_005a41e8 + local_20;
        iVar3 = _rand();
        uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a41e8 + local_28,local_24,fVar12);
        uVar2 = FUN_004310a0(uVar2);
        FUN_004310a0(uVar2);
        local_cc[0x1b] = 1;
        local_cc[0x40] = 0;
        local_cc[0x52] = 1;
        local_cc[0x50] = -0x40800000;
        (**(code **)(*local_cc + 0x3c))(0xffffff55);
        FUN_0054ac09(local_cc);
      }
    }
    if (*(int *)(param_1 + 0x80) == 0) {
      if (*(int *)(param_1 + 0x28f8) == 4) {
        *(undefined4 *)(param_1 + 0x28f8) = 0;
      }
      uVar2 = 1;
    }
    else if (*(int *)(param_1 + 0x2904) == 0) {
      if (*(int *)(param_1 + 0x84) == 0) {
        uVar2 = 1;
      }
      else {
        local_18 = iVar1 - *(int *)(param_1 + 0x80);
        if (local_18 < 0x5dd) {
          if (*(int *)(*(int *)(param_1 + 0x84) + 0x2e4) == 0) {
            local_48 = 0x1e;
          }
          else {
            local_48 = *(undefined4 *)(*(int *)(param_1 + 0x84) + 0x2e4);
          }
          local_18 = __ftol();
          local_50 = 100;
          local_4c = local_18 / 100;
          local_14 = *(int *)(param_1 + 0x28d0) + -6;
          if (local_4c < 0x2f) {
            puVar4 = (undefined4 *)(param_1 + 0x2850);
            puVar9 = (undefined4 *)(param_1 + 0xf8c + local_4c * 0x40);
            for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
              *puVar9 = *puVar4;
              puVar4 = puVar4 + 1;
              puVar9 = puVar9 + 1;
            }
            FUN_00421910(local_120,0x10,5,FUN_00430f10);
            FUN_00421910(local_150,0xc,4,FUN_00430f10);
            if ((3 < (int)local_4c) && ((int)local_4c < 0x18)) {
              *(undefined4 *)(param_1 + 0x28dc) = 1;
              thunk_FUN_00562ad1(local_120,param_1 + 0x1c50 + (local_4c - 3) * 0x40);
              thunk_FUN_00562ad1(local_110,param_1 + 0x1c50 + (local_4c - 2) * 0x40);
              thunk_FUN_00562ad1(local_100,param_1 + 0x1c50 + (local_4c - 1) * 0x40);
              thunk_FUN_00562ad1(local_f0,param_1 + 0x1c50 + local_4c * 0x40);
              thunk_FUN_00562ad1(local_e0,param_1 + 0x1c50 + (local_4c + 1) * 0x40);
              local_164 = (local_4c - 3) * 100;
              local_160 = (local_4c - 2) * 100;
              local_15c = (local_4c - 1) * 100;
              local_158 = local_4c * 100;
              local_154 = (local_4c + 1) * 100;
              local_1f4 = local_18 - 0xfa;
              local_1f0 = 8;
              local_1e0 = 0.0;
              for (local_38 = 0; local_38 < 0x16; local_38 = local_38 + 1) {
                if (*(int *)(param_1 + 0x28d4) == *(int *)(&DAT_005bb318 + local_38 * 8)) {
                  local_1e0 = *(float *)(&DAT_005bb31c + local_38 * 8);
                  break;
                }
              }
              FUN_00430f50(0,0,0);
              FUN_00430f50(0,0,local_1e0 * *(float *)(param_1 + 0x28e8));
              FUN_00430f10();
              FUN_00430f10();
              FUN_00430f50(0,0,0);
              FUN_00430f50(0,0,0);
              FUN_004dc930(local_1c4);
              FUN_00430fc0(local_1a4);
              for (local_38 = 0; local_38 < 0x20; local_38 = local_38 + 2) {
                if (local_1f4 < local_160) {
                  if (local_160 == local_164) {
                    local_1a8 = 0.0;
                  }
                  else {
                    local_1a8 = (float)(local_1f4 - local_164) / (float)(int)(local_160 - local_164)
                    ;
                  }
                  thunk_FUN_005630fc(local_1c4,local_120,local_110,local_1a8);
                  FUN_004a8940(local_1b4,local_150,local_144,local_1a8);
                  thunk_FUN_0056272c(local_1a4,local_1c4);
                  iVar3 = param_1 + 0xf8c + local_4c * 0x40;
                  puVar11 = local_398;
                  FUN_004c5b90(local_358,local_1a4);
                  puVar4 = (undefined4 *)FUN_004c5b90(puVar11,iVar3);
                  puVar9 = local_1a4;
                  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
                    *puVar9 = *puVar4;
                    puVar4 = puVar4 + 1;
                    puVar9 = puVar9 + 1;
                  }
                  local_174 = *(undefined4 *)(param_1 + 0x5c);
                  local_170 = *(undefined4 *)(param_1 + 0x60);
                  local_16c = *(undefined4 *)(param_1 + 100);
                }
                else if ((local_1f4 < local_160) || (local_15c <= local_1f4)) {
                  if ((local_1f4 < local_15c) || (local_158 <= local_1f4)) {
                    if ((local_158 <= local_1f4) && (local_1f4 < local_154)) {
                      if (local_154 == local_158) {
                        local_1a8 = 0.0;
                      }
                      else {
                        local_1a8 = (float)(local_1f4 - local_158) /
                                    (float)(int)(local_154 - local_158);
                      }
                      thunk_FUN_005630fc(local_1c4,local_f0,local_e0,local_1a8);
                      thunk_FUN_0056272c(local_1a4,local_1c4);
                      iVar3 = param_1 + 0xf8c + local_4c * 0x40;
                      puVar11 = local_518;
                      FUN_004c5b90(local_4d8,local_1a4);
                      puVar4 = (undefined4 *)FUN_004c5b90(puVar11,iVar3);
                      puVar9 = local_1a4;
                      for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
                        *puVar9 = *puVar4;
                        puVar4 = puVar4 + 1;
                        puVar9 = puVar9 + 1;
                      }
                      local_174 = *(undefined4 *)(param_1 + 0x5c);
                      local_170 = *(undefined4 *)(param_1 + 0x60);
                      local_16c = *(undefined4 *)(param_1 + 100);
                    }
                  }
                  else {
                    if (local_158 == local_15c) {
                      local_1a8 = 0.0;
                    }
                    else {
                      local_1a8 = (float)(local_1f4 - local_15c) /
                                  (float)(int)(local_158 - local_15c);
                    }
                    thunk_FUN_005630fc(local_1c4,local_100,local_f0,local_1a8);
                    FUN_004a8940(local_1b4,local_138,local_12c,local_1a8);
                    thunk_FUN_0056272c(local_1a4,local_1c4);
                    iVar3 = param_1 + 0xf8c + local_4c * 0x40;
                    puVar11 = local_498;
                    FUN_004c5b90(local_458,local_1a4);
                    puVar4 = (undefined4 *)FUN_004c5b90(puVar11,iVar3);
                    puVar9 = local_1a4;
                    for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
                      *puVar9 = *puVar4;
                      puVar4 = puVar4 + 1;
                      puVar9 = puVar9 + 1;
                    }
                    local_174 = *(undefined4 *)(param_1 + 0x5c);
                    local_170 = *(undefined4 *)(param_1 + 0x60);
                    local_16c = *(undefined4 *)(param_1 + 100);
                  }
                }
                else {
                  if (local_15c == local_160) {
                    local_1a8 = 0.0;
                  }
                  else {
                    local_1a8 = (float)(local_1f4 - local_160) / (float)(int)(local_15c - local_160)
                    ;
                  }
                  thunk_FUN_005630fc(local_1c4,local_110,local_100,local_1a8);
                  FUN_004a8940(local_1b4,local_144,local_138,local_1a8);
                  thunk_FUN_0056272c(local_1a4,local_1c4);
                  iVar3 = param_1 + 0xf8c + local_4c * 0x40;
                  puVar11 = local_418;
                  FUN_004c5b90(local_3d8,local_1a4);
                  puVar4 = (undefined4 *)FUN_004c5b90(puVar11,iVar3);
                  puVar9 = local_1a4;
                  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
                    *puVar9 = *puVar4;
                    puVar4 = puVar4 + 1;
                    puVar9 = puVar9 + 1;
                  }
                  local_174 = *(undefined4 *)(param_1 + 0x5c);
                  local_170 = *(undefined4 *)(param_1 + 0x60);
                  local_16c = *(undefined4 *)(param_1 + 100);
                }
                thunk_FUN_00561462(&local_1ec,local_1d0,local_1a4);
                uVar2 = FUN_00430f50(local_1ec,local_1e8,local_1e4);
                FUN_004310a0(uVar2);
                thunk_FUN_00561462(&local_1ec,local_1dc,local_1a4);
                uVar2 = FUN_00430f50(local_1ec,local_1e8,local_1e4);
                FUN_004310a0(uVar2);
                local_1a8 = (float)local_38 / _DAT_005a40fc;
                if (_DAT_005a3660 < local_1a8) {
                  local_1a8 = 1.0;
                }
                *(float *)(param_1 + 0x9c + (local_38 + 1) * 0x18) = _DAT_005a3660 - local_1a8;
                *(undefined4 *)(param_1 + 0x9c + local_38 * 0x18) =
                     *(undefined4 *)(param_1 + 0x9c + (local_38 + 1) * 0x18);
                *(undefined4 *)(param_1 + 0xa0 + local_38 * 0x18) = 0x3f800000;
                *(undefined4 *)(param_1 + 0xa0 + (local_38 + 1) * 0x18) = 0x3ca3d70a;
                local_1a8 = local_1a8 * _DAT_005a4be0;
                uVar5 = __ftol();
                uVar6 = __ftol();
                uVar7 = __ftol();
                *(uint *)(param_1 + 0x98 + (local_38 + 1) * 0x18) =
                     (uVar5 & 0xff) << 0x10 | (uVar6 & 0xff) << 8 | uVar7 & 0xff;
                *(undefined4 *)(param_1 + 0x98 + local_38 * 0x18) =
                     *(undefined4 *)(param_1 + 0x98 + (local_38 + 1) * 0x18);
                local_1f4 = local_1f4 + local_1f0;
              }
              if (((('@' < *(char *)(param_1 + 0x28f6)) && (*(char *)(param_1 + 0x28f6) < 'u')) &&
                  (DAT_0092e660 == 0)) && (100 < (uint)(iVar1 - *(int *)(param_1 + 0x2918)))) {
                local_214 = 0x3030ff;
                local_210 = 0xb6b6;
                local_20c = &DAT_00b600b6;
                local_208 = &DAT_00ff1010;
                uVar5 = local_228[(int)*(char *)(param_1 + 0x28f6) / 0xd];
                local_1fc = (int)*(char *)(param_1 + 0x28f6) % 0xd;
                local_204 = (((uVar5 & 0xff0000) >> 0x10) * local_1fc) / 0x11;
                local_228[4] = (((uVar5 & 0xff00) >> 8) * local_1fc) / 0x11;
                local_200 = ((uVar5 & 0xff) * local_1fc) / 0x11;
                local_1f8 = local_204 << 0x10 | local_228[4] << 8 | local_200;
                local_228[3] = 0;
                if (*(int *)(*(int *)(param_1 + 0x84) + 0x2f4) != 0) {
                  pvVar8 = operator_new(0xcc);
                  local_8 = 0xc;
                  if (pvVar8 == (void *)0x0) {
                    local_5f0 = 0;
                  }
                  else {
                    local_5f0 = FUN_004d8910(*(undefined4 *)(*(int *)(param_1 + 0x84) + 0x2f4),
                                             *(undefined4 *)(param_1 + 0x28fc),
                                             *(undefined4 *)(param_1 + 0x28e8),local_1f8,local_1f8,
                                             0x3e99999a);
                  }
                  local_8 = 0xffffffff;
                  local_228[3] = local_5f0;
                  if (local_5f0 != 0) {
                    *(undefined4 *)(local_5f0 + 0x84) = 0x3e4ccccd;
                    FUN_0054ac09(local_5f0);
                  }
                }
                *(int *)(param_1 + 0x2918) = iVar1;
              }
            }
          }
          if (((int)local_4c < 5) || (local_14 < (int)local_4c)) {
            *(undefined4 *)(param_1 + 0x28dc) = 0;
          }
          if (local_14 + 1 < (int)local_4c) {
            for (local_38 = 0; local_38 < 0x30; local_38 = local_38 + 1) {
              FUN_00430fc0(param_1 + 0xf8c + local_38 * 0x40);
              FUN_00430fc0(param_1 + 0x38c + local_38 * 0x40);
              FUN_00430fc0(param_1 + 0x1c50 + local_38 * 0x40);
            }
          }
          uVar2 = 1;
        }
        else {
          *(undefined4 *)(param_1 + 0x80) = 0;
          if (*(int *)(param_1 + 0x28f8) == 4) {
            *(undefined4 *)(param_1 + 0x28f8) = 0;
          }
          if (*(char *)(param_1 + 0x28f7) != '\0') {
            *(undefined1 *)(param_1 + 0x28f7) = 0;
          }
          uVar2 = 1;
        }
      }
    }
    else {
      uVar2 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}

