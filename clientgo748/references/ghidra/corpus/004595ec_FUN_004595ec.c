// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004595ec | Name: FUN_004595ec


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_004595ec(int *param_1,undefined4 param_2,undefined4 param_3,float param_4,undefined4 param_5,
            float param_6,uint param_7,int param_8,int *param_9)

{
  byte bVar1;
  undefined2 uVar2;
  short sVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  void *pvVar7;
  uint uVar8;
  size_t sVar9;
  int iVar10;
  float10 fVar11;
  undefined4 uVar12;
  int local_510;
  int local_50c;
  float local_4cc;
  int local_4c8;
  int local_4c4;
  int local_494;
  int local_490;
  int local_48c;
  int local_488;
  int local_484;
  int local_480;
  uint local_47c;
  int *local_470;
  int local_46c;
  int local_468;
  undefined1 local_45c [4];
  undefined4 local_458;
  undefined2 local_450;
  undefined2 local_44e;
  undefined2 local_44c;
  undefined2 local_44a;
  undefined2 local_448;
  undefined2 local_446;
  short local_444;
  undefined2 local_442;
  undefined1 local_440;
  undefined1 local_43f;
  undefined1 local_43e;
  undefined2 local_430 [26];
  undefined1 local_3fc [6];
  undefined2 local_3f6;
  undefined1 local_3de;
  byte local_3dd;
  int *local_39c;
  uint local_398;
  undefined4 local_394;
  int local_390;
  int local_38c;
  int local_388;
  int local_384;
  int local_380;
  int local_37c;
  int local_378;
  int local_374;
  int local_370;
  int local_36c;
  int local_368;
  uint local_364;
  int local_360;
  int local_35c;
  int local_358;
  uint local_354;
  int local_350;
  int local_34c;
  undefined1 local_348 [6];
  undefined2 local_342;
  undefined1 local_32a;
  byte local_329;
  int local_2e8;
  undefined4 local_2e4;
  int local_2e0;
  int local_2dc;
  byte local_2d8;
  undefined3 uStack_2d7;
  int local_2d4;
  int local_2d0;
  int local_2cc;
  int local_2c8;
  int local_2c4;
  int local_2c0;
  int local_2bc;
  int local_2b8;
  int local_2b4;
  undefined4 local_2b0;
  undefined4 local_2ac;
  uint local_2a8;
  uint local_2a4;
  undefined1 local_2a0 [4];
  undefined2 local_29c;
  undefined2 local_29a;
  short local_294;
  short local_292;
  undefined4 local_290;
  undefined4 local_28c;
  undefined2 local_288;
  undefined2 uStack_286;
  char local_26c [48];
  int local_23c;
  int local_238;
  int local_234;
  int local_230;
  int local_22c;
  int *local_228;
  int local_224;
  int local_220;
  int local_21c;
  int local_218;
  int local_214;
  int local_210;
  int local_20c;
  int local_208;
  int local_204 [24];
  int local_1a4;
  undefined2 local_1a0;
  int local_19c;
  float local_198;
  float local_194;
  float local_190;
  int local_18c;
  int local_188;
  int local_184;
  int local_180;
  int local_17c;
  uint local_178;
  uint local_174;
  uint local_170;
  uint local_16c;
  int local_168;
  uint local_164;
  uint local_160;
  uint local_15c;
  uint local_158;
  undefined4 local_154;
  undefined4 local_150;
  uint local_14c;
  int local_148;
  int local_144;
  int local_140;
  int local_13c;
  int local_138;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  uint local_118;
  uint local_114;
  int local_110;
  uint local_10c;
  int *local_108;
  int local_104;
  int local_100;
  int local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  int local_ec;
  int local_e8;
  int local_e4;
  int local_e0;
  int local_dc;
  float local_d8;
  uint local_d4;
  float local_d0;
  int local_cc;
  uint local_c8;
  int *local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  float local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  undefined1 local_a0 [4];
  undefined4 local_9c;
  undefined2 local_94;
  undefined2 local_92;
  ushort local_90;
  ushort local_8e;
  ushort local_8c;
  ushort uStack_8a;
  short sStack_88;
  undefined2 local_86;
  undefined1 local_84;
  undefined1 local_83;
  undefined1 local_82;
  undefined4 local_74;
  uint local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int *local_28;
  int local_24;
  int *local_20;
  int local_1c;
  char local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f685;
  local_10 = ExceptionList;
  if (*(int *)(param_1[0xa304] + 0x28) == 1) {
    return 0;
  }
  if (*(int *)(DAT_005d0504 + 0x434) == 1) {
    return 0;
  }
  if (*(char *)(param_1[0x13] + 0x795) == '\x01') {
    return 0;
  }
  ExceptionList = &local_10;
  param_1[0x9ba6] = 0;
  local_1c = 0;
  local_28 = DAT_013b71e8 + 0x1bb;
  if ((local_28 != (int *)0x0) && ((DAT_013b71e8[0x272] & 0x20000000U) != 0)) {
    local_1c = 1;
  }
  if (param_9 == (int *)0x0) {
    local_20 = (int *)param_1[0x1f];
  }
  else {
    local_20 = param_9;
  }
  if (*(int *)(DAT_013b71e0 + 0x10c) == 1) {
    param_1[0x9ba4] = 0;
  }
  local_18 = *(char *)((int)DAT_013b71e8 + *(char *)((int)DAT_013b71e8 + 0xef5) + 0xef6);
  local_14 = -1;
  iVar4 = FUN_00434228((int)local_18);
  if (iVar4 == 0) {
    ExceptionList = local_10;
    return 0;
  }
  if (local_18 == 'c') {
    ExceptionList = local_10;
    return 0;
  }
  if (local_18 == 'T') {
    ExceptionList = local_10;
    return 0;
  }
  if (local_18 == 'a') {
    local_2c = 0;
    for (local_30 = 0; local_30 < 0x60; local_30 = local_30 + 1) {
      local_34 = (**(code **)(*DAT_013b71e8 + 0x38))(local_30 + 0x3a99);
      if ((local_34 != 0) && (*(short *)(local_34 + 0x160) == 0x2ea)) {
        local_38 = local_34;
        iVar4 = __ftol();
        iVar5 = __ftol();
        if (iVar4 == iVar5) {
          iVar4 = __ftol();
          iVar5 = __ftol();
          if (iVar4 == iVar5) {
            local_2c = 1;
            local_14 = local_30 + 0x3a99;
            break;
          }
        }
      }
    }
    if (local_2c == 0) {
      uVar6 = FUN_00455ea7(param_7);
      ExceptionList = local_10;
      return uVar6;
    }
  }
  if ((*(int *)(param_1[0x13] + 0x740) == 2) && (*(int *)(param_1[0x13] + 0x740) == 3)) {
    ExceptionList = local_10;
    return 0;
  }
  if ((local_18 < '\0') || ('g' < local_18)) {
    uVar6 = FUN_00455ea7(param_7);
    ExceptionList = local_10;
    return uVar6;
  }
  if ((uint)*(ushort *)((int)param_1 + 0x275ae) == *(uint *)(param_1[0x13] + 0x20)) {
    uVar6 = FUN_00455ea7(param_7);
    ExceptionList = local_10;
    return uVar6;
  }
  if (*(int *)(&DAT_0092ff50 + local_18 * 0x60) == 1) {
    uVar6 = FUN_00455ea7(param_7);
    ExceptionList = local_10;
    return uVar6;
  }
  if (((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1) && (local_20 != (int *)0x0)) &&
     ((char)local_20[0x92] == '\x01')) {
    uVar6 = FUN_00455ea7(param_7);
    ExceptionList = local_10;
    return uVar6;
  }
  if ((((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1) && (local_20 != (int *)0x0)) &&
      ((*(short *)(param_1[0x13] + 0x680) != 0 && (*(short *)((int)DAT_013b71e8 + 0xf12) != 0)))) &&
     ((*(short *)(param_1[0x13] + 0x680) == (short)local_20[0x1a0] ||
      (*(short *)((int)DAT_013b71e8 + 0xf12) == (short)local_20[0x1a0])))) {
    uVar6 = FUN_00455ea7(param_7);
    ExceptionList = local_10;
    return uVar6;
  }
  if (DAT_005d03f8 == 0) {
    if (((DAT_005d0404 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
       (((('\0' < *(char *)(param_1[0x13] + 0x79b) &&
          ((local_20 != (int *)0x0 && (*(char *)((int)local_20 + 0x79b) == '\0')))) &&
         ((uint)local_20[8] < 1000)) && (iVar4 = (**(code **)(*local_20 + 0x54))(), iVar4 == 1)))) {
      uVar6 = FUN_00455ea7(param_7);
      ExceptionList = local_10;
      return uVar6;
    }
    if ((((DAT_005d0404 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
        (local_20 != (int *)0x0)) &&
       (((*(char *)((int)local_20 + 0x249) == '\x01' &&
         ((short)DAT_013b71e8[0x3c4] != (short)local_20[0x1a0])) && ((short)local_20[0x1a0] != 0))))
    {
      uVar6 = FUN_00455ea7(param_7);
      ExceptionList = local_10;
      return uVar6;
    }
  }
  else {
    if (((((DAT_005d0404 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
         (local_20 != (int *)0x0)) &&
        ((*(char *)((int)local_20 + 0x79b) == *(char *)(param_1[0x13] + 0x79b) &&
         ((uint)local_20[8] < 1000)))) && (iVar4 = (**(code **)(*local_20 + 0x54))(), iVar4 == 1)) {
      uVar6 = FUN_00455ea7(param_7);
      ExceptionList = local_10;
      return uVar6;
    }
    if ((((DAT_005d0404 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
        ((local_20 != (int *)0x0 &&
         (((*(char *)((int)local_20 + 0x249) == '\x01' &&
           ((short)DAT_013b71e8[0x3c4] != (short)local_20[0x1a0])) && ((short)local_20[0x1a0] != 0))
         )))) && (iVar4 = (**(code **)(*local_20 + 0x58))(), iVar4 == 0)) {
      uVar6 = FUN_00455ea7(param_7);
      ExceptionList = local_10;
      return uVar6;
    }
  }
  local_24 = *(int *)(&DAT_0092ff04 + local_18 * 0x60);
  if ((8 < param_1[0x9b9e]) && (1 < local_24)) {
    local_24 = local_24 + -1;
  }
  if (param_7 < (uint)(*(int *)(&DAT_0092eaf8 + local_18 * 4) + local_24 * 1000)) {
    ExceptionList = local_10;
    return 0;
  }
  if ((param_7 <= param_1[0x9d18] + 1000U) ||
     ((((*(int *)(&DAT_0092fefc + local_18 * 0x60) != 0 &&
        (*(int *)(&DAT_0092fefc + local_18 * 0x60) != 3)) &&
       (*(int *)(&DAT_0092fefc + local_18 * 0x60) != 4)) &&
      ((*(int *)(&DAT_0092fefc + local_18 * 0x60) != 5 &&
       (*(int *)(&DAT_0092fefc + local_18 * 0x60) != 6)))))) goto LAB_0045cc76;
  local_40 = (uint)*(short *)(param_1[0x13] + 0x444);
  local_3c = (int)local_18 + (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18;
  if (local_18 < '`') {
    local_40 = (uint)*(byte *)((int)DAT_013b71e8 +
                              ((int)(local_3c + (local_3c >> 0x1f & 7U)) >> 3) + 0x745);
  }
  iVar4 = FUN_00555790((int)local_18,*(undefined1 *)((int)DAT_013b71e8 + 0x9d3),local_40);
  if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar4) {
    local_a4 = param_1[0x9eba];
    pvVar7 = operator_new(0xe50);
    local_8 = 0;
    if (pvVar7 == (void *)0x0) {
      local_4c4 = 0;
    }
    else {
      local_4c4 = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    local_a8 = local_4c4;
    if (local_4c4 != 0) {
      FUN_00408d33(local_4c4);
    }
    if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x21), iVar4 != 0)) {
      uVar12 = 0;
      uVar6 = 0;
      FUN_00429a6d(0x21);
      FUN_0042ad2b(uVar6,uVar12);
    }
    ExceptionList = local_10;
    return 0;
  }
  if ((local_18 == 'V') &&
     (iVar4 = (uint)*(byte *)(param_1[0x13] + 0x45e) * 100,
     iVar4 - DAT_013b71e8[0x1c1] != 0 && DAT_013b71e8[0x1c1] <= iVar4)) {
    local_ac = param_1[0x9eba];
    pvVar7 = operator_new(0xe50);
    local_8 = 1;
    if (pvVar7 == (void *)0x0) {
      local_4c8 = 0;
    }
    else {
      local_4c8 = FUN_00407203(&DAT_00a38ef8,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    local_b0 = local_4c8;
    if (local_4c8 != 0) {
      FUN_00408d33(local_4c8);
    }
    if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x21), iVar4 != 0)) {
      uVar12 = 0;
      uVar6 = 0;
      FUN_00429a6d(0x21);
      FUN_0042ad2b(uVar6,uVar12);
    }
    ExceptionList = local_10;
    return 0;
  }
  iVar4 = (**(code **)(*(int *)param_1[0x13] + 0x50))();
  if ((iVar4 == 1) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
    ExceptionList = local_10;
    return 0;
  }
  _memset(local_a0,0,0x60);
  local_9c = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x36c);
  local_94 = *(undefined2 *)(param_1[0x13] + 0x20);
  local_90 = __ftol();
  local_8e = __ftol();
  local_8c = __ftol();
  uStack_8a = __ftol();
  if (param_1[0xa1e5] != 0) {
    local_90 = *(ushort *)(param_1 + 0xa1e5);
    local_8e = *(ushort *)(param_1 + 0xa1e6);
    local_8c = local_90;
    uStack_8a = local_8e;
  }
  local_86 = 0xffff;
  sStack_88 = (short)local_18;
  local_83 = 0;
  local_84 = 0xff;
  if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 5) {
    local_82 = 0;
    local_b4 = 0;
    local_c8 = (uint)*(byte *)((int)DAT_013b71e8 + 0x9d2);
    local_c4 = *(int **)(param_1[0xd] + 0x10);
    if (local_c4 == (int *)0x0) {
      ExceptionList = local_10;
      return 1;
    }
    local_d0 = param_4 - *(float *)(param_1[0x13] + 0x28);
    local_d8 = param_6 - *(float *)(param_1[0x13] + 0x2c);
    fVar11 = (float10)FUN_00493c30(local_d0,local_d8);
    local_b8 = (float)fVar11;
    local_d4 = *(byte *)(param_1[0x13] + 0x45f) / 0x4b;
    if (3 < local_d4) {
      local_d4 = 3;
    }
    local_bc = __ftol();
    local_c0 = __ftol();
    local_cc = 0;
LAB_0045a185:
    do {
      while( true ) {
        if ((local_c4 == (int *)0x0) || (1000 < local_cc)) goto LAB_0045c794;
        local_cc = local_cc + 1;
        if (local_c4 != (int *)param_1[0x13]) break;
        local_c4 = (int *)local_c4[3];
      }
      local_dc = __ftol();
      local_e0 = __ftol();
      local_e4 = __ftol();
      local_e8 = __ftol();
      if (param_1[0xa1e5] != 0) {
        local_e4 = param_1[0xa1e5];
        local_e8 = param_1[0xa1e6];
      }
      local_ec = FUN_005563a0(local_e4,local_e8,local_dc,local_e0);
      if (((local_c4[0x93] == 0x38) && (*(short *)((int)local_c4 + 0x1d2) == 0)) &&
         (local_ec = local_ec + -0xc, local_ec < 0)) {
        local_ec = 0;
      }
      if (local_ec <= (int)(local_d4 + 3)) {
        local_f0 = (float)local_c4[10] - *(float *)(param_1[0x13] + 0x28);
        local_f4 = (float)local_c4[0xb] - *(float *)(param_1[0x13] + 0x2c);
        fVar11 = (float10)FUN_00493c30(local_f0,local_f4);
        local_f8 = (float)fVar11;
        if (local_f8 - local_b8 <= _DAT_005a3534) {
          local_4cc = -(local_f8 - local_b8);
        }
        else {
          local_4cc = local_f8 - local_b8;
        }
        if (local_4cc < _DAT_005a437c) {
          local_100 = local_dc;
          local_fc = local_e0;
          FUN_00556416(local_e4,local_e8,&local_100,&local_fc,param_1 + 0x25,8);
          if ((local_c4[0x93] == 0x38) && (*(short *)((int)local_c4 + 0x1d2) == 0)) {
            local_100 = local_dc;
            local_fc = local_e0;
          }
          if ((local_100 != local_dc) || (local_fc != local_e0)) {
            local_c4 = (int *)local_c4[3];
            goto LAB_0045a185;
          }
          iVar4 = (**(code **)(*local_c4 + 0x54))();
          if (((iVar4 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
             ((uint)local_c4[8] < 1000)) {
            local_c4 = (int *)local_c4[3];
            goto LAB_0045a185;
          }
          iVar4 = (**(code **)(*local_c4 + 0x54))();
          if ((iVar4 == 1) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
            if (((uint)local_c4[8] < 1000) &&
               ((DAT_005d0404 == 0 && ((short)DAT_013b71e8[0x3c4] != (short)local_c4[0x1a0])))) {
              if (DAT_005d03f8 == 0) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
              if ((*(char *)(param_1[0x13] + 0x79b) != '\0') &&
                 (*(char *)((int)local_c4 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
            }
            if ((((DAT_005d0404 == 0) && (*(char *)((int)local_c4 + 0x249) == '\x01')) &&
                ((short)DAT_013b71e8[0x3c4] != (short)local_c4[0x1a0])) &&
               ((short)local_c4[0x1a0] != 0)) {
              if (DAT_005d03f8 == 0) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
              if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
                  (*(char *)((int)local_c4 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) &&
                 (iVar4 = (**(code **)(*local_c4 + 0x58))(), iVar4 == 0)) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
            }
            if (((DAT_005d0404 == 0) && (*(char *)((int)local_c4 + 0x249) == '\x01')) &&
               ((short)local_c4[0x1a0] == 0)) {
              if (DAT_005d03f8 == 0) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
              if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
                  (*(char *)((int)local_c4 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) &&
                 (iVar4 = (**(code **)(*local_c4 + 0x58))(), iVar4 == 0)) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
            }
            if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
                ('\0' < *(char *)((int)local_c4 + 0x79b))) &&
               ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_c4 + 0x79b) &&
                (DAT_005d0404 == 0)))) {
              if (DAT_005d03f8 != 0) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
              if (999 < (uint)local_c4[8]) {
                local_c4 = (int *)local_c4[3];
                goto LAB_0045a185;
              }
            }
            if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
               ((iVar4 = (**(code **)(*local_c4 + 0x58))(), iVar4 == 0 && ((uint)local_c4[8] < 1000)
                ))) {
              local_c4 = (int *)local_c4[3];
              goto LAB_0045a185;
            }
            if ((char)local_c4[0x92] == '\x01') {
              local_c4 = (int *)local_c4[3];
              goto LAB_0045a185;
            }
          }
          if (*(char *)((int)local_c4 + 0x245) == '\x01') {
            local_c4 = (int *)local_c4[3];
            goto LAB_0045a185;
          }
          *(short *)(&local_74 + local_b4) = (short)local_c4[8];
          *(undefined2 *)((int)&local_74 + local_b4 * 4 + 2) = 0xffff;
          local_b4 = local_b4 + 1;
          local_8c = (ushort)local_dc;
          uStack_8a = (ushort)local_e0;
        }
      }
      local_c4 = (int *)local_c4[3];
      if ((*(int *)(&DAT_0092ff44 + local_18 * 0x60) <= local_b4) || (0xc < local_b4))
      goto LAB_0045c794;
    } while( true );
  }
  if (((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3) ||
      (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 4)) ||
     (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 6)) {
    local_82 = 0;
    local_104 = 0;
    local_10c = (uint)*(byte *)((int)DAT_013b71e8 + 0x9d2);
    local_108 = *(int **)(param_1[0xd] + 0x10);
    if (local_108 == (int *)0x0) {
      ExceptionList = local_10;
      return 0;
    }
    local_114 = __ftol();
    local_118 = __ftol();
    if (((local_114 == 0) && (local_118 == 0)) && (local_20 != (int *)0x0)) {
      local_114 = __ftol();
      local_118 = __ftol();
    }
    local_110 = 0;
LAB_0045aa20:
    while( true ) {
      while( true ) {
        if ((local_108 == (int *)0x0) || (1000 < local_110)) goto LAB_0045bff9;
        local_110 = local_110 + 1;
        local_11c = __ftol();
        local_124 = __ftol();
        if ((local_18 == '\0') ||
           ((((((local_18 == '\x02' || (local_18 == '\a')) || (local_18 == '\x11')) ||
              ((local_18 == '\x17' || (local_18 == '#')))) || (local_18 == '\'')) ||
            ((local_18 == '3' || (local_18 == '7')))))) {
          if (local_20 == (int *)0x0) {
            ExceptionList = local_10;
            return 1;
          }
          local_114 = __ftol();
          local_114 = local_114 & 0xffff;
          local_118 = __ftol();
          local_118 = local_118 & 0xffff;
          if ((local_20[0x93] == 0x38) && (*(short *)((int)local_20 + 0x1d2) == 0)) {
            local_140 = __ftol();
            local_144 = __ftol();
            if (param_1[0xa1e5] != 0) {
              local_140 = param_1[0xa1e5];
              local_144 = param_1[0xa1e6];
            }
            uVar6 = __ftol();
            uVar6 = __ftol(uVar6);
            local_148 = FUN_005563a0(local_140,local_144,uVar6);
            local_148 = local_148 + -0xc;
            if (local_148 < 1) {
              local_114 = __ftol();
              local_118 = __ftol();
            }
          }
        }
        if ((local_18 != '_') || (local_108 == local_20)) break;
        if (local_20 == (int *)0x0) {
          ExceptionList = local_10;
          return 0;
        }
        local_108 = (int *)local_108[3];
        local_114 = __ftol();
        local_118 = __ftol();
      }
      if (((local_108 == (int *)0x0) ||
          (((short)local_108[0x1a0] == 0 || (*(int *)(&DAT_0092ff40 + local_18 * 0x60) != 1)))) ||
         ((*(short *)(param_1[0x13] + 0x680) != (short)local_108[0x1a0] &&
          (*(short *)((int)DAT_013b71e8 + 0xf12) != (short)local_108[0x1a0])))) break;
      local_108 = (int *)local_108[3];
    }
    local_12c = __ftol();
    local_130 = __ftol();
    if (param_1[0xa1e5] != 0) {
      local_12c = param_1[0xa1e5];
      local_130 = param_1[0xa1e6];
    }
    local_134 = FUN_005563a0(local_12c,local_130,local_114,local_118);
    if (((local_18 == '\0') && (local_134 == 2)) &&
       ((uVar8 = __ftol(), local_114 == uVar8 || (uVar8 = __ftol(), local_118 == uVar8)))) {
      local_134 = 1;
    }
    if ((((local_20 != (int *)0x0) && (local_20[0x93] == 0x38)) &&
        (*(short *)((int)local_20 + 0x1d2) == 0)) && (local_134 = local_134 + -0xc, local_134 < 0))
    {
      local_134 = 0;
    }
    if ((((local_18 == '\0') || (local_18 == '#')) ||
        (((local_18 == '\x02' || ((local_18 == '\a' || (local_18 == '\x11')))) ||
         (local_18 == '\x17')))) ||
       ((((local_18 == '\'' || (local_18 == '3')) || (local_18 == '7')) ||
        ((local_18 == '_' || (local_18 == 'a')))))) {
      if ((*(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c < local_134) &&
         (*(int *)(&DAT_0092ff08 + local_18 * 0x60) != -1)) {
        if ((((local_18 == '\0') || (local_18 == '\x02')) || (local_18 == '\a')) ||
           (((((local_18 == '\x11' || (local_18 == '\x17')) || (local_18 == '#')) ||
             ((local_18 == '\'' || (local_18 == '3')))) || ((local_18 == '7' || (local_18 == '_'))))
           )) {
          local_160 = local_114;
          local_164 = local_118;
          local_16c = local_114;
          local_15c = local_118;
          local_168 = 1;
          uVar6 = __ftol(&local_160,&local_164,param_1 + 0x25,8);
          uVar6 = __ftol(uVar6);
          FUN_00556416(uVar6);
          if ((local_160 != local_114) || (local_164 != local_118)) {
            local_168 = 0;
          }
          if (((param_8 == 1) && (*(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c <= local_134)
              ) && (local_168 == 1)) {
            if ((3 < *(int *)(param_1[0x13] + 0x740)) && (*(int *)(param_1[0x13] + 0x740) < 10)) {
              local_170 = *(uint *)(&DAT_0065a4a8 +
                                   *(int *)(*(int *)(param_1[0x13] + 0x60) + 0x2d0) * 4 +
                                   *(int *)(param_1[0x13] + 0x5c) * 900);
              if (2 < local_170) {
                local_170 = local_170 - 2;
              }
              if ((*(int *)(DAT_005d0504 + 0x1c) != 0) &&
                 (param_7 < *(int *)(param_1[0x13] + 0x600) +
                            *(int *)(*(int *)(param_1[0x13] + 0x60) + 0x2e4) * local_170 * 4)) {
                ExceptionList = local_10;
                return 1;
              }
            }
            if (param_1[0x10] != 0) {
              FUN_004fa8a1(param_1[0x9b9f]);
              iVar4 = __ftol();
              iVar5 = __ftol();
              if (iVar4 == iVar5) {
                iVar4 = __ftol();
                iVar5 = __ftol();
                if (iVar4 == iVar5) {
                  ExceptionList = local_10;
                  return 1;
                }
              }
              param_1[0x9ba6] = 0;
              *(int **)(param_1[0x13] + 0x76c) = local_20;
              param_1[0x9d35] = param_7;
              local_174 = __ftol();
              local_178 = __ftol();
              if (param_1[0xa1e5] != 0) {
                local_174 = param_1[0xa1e5];
                local_178 = param_1[0xa1e6];
              }
              if (local_20 != (int *)0x0) {
                local_160 = __ftol();
                local_164 = __ftol();
                local_17c = FUN_005563a0(local_174,local_178,local_160,local_164);
                local_184 = 1;
                local_188 = 1;
                if ((int)local_160 < (int)local_174) {
                  local_184 = -1;
                }
                if ((int)local_164 < (int)local_178) {
                  local_188 = -1;
                }
                local_180 = 0;
                while (*(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c < local_17c) {
                  if (500 < local_180) {
                    ExceptionList = local_10;
                    return 1;
                  }
                  if (local_174 != local_160) {
                    local_174 = local_174 + local_184;
                  }
                  if (local_178 != local_164) {
                    local_178 = local_178 + local_188;
                  }
                  local_17c = FUN_005563a0(local_174,local_178,local_160,local_164);
                  local_180 = local_180 + 1;
                }
                param_1[0x9d6c] = local_174;
                param_1[0x9d6d] = local_178;
              }
            }
          }
        }
        ExceptionList = local_10;
        return 0;
      }
      if (local_18 == 'a') {
        if (local_134 < 4) {
          ExceptionList = local_10;
          return 1;
        }
        local_18c = (**(code **)(*DAT_013b71e8 + 0x38))(local_14);
        iVar4 = __ftol();
        iVar4 = __ftol((float)(int)(local_118 - iVar4));
        fVar11 = (float10)FUN_00493c30((float)(int)(local_114 - iVar4));
        local_198 = (float)fVar11;
        if (local_198 < _DAT_005a3534) {
          local_198 = local_198 + _DAT_005a4290;
        }
        local_194 = *(float *)(local_18c + 0x34) - _DAT_005a4380;
        local_190 = *(float *)(local_18c + 0x34) + _DAT_005a4380;
        if (local_198 < local_194) {
          ExceptionList = local_10;
          return 1;
        }
        if (local_190 < local_198) {
          ExceptionList = local_10;
          return 1;
        }
      }
      if (((local_18 == '#') &&
          ((((uint)local_20[8] < 1000 || ((char)local_20[0x92] == '\x01')) && (iRam00930c60 == 1))))
         && (iVar4 = (**(code **)(*local_20 + 0x54))(), iVar4 == 0)) {
        ExceptionList = local_10;
        return 1;
      }
      if (*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) {
        local_19c = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b);
        local_19c = local_19c + local_1c;
        if (local_19c < local_134) {
          ExceptionList = local_10;
          return 1;
        }
      }
      local_150 = __ftol();
      local_154 = __ftol();
      local_158 = local_114;
      local_14c = local_118;
      FUN_00556416(local_150,local_154,&local_158,&local_14c,param_1 + 0x25,8);
      if (((local_20 != (int *)0x0) && (local_20[0x93] == 0x38)) &&
         (*(short *)((int)local_20 + 0x1d2) == 0)) {
        local_158 = local_114;
        local_14c = local_118;
      }
      if ((local_158 != local_114) || (local_14c != local_118)) {
        ExceptionList = local_10;
        return 1;
      }
    }
    else {
      local_114 = __ftol();
      local_114 = local_114 & 0xffff;
      local_118 = __ftol();
      local_118 = local_118 & 0xffff;
      if (param_1[0xa1e5] != 0) {
        local_114 = param_1[0xa1e5];
        local_118 = param_1[0xa1e6];
      }
    }
    local_13c = FUN_005563a0(local_114,local_118,local_11c,local_124);
    local_120 = local_11c;
    local_128 = local_124;
    FUN_00556927(local_114,local_118,&local_120,&local_128,param_1 + 0x25,8);
    local_138 = -1;
    if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3) {
      local_138 = 1;
    }
    else if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 4) {
      local_138 = 2;
    }
    else if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 6) {
      local_138 = 3;
    }
    iVar4 = (**(code **)(*local_108 + 0x54))();
    if ((iVar4 == 1) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
      if (((uint)local_108[8] < 1000) &&
         ((DAT_005d0404 == 0 && ((short)DAT_013b71e8[0x3c4] != (short)local_108[0x1a0])))) {
        if (DAT_005d03f8 == 0) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
        if ((*(char *)(param_1[0x13] + 0x79b) != '\0') &&
           (*(char *)((int)local_108 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
      }
      if ((((DAT_005d0404 == 0) && (*(char *)((int)local_108 + 0x249) == '\x01')) &&
          ((short)DAT_013b71e8[0x3c4] != (short)local_108[0x1a0])) && ((short)local_108[0x1a0] != 0)
         ) {
        if (DAT_005d03f8 == 0) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
        if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
            (*(char *)((int)local_108 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) &&
           (iVar4 = (**(code **)(*local_108 + 0x58))(), iVar4 == 0)) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
      }
      if (((DAT_005d0404 == 0) && (*(char *)((int)local_108 + 0x249) == '\x01')) &&
         ((short)local_108[0x1a0] == 0)) {
        if (DAT_005d03f8 == 0) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
        if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
            (*(char *)((int)local_108 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) &&
           (iVar4 = (**(code **)(*local_108 + 0x58))(), iVar4 == 0)) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
      }
      if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) && ('\0' < *(char *)((int)local_108 + 0x79b)))
         && ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_108 + 0x79b) &&
             (DAT_005d0404 == 0)))) {
        if (DAT_005d03f8 != 0) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
        if (999 < (uint)local_108[8]) {
          local_108 = (int *)local_108[3];
          goto LAB_0045aa20;
        }
      }
      if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
         ((iVar4 = (**(code **)(*local_108 + 0x58))(), iVar4 == 0 && ((uint)local_108[8] < 1000))))
      {
        local_108 = (int *)local_108[3];
        goto LAB_0045aa20;
      }
      if ((((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
          (*(char *)(param_1[0x13] + 0x79b) == '\x03')) &&
         (((uint)local_108[8] < 1000 ||
          (('\0' < *(char *)((int)local_108 + 0x79b) &&
           (*(char *)((int)local_108 + 0x79b) != '\x04')))))) {
        local_108 = (int *)local_108[3];
        goto LAB_0045aa20;
      }
      if ((char)local_108[0x92] == '\x01') {
        local_108 = (int *)local_108[3];
        goto LAB_0045aa20;
      }
      if (((DAT_005d0404 == 0) && ((uint)local_108[8] < 1000)) &&
         ((*(char *)((int)local_108 + 0x79b) == '\0' &&
          (((short)DAT_013b71e8[0x3c4] != (short)local_108[0x1a0] && (DAT_005d03f8 == 0)))))) {
        local_108 = (int *)local_108[3];
        goto LAB_0045aa20;
      }
    }
    if (((((uint)local_108[8] < 1000) || ((char)local_108[0x92] != '\0')) ||
        (*(char *)((int)local_108 + 0x249) != '\0')) || (iVar4 = FUN_00516105(), iVar4 == 0)) {
      if (*(char *)((int)local_108 + 0x245) == '\x01') {
        local_108 = (int *)local_108[3];
      }
      else {
        iVar4 = (**(code **)(*local_108 + 0x54))();
        if (((iVar4 == 1) && (iVar4 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar4 == 0)) &&
           (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)) {
          local_108 = (int *)local_108[3];
        }
        else {
          if ((((local_13c <= local_138) && (local_120 == local_11c)) &&
              (((local_128 == local_124 && (local_108 != (int *)param_1[0x13])) &&
               ((iVar4 = (**(code **)(*local_108 + 0x50))(), iVar4 == 0 ||
                (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)))))) &&
             (((999 < (uint)local_108[8] &&
               (((char)local_108[0x92] == '\0' && (*(char *)((int)local_108 + 0x249) == '\0')))) ||
              ((((uint)local_108[8] < 1000 && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)) ||
               (iVar4 = (**(code **)(*local_108 + 0x54))(), iVar4 != 0)))))) {
            if ((*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) && (local_104 == 0)) {
                    /* WARNING: Ignoring partial resolution of indirect */
              local_74._0_2_ = (short)local_108[8];
              local_92 = DAT_005d0400;
              local_104 = 1;
            }
            *(short *)(&local_74 + local_104) = (short)local_108[8];
            *(undefined2 *)((int)&local_74 + local_104 * 4 + 2) = 0xffff;
            if ((((local_20 != (int *)0x0) && (local_20 == local_108)) && (local_104 != 0)) &&
               (((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3 ||
                 (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 4)) ||
                (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 6)))) {
              uVar2 = (undefined2)local_74;
              local_1a0 = (undefined2)local_74;
              sVar3 = local_74._2_2_;
              local_1a4 = (int)local_74._2_2_;
              local_74 = CONCAT22(local_74._2_2_,*(undefined2 *)(&local_74 + local_104));
              local_74 = CONCAT22(*(undefined2 *)((int)&local_74 + local_104 * 4 + 2),
                                  *(undefined2 *)(&local_74 + local_104));
              *(undefined2 *)(&local_74 + local_104) = uVar2;
              *(short *)((int)&local_74 + local_104 * 4 + 2) = sVar3;
            }
            local_104 = local_104 + 1;
          }
          local_108 = (int *)local_108[3];
          if (((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3) && (7 < local_104)) ||
             ((*(int *)(&DAT_0092ff44 + local_18 * 0x60) <= local_104 || (0xc < local_104))))
          goto LAB_0045bff9;
        }
      }
    }
    else {
      local_108 = (int *)local_108[3];
    }
    goto LAB_0045aa20;
  }
  if ((local_18 == '\x1d') || (local_18 == ',')) {
    if (*(char *)(param_1[0x13] + 0x248) == '\x01') {
      local_218 = 0;
      local_214 = 0;
      _memset(local_204,0,0x60);
      local_224 = param_1[0xa1cb];
      for (local_210 = 0; local_210 < *(int *)(local_224 + 0x210); local_210 = local_210 + 1) {
        local_22c = *(int *)(local_224 + 0x214 + local_210 * 4);
        local_228 = (int *)(**(code **)(*DAT_013b71e8 + 0x34))(*(undefined4 *)(local_22c + 0xe50));
        if ((local_228 != (int *)0x0) &&
           ((iVar4 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar4 != 0 ||
            (iVar4 = (**(code **)(*local_228 + 0x54))(), iVar4 != 1)))) {
          local_230 = __ftol();
          local_234 = __ftol();
          if (param_1[0xa1e5] != 0) {
            local_230 = param_1[0xa1e5];
            local_234 = param_1[0xa1e6];
          }
          uVar6 = __ftol();
          uVar6 = __ftol(uVar6);
          local_238 = FUN_005563a0(local_230,local_234,uVar6);
          if (local_238 < *(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c) {
            local_204[local_218 * 2] = *(int *)(local_22c + 0xe50);
            local_204[local_218 * 2 + 1] = local_238;
            local_218 = local_218 + 1;
          }
        }
      }
      local_214 = local_218;
      if (0xb < local_218) {
        local_214 = 0xc;
      }
      local_20c = 0;
      for (local_218 = 0; local_218 < local_214; local_218 = local_218 + 1) {
        local_20c = local_218;
        local_220 = local_204[local_218 * 2];
        local_21c = local_204[local_218 * 2 + 1];
        local_23c = local_218;
        while (local_23c = local_23c + 1, local_23c < local_214) {
          if (local_204[local_23c * 2 + 1] < local_21c) {
            local_220 = local_204[local_23c * 2];
            local_21c = local_204[local_23c * 2 + 1];
            local_20c = local_23c;
          }
        }
        iVar4 = local_204[local_218 * 2 + 1];
        local_204[local_20c * 2] = local_204[local_218 * 2];
        local_204[local_20c * 2 + 1] = iVar4;
        local_204[local_218 * 2] = local_220;
        local_204[local_218 * 2 + 1] = local_21c;
      }
      local_218 = 0;
      for (local_208 = 0;
          ((local_218 < local_214 && (local_208 < *(int *)(&DAT_0092ff44 + local_18 * 0x60))) &&
          (local_208 < 0xd)); local_208 = local_208 + 1) {
        *(short *)(&local_74 + local_208) = (short)local_204[local_218 * 2];
        *(undefined2 *)((int)&local_74 + local_208 * 4 + 2) = 0xffff;
        local_218 = local_218 + 1;
      }
    }
    else {
      local_74 = CONCAT22(0xffff,*(undefined2 *)(param_1[0x13] + 0x20));
    }
  }
  else {
    if (local_18 == '*') {
      if (*(char *)(param_1[0x13] + 0x248) == '\x01') {
        FUN_00403df2(&DAT_00a350f8,1000);
      }
      else {
        FUN_00403df2(&DAT_00a35178,1000);
      }
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    if (local_18 == 'I') {
      if (*(short *)((int)DAT_013b71e8 + 0x73a) < DAT_00931a60) {
        ExceptionList = local_10;
        return 1;
      }
      if (1000 < param_7 - *(int *)(param_1[0x13] + 0x5ec)) {
        local_2a4 = __ftol();
        local_2a8 = __ftol();
        local_2ac = __ftol();
        local_2b0 = __ftol();
        _memset(local_26c,0,0x30);
        FUN_00555ad8(local_2ac,local_2b0,&local_2a4,&local_2a8,local_26c,8,param_1 + 0x25,8);
        sVar9 = _strlen(local_26c);
        if (sVar9 == 0) {
          ExceptionList = local_10;
          return 1;
        }
        _memset(local_2a0,0,0x34);
        local_29a = *(undefined2 *)(param_1[0x13] + 0x20);
        local_294 = (short)local_2ac;
        local_292 = (short)local_2b0;
        local_28c = 6;
        local_29c = 0x368;
        local_290 = DAT_005bb418;
        local_288 = (undefined2)local_2a4;
        uStack_286 = (undefined2)local_2a8;
        param_1[0xa1e3] = (int)(short)local_2ac;
        param_1[0xa1e4] = (int)(short)local_2b0;
        param_1[0xa1e5] = local_2a4 & 0xffff;
        param_1[0xa1e6] = local_2a8 & 0xffff;
        FUN_0055f2dd(local_2a0,0x34);
        FUN_0044dc1f();
        *(uint *)(&DAT_0092eaf8 + local_18 * 4) = param_7;
        *(uint *)(param_1[0x13] + 0x5ec) = param_7;
      }
      ExceptionList = local_10;
      return 1;
    }
    local_74 = CONCAT22(0xffff,*(undefined2 *)(param_1[0x13] + 0x20));
    local_8c = __ftol();
    uStack_8a = __ftol();
    if (param_1[0xa1e5] != 0) {
      local_8c = *(ushort *)(param_1 + 0xa1e5);
      uStack_8a = *(ushort *)(param_1 + 0xa1e6);
    }
    if (((((local_18 == '8') || (local_18 == '9')) ||
         ((local_18 == ':' || ((local_18 == ';' || (local_18 == '<')))))) || (local_18 == '=')) ||
       ((local_18 == '>' || (local_18 == '?')))) {
      *(uint *)(&DAT_0092eaf8 + local_18 * 4) = param_7;
      *(uint *)(param_1[0x13] + 0x5ec) = param_7;
    }
  }
LAB_0045c794:
  if (local_18 == 'b') {
    local_2bc = __ftol();
    local_2c0 = __ftol();
    local_2c4 = __ftol();
    local_2d0 = __ftol();
    if (param_1[0xa1e5] != 0) {
      local_2c4 = param_1[0xa1e5];
      local_2d0 = param_1[0xa1e6];
    }
    local_2cc = FUN_005563a0(local_2c4,local_2d0,local_2bc,local_2c0);
    local_2c8 = *(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c;
    local_2b4 = local_2bc;
    local_2b8 = local_2c0;
    FUN_00556416(local_2c4,local_2d0,&local_2b4,&local_2b8,param_1 + 0x25,8);
    if (local_2c8 == -1) {
      local_2c8 = 1;
    }
    iVar4 = FUN_0054cd07(local_28 + 0x23,0x15);
    if (iVar4 < 100) {
      local_2d4 = FUN_0054eb20(local_28,0x1b);
      local_2d4 = local_2d4 + local_1c;
      if (local_2c8 < local_2d4) {
        local_2c8 = local_2d4;
      }
    }
    if (((local_2c8 < local_2cc) || (local_2b4 != local_2bc)) || (local_2b8 != local_2c0)) {
      ExceptionList = local_10;
      return 1;
    }
    uVar6 = __ftol();
    uVar6 = __ftol(uVar6);
    local_2dc = FUN_005563a0(local_2c4,local_2d0,uVar6);
    if (*(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c < local_2dc) {
      ExceptionList = local_10;
      return 1;
    }
    local_74 = CONCAT22(0xffff,*(undefined2 *)(param_1[0x13] + 0x20));
    local_8c = __ftol();
    uStack_8a = __ftol();
    bVar1 = (&DAT_00934178)[((int)(uint)local_8c >> 2) + ((int)(uint)uStack_8a >> 2) * 0x400];
    _local_2d8 = CONCAT31(uStack_2d7,bVar1);
    if (((bVar1 & 1) != 0) && ((bVar1 & 0x40) == 0)) {
      ExceptionList = local_10;
      return 1;
    }
  }
  if (local_18 == 'f') {
    local_2e0 = (int)*(short *)(param_1[0x13] + 0x250) % 10;
    iVar4 = FUN_00434315(0x1f);
    if (((iVar4 == 0) && (iVar4 = FUN_00434315(0x27), iVar4 == 0)) &&
       (iVar4 = FUN_00434315(0x2f), iVar4 == 0)) {
      FUN_00403df2(&DAT_00a3f3f8,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
  }
  if (((((local_74 & 0xffff) != 0) || (local_18 == 'a')) || (local_18 == '#')) || (local_18 == '3'))
  {
    local_2e8 = *(int *)(&DAT_0092ff44 + local_18 * 0x60);
    for (local_34c = local_2e8; local_34c < 0xd; local_34c = local_34c + 1) {
      *(undefined2 *)(&local_74 + local_34c) = 0;
      *(undefined2 *)((int)&local_74 + local_34c * 4 + 2) = 0;
    }
    local_2e4 = 0x60;
    if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 1) {
      local_9c = CONCAT22(local_9c._2_2_,0x39d);
      local_2e4 = 0x30;
    }
    if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 2) {
      local_9c = CONCAT22(local_9c._2_2_,0x39e);
      local_2e4 = 0x34;
    }
    FUN_0055f2dd(local_a0,local_2e4);
    FUN_0044dc1f();
    _memset(local_348,0,0x60);
    FUN_0058f220(local_348,local_a0,local_2e4);
    local_342 = (undefined2)param_1[8];
    local_32a = 1;
    if (local_1c != 0) {
      local_329 = local_329 | 8;
    }
    (**(code **)(*param_1 + 4))(local_9c & 0xffff,local_348);
    param_1[0x9d18] = param_7;
    *(uint *)(&DAT_0092eaf8 + local_18 * 4) = param_7;
  }
  if (local_18 != 'U') {
    ExceptionList = local_10;
    return 1;
  }
LAB_0045cc76:
  if (local_18 == 'U') {
    iVar4 = FUN_0040c0f0();
    if (iVar4 == 0) {
      local_354 = (uint)*(short *)(param_1[0x13] + 0x444);
      local_350 = (int)local_18 + (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18;
      if (local_18 < '`') {
        local_354 = (uint)*(byte *)((int)DAT_013b71e8 +
                                   ((int)(local_350 + (local_350 >> 0x1f & 7U)) >> 3) + 0x745);
      }
      iVar4 = FUN_00555790((int)local_18,*(undefined1 *)((int)DAT_013b71e8 + 0x9d3),local_354);
      if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar4) {
        local_358 = param_1[0x9eba];
        pvVar7 = operator_new(0xe50);
        local_8 = 2;
        if (pvVar7 == (void *)0x0) {
          local_50c = 0;
        }
        else {
          local_50c = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1
                                   ,0);
        }
        local_8 = 0xffffffff;
        local_35c = local_50c;
        if (local_50c != 0) {
          FUN_00408d33(local_50c);
        }
        if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x21), iVar4 != 0)) {
          uVar12 = 0;
          uVar6 = 0;
          FUN_00429a6d(0x21);
          FUN_0042ad2b(uVar6,uVar12);
        }
        ExceptionList = local_10;
        return 1;
      }
      FUN_004484f3(1);
    }
    ExceptionList = local_10;
    return 1;
  }
  if ((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 2) && (local_20 == (int *)0x0)) {
    local_20 = (int *)param_1[0x13];
    local_20[0x8e] = 1;
  }
  if (((local_20 == (int *)0x0) ||
      (((local_20[0x8e] != 1 && (param_9 == (int *)0x0)) || (*(int *)(param_1[0x13] + 0x740) == 2)))
      ) || (*(int *)(param_1[0x13] + 0x740) == 3)) {
    param_1[0x1f] = 0;
  }
  else {
    if (*(char *)(param_1[0x13] + 0x795) == '\x01') {
      ExceptionList = local_10;
      return 1;
    }
    if ((uint)*(ushort *)((int)param_1 + 0x275ae) == *(uint *)(param_1[0x13] + 0x20)) {
      ExceptionList = local_10;
      return 1;
    }
    local_364 = (uint)*(short *)(param_1[0x13] + 0x444);
    local_360 = (int)local_18 + (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18;
    if (local_18 < '`') {
      local_364 = (uint)*(byte *)((int)DAT_013b71e8 +
                                 ((int)(local_360 + (local_360 >> 0x1f & 7U)) >> 3) + 0x745);
    }
    iVar4 = FUN_00555790((int)local_18,*(undefined1 *)((int)DAT_013b71e8 + 0x9d3),local_364);
    if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar4) {
      local_368 = param_1[0x9eba];
      pvVar7 = operator_new(0xe50);
      local_8 = 3;
      if (pvVar7 == (void *)0x0) {
        local_510 = 0;
      }
      else {
        local_510 = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0
                                );
      }
      local_8 = 0xffffffff;
      local_36c = local_510;
      if (local_510 != 0) {
        FUN_00408d33(local_510);
      }
      if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x21), iVar4 != 0)) {
        uVar12 = 0;
        uVar6 = 0;
        FUN_00429a6d(0x21);
        FUN_0042ad2b(uVar6,uVar12);
      }
      ExceptionList = local_10;
      return 1;
    }
    iVar4 = (**(code **)(*(int *)param_1[0x13] + 0x50))();
    if (((iVar4 == 1) || (iVar4 = (**(code **)(*local_20 + 0x50))(), iVar4 == 1)) &&
       (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
      ExceptionList = local_10;
      return 1;
    }
    if (((((uint)local_20[8] < 1000) || ((char)local_20[0x92] == '\x01')) &&
        (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
       (iVar4 = (**(code **)(*local_20 + 0x54))(), iVar4 == 0)) {
      ExceptionList = local_10;
      return 1;
    }
    if ((((uint)local_20[8] < 1000) || ((char)local_20[0x92] == '\x01')) &&
       ((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1 &&
        (iVar4 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar4 == 0)))) {
      ExceptionList = local_10;
      return 1;
    }
    iVar4 = (**(code **)(*local_20 + 0x54))();
    if (((iVar4 == 1) && (iVar4 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar4 == 0)) &&
       (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)) {
      ExceptionList = local_10;
      return 1;
    }
    iVar4 = (**(code **)(*local_20 + 0x54))();
    if (((iVar4 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
       ((*(char *)((int)local_20 + 0x249) == '\x01' || ((char)local_20[0x92] == '\x01')))) {
      ExceptionList = local_10;
      return 1;
    }
    if ((((DAT_005d0404 == 0) && (*(char *)((int)local_20 + 0x249) == '\x01')) &&
        ((short)DAT_013b71e8[0x3c4] != (short)local_20[0x1a0])) && ((short)local_20[0x1a0] != 0)) {
      if (DAT_005d03f8 == 0) {
        ExceptionList = local_10;
        return 1;
      }
      if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
          (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_20 + 0x79b))) &&
         (iVar4 = (**(code **)(*local_20 + 0x58))(), iVar4 == 0)) {
        ExceptionList = local_10;
        return 1;
      }
    }
    if (((DAT_005d0404 == 0) && ((short)local_20[0x1a0] != 0)) &&
       (((short)DAT_013b71e8[0x3c4] != (short)local_20[0x1a0] &&
        (((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1 && ((uint)local_20[8] < 1000)) &&
         (iVar4 = (**(code **)(*local_20 + 0x54))(), iVar4 == 1)))))) {
      if (DAT_005d03f8 == 0) {
        ExceptionList = local_10;
        return 1;
      }
      if (('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
         (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_20 + 0x79b))) {
        ExceptionList = local_10;
        return 1;
      }
    }
    if (((((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
         (*(char *)(param_1[0x13] + 0x79b) == '\x03')) &&
        (((uint)local_20[8] < 1000 ||
         (('\0' < *(char *)((int)local_20 + 0x79b) && (*(char *)((int)local_20 + 0x79b) != '\x04')))
         ))) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
      ExceptionList = local_10;
      return 1;
    }
    if ((((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0) && (999 < (uint)local_20[8])) &&
        ((char)local_20[0x92] == '\0')) && (*(char *)((int)local_20 + 0x249) == '\0')) {
      ExceptionList = local_10;
      return 1;
    }
    if (((local_20 != (int *)0x0) && ((short)local_20[0x1a0] != 0)) &&
       ((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1 &&
        ((*(short *)(param_1[0x13] + 0x680) == (short)local_20[0x1a0] ||
         (*(short *)((int)DAT_013b71e8 + 0xf12) == (short)local_20[0x1a0])))))) {
      ExceptionList = local_10;
      return 1;
    }
    if ((((local_20 != (int *)0x0) && (DAT_005d0404 == 0)) &&
        (*(char *)((int)local_20 + 0x249) == '\x01')) && ((short)local_20[0x1a0] == 0)) {
      if (DAT_005d03f8 == 0) {
        ExceptionList = local_10;
        return 1;
      }
      if (('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
         (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_20 + 0x79b))) {
        ExceptionList = local_10;
        return 1;
      }
    }
    if (((((uint)local_20[8] < 1000) || ((char)local_20[0x92] != '\0')) ||
        ((*(char *)((int)local_20 + 0x249) != '\0' || (iVar4 = FUN_00516105(), iVar4 == 0)))) &&
       (((char)local_20[0x1c9] == '\0' && (param_1[0x9d18] + 1000U < param_7)))) {
      if (((*(byte *)((int)local_20 + 0x44a) & 0xf) == 0xf) &&
         (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
        if (*(char *)(param_1[0x13] + 0x79b) == '\0') {
          ExceptionList = local_10;
          return 1;
        }
        if ((('\0' < *(char *)((int)local_20 + 0x79b)) && ('\0' < *(char *)(param_1[0x13] + 0x79b)))
           && (*(char *)((int)local_20 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) {
          ExceptionList = local_10;
          return 1;
        }
      }
      local_378 = __ftol();
      local_37c = __ftol();
      local_380 = __ftol();
      local_38c = __ftol();
      if (param_1[0xa1e5] != 0) {
        local_380 = param_1[0xa1e5];
        local_38c = param_1[0xa1e6];
      }
      local_388 = FUN_005563a0(local_380,local_38c,local_378,local_37c);
      local_384 = *(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_1c;
      local_370 = local_378;
      local_374 = local_37c;
      FUN_00556416(local_380,local_38c,&local_370,&local_374,param_1 + 0x25,8);
      if (local_384 == -1) {
        local_384 = 1;
      }
      if (((local_20 != (int *)0x0) && (local_20[0x93] == 0x38)) &&
         (*(short *)((int)local_20 + 0x1d2) == 0)) {
        local_388 = local_388 + -0xc;
        if (local_388 < 0) {
          local_388 = 0;
        }
        local_370 = __ftol();
        local_374 = __ftol();
      }
      iVar4 = FUN_0054cd07(local_28 + 0x23,0x15);
      if (iVar4 < 100) {
        local_390 = FUN_0054eb20(local_28,0x1b);
        local_390 = local_390 + local_1c;
        if (local_384 < local_390) {
          local_384 = local_390;
        }
      }
      if (((local_388 <= local_384) && (local_370 == local_378)) && (local_374 == local_37c)) {
        if ((*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) &&
           (iVar4 = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b), iVar4 + local_1c < local_388)) {
          ExceptionList = local_10;
          return 1;
        }
        if (((DAT_005d0404 == 0) &&
            (((*(char *)(param_1[0x13] + 0x79b) == '\0' && ((uint)local_20[8] < 1000)) &&
             (iVar4 = (**(code **)(*local_20 + 0x54))(), iVar4 == 1)))) &&
           (((short)DAT_013b71e8[0x3c4] != (short)local_20[0x1a0] &&
            (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))) {
          ExceptionList = local_10;
          return 1;
        }
        if ((DAT_005d0404 == 0) &&
           (((0 < DAT_005d03f8 && ('\0' < *(char *)(param_1[0x13] + 0x79b))) &&
            (('\0' < *(char *)((int)local_20 + 0x79b) &&
             ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_20 + 0x79b) &&
              (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))))))) {
          ExceptionList = local_10;
          return 1;
        }
        if ((DAT_005d0404 == 0) &&
           ((((short)DAT_013b71e8[0x3c4] != (short)local_20[0x1a0] && ((uint)local_20[8] < 1000)) &&
            (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))) {
          if (DAT_005d03f8 == 0) {
            ExceptionList = local_10;
            return 1;
          }
          if (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_20 + 0x79b)) {
            ExceptionList = local_10;
            return 1;
          }
        }
        if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
           (((iVar4 = (**(code **)(*local_20 + 0x58))(), iVar4 == 0 &&
             (((uint)local_20[8] < 1000 || (*(char *)((int)local_20 + 0x249) == '\x01')))) &&
            (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))) {
          ExceptionList = local_10;
          return 1;
        }
        _memset(local_45c,0,0x60);
        local_458 = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x36c);
        local_450 = *(undefined2 *)(param_1[0x13] + 0x20);
        local_44c = (undefined2)param_1[0xa1e5];
        local_44a = (undefined2)param_1[0xa1e6];
        local_448 = __ftol();
        local_446 = __ftol();
        local_442 = 0xffff;
        local_444 = (short)local_18;
        local_43f = 0;
        local_440 = 0xff;
        local_43e = 0;
        local_430[0] = (short)local_20[8];
        local_39c = (int *)(**(code **)(*DAT_013b71e8 + 0x34))((short)local_20[8]);
        if (local_39c == (int *)0x0) {
          ExceptionList = local_10;
          return 1;
        }
        if (*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) {
                    /* WARNING: Ignoring partial resolution of indirect */
          local_430[0] = (short)local_39c[8];
          local_44e = DAT_005d0400;
        }
        local_398 = (uint)(*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1);
        local_430[local_398 * 2] = (short)local_39c[8];
        local_430[local_398 * 2 + 1] = 0xffff;
        local_398 = local_398 + 1;
        if (((local_18 == '\x10') || (local_18 == '\f')) || (local_18 == '\x1c')) {
          local_468 = __ftol();
          iVar4 = __ftol();
          local_468 = local_468 - iVar4;
          local_46c = __ftol();
          iVar4 = __ftol();
          local_46c = local_46c - iVar4;
          if (local_468 < 1) {
            if (local_468 < 0) {
              local_468 = -1;
            }
          }
          else {
            local_468 = 1;
          }
          if (local_46c < 1) {
            if (local_46c < 0) {
              local_46c = -1;
            }
          }
          else {
            local_46c = 1;
          }
          iVar4 = __ftol();
          iVar5 = __ftol();
          local_470 = *(int **)(param_1[0xd] + 0x10);
          while (local_470[3] != 0) {
            if (((local_470 == (int *)param_1[0x13]) || (local_470 == local_39c)) ||
               ((iVar10 = __ftol(), iVar10 != iVar4 + local_468 ||
                (iVar10 = __ftol(), iVar10 != iVar5 + local_46c)))) {
              local_470 = (int *)local_470[3];
            }
            else if ((uint)local_470[8] < 1000) {
              iVar10 = (**(code **)(*local_470 + 0x54))();
              if (((iVar10 == 0) || (DAT_005d0404 == 0)) || ((char)local_470[0x92] == '\x01')) {
                local_470 = (int *)local_470[3];
              }
              else if (((DAT_005d0404 == 0) &&
                       ((short)DAT_013b71e8[0x3c4] != (short)local_470[0x1a0])) &&
                      (DAT_005d03f8 == 0)) {
                local_470 = (int *)local_470[3];
              }
              else if (((DAT_005d0404 == 0) && ('\0' < *(char *)(param_1[0x13] + 0x79b))) &&
                      ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_470 + 0x79b) &&
                       (0 < DAT_005d03f8)))) {
                local_470 = (int *)local_470[3];
              }
              else if ((((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
                       (*(char *)(param_1[0x13] + 0x79b) == '\x03')) &&
                      (((uint)local_470[8] < 1000 ||
                       (('\0' < *(char *)((int)local_470 + 0x79b) &&
                        (*(char *)((int)local_470 + 0x79b) != '\x04')))))) {
                local_470 = (int *)local_470[3];
              }
              else {
                if (((DAT_005d0404 != 0) || (DAT_005d03f8 < 1)) ||
                   (iVar10 = (**(code **)(*local_470 + 0x58))(), iVar10 != 0)) {
LAB_0045e1a5:
                  local_430[local_398 * 2] = (short)local_470[8];
                  local_430[local_398 * 2 + 1] = 0xffff;
                  local_398 = local_398 + 1;
                  break;
                }
                local_470 = (int *)local_470[3];
              }
            }
            else if (((DAT_005d0404 == 0) && (*(char *)((int)local_470 + 0x249) == '\x01')) &&
                    ((((short)DAT_013b71e8[0x3c4] != (short)local_470[0x1a0] &&
                      ((short)local_470[0x1a0] != 0)) || ((short)local_470[0x1a0] == 0)))) {
              iVar10 = (**(code **)(*local_470 + 0x58))();
              if (((iVar10 == 0) && (0 < DAT_005d03f8)) || (DAT_005d03f8 == 0)) {
                local_470 = (int *)local_470[3];
              }
            }
            else {
              if (((local_20 == (int *)0x0) || (DAT_005d0404 != 0)) ||
                 ((*(char *)((int)local_470 + 0x249) != '\x01' || ((short)local_470[0x1a0] != 0))))
              goto LAB_0045e1a5;
              iVar10 = (**(code **)(*local_470 + 0x58))();
              if (((iVar10 == 0) && (0 < DAT_005d03f8)) || (DAT_005d03f8 == 0)) {
                local_470 = (int *)local_470[3];
              }
            }
          }
        }
        local_448 = __ftol();
        local_446 = __ftol();
        if (local_18 != '\0') {
          local_44c = (undefined2)param_1[0xa1e5];
          local_44a = (undefined2)param_1[0xa1e6];
          local_448 = (undefined2)param_1[0xa1e5];
          local_446 = (undefined2)param_1[0xa1e6];
        }
        local_394 = 0x60;
        if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 1) {
          local_458 = CONCAT22(local_458._2_2_,0x39d);
          local_394 = 0x30;
        }
        if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 2) {
          local_458 = CONCAT22(local_458._2_2_,0x39e);
          local_394 = 0x34;
        }
        FUN_0055f2dd(local_45c,local_394);
        FUN_0044dc1f();
        FUN_0058f220(local_3fc,local_45c,0x60);
        local_3f6 = (undefined2)param_1[8];
        local_3de = 1;
        if (local_1c != 0) {
          local_3dd = local_3dd | 8;
        }
        (**(code **)(*param_1 + 4))(local_458 & 0xffff,local_3fc);
        param_1[0x9d18] = param_7;
        *(uint *)(&DAT_0092eaf8 + local_18 * 4) = param_7;
        *(undefined4 *)(param_1[0x13] + 0x76c) = 0;
        ExceptionList = local_10;
        return 1;
      }
      if ((((param_8 == 1) && (local_384 <= local_388)) && (local_370 == local_378)) &&
         (local_374 == local_37c)) {
        if ((3 < *(int *)(param_1[0x13] + 0x740)) && (*(int *)(param_1[0x13] + 0x740) < 10)) {
          local_47c = *(uint *)(&DAT_0065a4a8 +
                               *(int *)(*(int *)(param_1[0x13] + 0x60) + 0x2d0) * 4 +
                               *(int *)(param_1[0x13] + 0x5c) * 900);
          if (2 < local_47c) {
            local_47c = local_47c - 2;
          }
          if ((*(int *)(DAT_005d0504 + 0x1c) != 0) &&
             (param_7 < *(int *)(param_1[0x13] + 0x600) +
                        *(int *)(*(int *)(param_1[0x13] + 0x60) + 0x2e4) * local_47c * 4)) {
            ExceptionList = local_10;
            return 1;
          }
        }
        if (param_1[0x10] != 0) {
          FUN_004fa8a1(param_1[0x9b9f]);
          iVar4 = __ftol();
          iVar5 = __ftol();
          if (iVar4 == iVar5) {
            iVar4 = __ftol();
            iVar5 = __ftol();
            if (iVar4 == iVar5) {
              ExceptionList = local_10;
              return 1;
            }
          }
          param_1[0x9ba6] = 0;
          *(int **)(param_1[0x13] + 0x76c) = local_20;
          param_1[0x9d35] = param_7;
          local_480 = __ftol();
          local_484 = __ftol();
          if (param_1[0xa1e5] != 0) {
            local_480 = param_1[0xa1e5];
            local_484 = param_1[0xa1e6];
          }
          if (local_20 != (int *)0x0) {
            local_378 = __ftol();
            local_37c = __ftol();
            local_488 = FUN_005563a0(local_480,local_484,local_378,local_37c);
            local_490 = 1;
            local_494 = 1;
            if (local_378 < local_480) {
              local_490 = -1;
            }
            if (local_37c < local_484) {
              local_494 = -1;
            }
            local_48c = 0;
            while (local_384 < local_488) {
              if (500 < local_48c) {
                ExceptionList = local_10;
                return 1;
              }
              if (local_480 != local_378) {
                local_480 = local_480 + local_490;
              }
              if (local_484 != local_37c) {
                local_484 = local_484 + local_494;
              }
              local_488 = FUN_005563a0(local_480,local_484,local_378,local_37c);
              local_48c = local_48c + 1;
            }
            param_1[0x9d6c] = local_480;
            param_1[0x9d6d] = local_484;
          }
        }
      }
    }
  }
  uVar6 = FUN_00455ea7(param_7);
  ExceptionList = local_10;
  return uVar6;
LAB_0045bff9:
  local_8c = (ushort)local_114;
  uStack_8a = (ushort)local_118;
  goto LAB_0045c794;
}

