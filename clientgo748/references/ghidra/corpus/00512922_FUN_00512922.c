// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00512922 | Name: FUN_00512922


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00512922(int *param_1)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  void *pvVar6;
  uint uVar7;
  undefined8 *puVar8;
  float10 extraout_ST0;
  float10 extraout_ST0_00;
  float10 fVar9;
  float10 fVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined8 uVar17;
  undefined *puVar18;
  undefined8 uVar19;
  float *pfVar20;
  undefined4 uVar21;
  float fVar22;
  undefined4 uVar23;
  int local_28c;
  int *local_284;
  int *local_280;
  int local_274;
  int *local_270;
  float local_230;
  int local_20c;
  int *local_208;
  undefined1 local_188 [12];
  undefined1 local_17c [8];
  undefined1 local_174 [20];
  void *local_160;
  int local_15c;
  void *local_158;
  int *local_154;
  float local_150;
  float local_14c;
  float local_148;
  int local_144;
  int local_140;
  int local_13c;
  uint local_138;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  int *local_118;
  int local_114;
  uint local_110;
  int local_10c;
  undefined4 local_108;
  int local_104;
  int *local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  uint local_f4;
  int local_f0;
  float local_ec;
  undefined4 local_e8;
  int *local_e4;
  undefined4 local_e0;
  float local_dc;
  undefined4 local_d8;
  int local_d4;
  int local_d0;
  int local_cc;
  undefined4 local_c8;
  int local_c4;
  int local_c0;
  int local_bc;
  uint local_b8;
  int local_b4;
  float local_b0;
  float local_ac;
  int local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  float local_98;
  float local_94;
  undefined4 local_90;
  int local_8c;
  int local_88;
  int *local_84;
  float local_80;
  float local_7c;
  int local_78;
  undefined4 local_74;
  undefined4 local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  int local_58;
  float local_54;
  uint local_50;
  float local_4c;
  int local_48;
  int *local_44;
  float local_40;
  int local_3c;
  uint local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a201a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (param_1[0x109] != 0) {
    ExceptionList = &local_10;
    (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
  }
  if ((DAT_005b892c == 2) && (param_1[0x108] != 0)) {
    (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
  }
  if ((param_1[0x85] != 0) && ((char)param_1[5] != '\x01')) {
    iVar3 = param_1[0x85];
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    if (uVar2 <= iVar3 + 10000U) {
      ExceptionList = local_10;
      return 1;
    }
    param_1[0x85] = 0;
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
    ExceptionList = local_10;
    return 1;
  }
  if ((char)param_1[5] == '\x01') {
    ExceptionList = local_10;
    return 1;
  }
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  if (param_1[500] + 100U < uVar2) {
    *(undefined1 *)(param_1 + 499) = 0;
  }
  if (((param_1[0x12] == 1) && ((**(code **)(*param_1 + 0x6c))(), param_1[0x182] != 0)) &&
     (uVar2 - param_1[0x182] < (uint)param_1[0x187])) {
    if (*(int *)(param_1[0x10b] + 0x28) == 0) {
      (**(code **)(*(int *)param_1[0x10b] + 0x60))(1);
    }
    if (*(int *)(param_1[0x106] + 0x28) == 0) {
      (**(code **)(*(int *)param_1[0x106] + 0x60))(1);
    }
  }
  if ((param_1[0x10c] != 0) && (local_58 = *(int *)(DAT_013b71e8[0x6c21] + 0x54), local_58 != 0)) {
    local_5c = *(float *)(local_58 + 0x28) - (float)param_1[10];
    local_60 = *(float *)(local_58 + 0x2c) - (float)param_1[0xb];
    iVar3 = __ftol();
    local_64 = (float)((float10)iVar3 * (float10)_DAT_005a3aa4 + extraout_ST0);
    iVar3 = __ftol();
    local_68 = (float)((float10)iVar3 * (float10)_DAT_005a3aa4 + extraout_ST0_00);
    (**(code **)(*(int *)param_1[0x10c] + 0x70))(local_64,local_68);
  }
  if (param_1[0x12] == 0) {
    (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
    if (param_1[0x1f9] != 0) {
      (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
    }
    (**(code **)(*(int *)param_1[0x10b] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
    if (param_1[0x95] == 0) {
      FUN_004f9b6f();
      ExceptionList = local_10;
      return 1;
    }
  }
  if ((param_1[0x182] != 0) && ((uint)param_1[0x187] < uVar2 - param_1[0x182])) {
    (**(code **)(*(int *)param_1[0x10b] + 0x60))(0);
    param_1[0x182] = 0;
    param_1[0x187] = 3000;
  }
  local_48 = DAT_0067cf38;
  local_40 = 0.0;
  local_3c = 10000;
  if ((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) {
    local_3c = 600000;
  }
  if (((param_1[0x17e] != 0) && ((uint)(param_1[0x17e] + local_3c) < uVar2)) &&
     ((param_1[0x95] == 1 || (param_1[0x95] == 2)))) {
    param_1[0x17e] = 0;
    FUN_004f9b6f();
    ExceptionList = local_10;
    return 1;
  }
  if ((param_1[0x17e] != 0) &&
     (((param_1[0x95] == 1 || (param_1[0x95] == 2)) && (*(char *)((int)param_1 + 0x245) == '\x01')))
     ) {
    local_6c = 1.0;
    if (param_1[0x93] == 0x2c) {
      local_6c = 4.0;
    }
    if ((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) {
      local_40 = 0.0;
    }
    else {
      local_40 = ((float)((param_1[0x17e] + 10000) - uVar2) / _DAT_005a420c - _DAT_005a3660) *
                 (float)param_1[0xe] * *(float *)(&DAT_0092e7d4 + param_1[0x17] * 8) * local_6c;
    }
  }
  if ((param_1[299] != 0) && (param_1[299] + 200U < uVar2)) {
    FUN_00522d56(param_1[300],param_1[0x12d],param_1[0x12e]);
    _memset(param_1 + 299,0,0x10);
    param_1[299] = 0;
  }
  if ((param_1[399] != 0) && (1000 < uVar2 - param_1[399])) {
    param_1[399] = 0;
    if (param_1[0x93] == 0x38) {
      uVar21 = 1;
      (**(code **)(*DAT_013b71e8 + 0x78))(1);
      FUN_004efb89(uVar21);
    }
    else if (param_1[0x93] == 0x20) {
      uVar21 = 1;
      (**(code **)(*DAT_013b71e8 + 0x78))(1);
      FUN_004efb89(uVar21);
      local_78 = 0;
      local_74 = 0;
      local_70 = 0x8e;
      fVar9 = (float10)FUN_00423b00((float)param_1[0xd] - _DAT_005a4148);
      fVar22 = (float)((float10)(float)param_1[0xb] - fVar9 * (float10)_DAT_005a430c);
      fVar9 = (float10)FUN_00493bf0((float)param_1[0xd] - _DAT_005a4148,fVar22);
      FUN_00430f20((float)(fVar9 * (float10)_DAT_005a430c + (float10)(float)param_1[10]),fVar22);
      iVar3 = FUN_0049cf38(local_80,local_7c,param_1[0xc],&local_74);
      if (iVar3 == 1) {
        local_78 = 0x97;
        local_70 = 10;
      }
      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(local_70), iVar3 != 0)) {
        FUN_00429a6d(local_70);
        iVar3 = FUN_0042afd0();
        if (iVar3 == 0) {
          uVar23 = 0;
          uVar21 = 0;
          FUN_00429a6d(local_70);
          FUN_0042ad2b(uVar21,uVar23);
        }
      }
      if (local_78 == 0) {
        local_158 = operator_new(0xc0);
        local_8 = 0;
        if (local_158 == (void *)0x0) {
          local_208 = (int *)0x0;
        }
        else {
          local_208 = (int *)FUN_004df250(2,0x75,0x3f800000);
        }
        local_154 = local_208;
        local_8 = 0xffffffff;
        local_84 = local_208;
        if (local_208 != (int *)0x0) {
          local_208[0x2d] = 0;
          local_208[0x28] = 5000;
          (**(code **)(*local_208 + 0x44))(0xaaaaaaaa);
          (**(code **)(*local_84 + 0x3c))(local_80,local_7c);
          FUN_0054ac09(local_84);
        }
      }
      if ((*(int *)(DAT_005ccf9c + 0x2a39c) == 0) && (*(int *)(DAT_005ccf9c + 0x2a3a0) == 0)) {
        for (local_88 = 0; local_88 < 3; local_88 = local_88 + 1) {
          local_160 = operator_new(0x180);
          local_8 = 1;
          if (local_160 == (void *)0x0) {
            local_20c = 0;
          }
          else {
            local_20c = FUN_004c8f70(local_78,2000,0x3f4ccccd,0x3f4ccccd,0x3f4ccccd,0x3a83126f,1,
                                     0x50);
          }
          local_15c = local_20c;
          local_8 = 0xffffffff;
          local_8c = local_20c;
          if (local_20c != 0) {
            *(undefined4 *)(local_20c + 0x100) = 1;
            iVar3 = _rand();
            fVar22 = (float)((iVar3 % 3 + -1) * local_88) * _DAT_005a4154 + local_7c;
            iVar3 = param_1[0xc];
            iVar5 = _rand();
            uVar21 = FUN_00430f50((float)((iVar5 % 3 + -1) * local_88) * _DAT_005a4154 + local_80,
                                  iVar3,fVar22);
            FUN_004310a0(uVar21);
            FUN_0054ac09(local_8c);
            param_1[0x18c] = uVar2;
          }
        }
      }
    }
  }
  if ((((param_1[0x17] == 0x14) && (*(short *)((int)param_1 + 0x1d6) == 0)) &&
      ((int)(((ulonglong)uVar2 % 5000) / 100) == 0)) &&
     ((local_90 = *(undefined4 *)(&DAT_00672574 + param_1[0x17] * 0x2a0), DAT_005ccf98 != 0 &&
      (iVar3 = FUN_00429a6d(local_90), iVar3 != 0)))) {
    FUN_00429a6d(local_90);
    iVar3 = FUN_0042afd0();
    if (iVar3 == 0) {
      uVar23 = 0;
      uVar21 = 0;
      FUN_00429a6d(local_90);
      FUN_0042ad2b(uVar21,uVar23);
    }
  }
  local_14 = 1000;
  if (_DAT_005a3534 < (float)param_1[0x88]) {
    local_14 = __ftol();
  }
  local_38 = uVar2 - param_1[0x17d];
  if (local_14 == 0) {
    FUN_00403df2(s_Crashed_Character_Unit_Time_005bb758,0);
    (**(code **)(**(int **)(local_48 + 0x84) + 0x88))(1,1);
    ExceptionList = local_10;
    return 1;
  }
  local_50 = local_38 / local_14;
  local_24 = (float)(local_38 - local_14 * local_50) / (float)(int)local_14;
  if ((int)local_50 < param_1[0x104]) {
    param_1[0x90] = 1;
  }
  else {
    local_50 = param_1[0x104];
    local_24 = 1.0;
    param_1[0x90] = 0;
  }
  if ((param_1[0x104] + 1 < 0x30) &&
     (((float)param_1[param_1[0x104] * 2 + 0xa4] != (float)param_1[param_1[0x104] * 2 + 0xa6] ||
      ((float)param_1[param_1[0x104] * 2 + 0xa5] != (float)param_1[param_1[0x104] * 2 + 0xa7])))) {
    param_1[0x104] = param_1[0x104] + 1;
  }
  if (local_50 != param_1[0x105]) {
    iVar3 = (int)(local_50 + 1) % 0x30;
    FUN_00520fa4(param_1[iVar3 * 2 + 0xa4],param_1[iVar3 * 2 + 0xa5]);
    param_1[0x105] = (int)local_50 % 0x30;
  }
  local_54 = (float)(uVar2 - param_1[0x17c]) * _DAT_005a43e8;
  if (_DAT_005a3660 < local_54) {
    local_54 = 1.0;
  }
  local_4c = (float)param_1[0x8b] - (float)param_1[0x89];
  if ((float)param_1[0xd] - (float)param_1[0x8b] <= _DAT_005a3534) {
    local_230 = -((float)param_1[0xd] - (float)param_1[0x8b]);
  }
  else {
    local_230 = (float)param_1[0xd] - (float)param_1[0x8b];
  }
  if (_DAT_005a5654 < local_230) {
    param_1[0xd] = (int)(local_54 * local_4c + (float)param_1[0x89]);
    (**(code **)(*param_1 + 0x48))(0,param_1[0xd],0);
  }
  param_1[0x8c] = (int)local_24;
  uVar21 = FUN_0052fb00(local_17c,local_24);
  pfVar20 = &local_34;
  FUN_0052fb00(local_174,_DAT_005a3660 - local_24);
  FUN_0040bd60(pfVar20,uVar21);
  if ((char)param_1[0x1dd] == '\x01') {
    local_24 = (float)(uVar2 - param_1[0x17d]) / _DAT_005a441c;
    local_34 = (float)param_1[0x11f] * local_24 + (_DAT_005a3660 - local_24) * (float)param_1[0x11d]
               + _DAT_005a34a0;
    local_30 = (float)param_1[0x120] * local_24 + (_DAT_005a3660 - local_24) * (float)param_1[0x11e]
               + _DAT_005a34a0;
  }
  if (_DAT_005a3660 < local_24) {
    *(undefined1 *)(param_1 + 0x1dd) = 0;
  }
  FUN_0040bd30(&local_34);
  FUN_00506f9d(uVar2);
  if ((local_48 != 0) && (*(int *)(local_48 + 0x40) != 0)) {
    iVar3 = FUN_0049c70f(param_1[local_50 * 2 + 0xa4],param_1[local_50 * 2 + 0xa5]);
    local_98 = (float)iVar3 * _DAT_005a4214;
    iVar3 = FUN_0049c70f(param_1[local_50 * 2 + 0xa6],param_1[local_50 * 2 + 0xa7]);
    local_94 = (float)iVar3 * _DAT_005a4214;
    if ((char)param_1[0x1dd] == '\x01') {
      iVar3 = FUN_0049c792(param_1[0x11d],param_1[0x11e]);
      local_98 = (float)iVar3 * _DAT_005a4214;
      iVar3 = FUN_0049c792(param_1[0x11f],param_1[0x120]);
      local_94 = (float)iVar3 * _DAT_005a4214;
    }
    param_1[0x8a] = (int)(local_94 * local_24 + (_DAT_005a3660 - local_24) * local_98);
    if (*(int *)(local_48 + 0x24) == 0x7531) {
      if (_DAT_005a5650 <= (float)param_1[0xb]) {
        param_1[0x8a] = 0x40833333;
      }
      else {
        param_1[0x8a] = 0x3dcccccd;
      }
    }
    if (999 < (uint)param_1[8]) {
      param_1[0x8a] = (int)(local_40 + (float)param_1[0x8a]);
    }
    if (param_1[0x17] == 0x14) {
      if ((*(short *)((int)param_1 + 0x1d6) == 0) && (param_1[0x1d0] == 3)) {
        param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a4678);
      }
      else if ((*(short *)((int)param_1 + 0x1d6) == 2) && (param_1[0x1d0] == 3)) {
        param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a3660);
      }
    }
    else if (param_1[0x17] == 0x18) {
      if ((param_1[0x1d0] == 3) || ((param_1[0x1d0] == 2 && ((char)param_1[0x92] == '\x01')))) {
        param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a4678);
      }
      else if ((param_1[0x1d0] == 0) || (param_1[0x1d0] == 1)) {
        if ((_DAT_005a365c < (float)param_1[0x88]) || ((char)param_1[0x92] == '\x01')) {
          param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a4678);
        }
      }
      else if (((3 < param_1[0x1d0]) && (param_1[0x1d0] < 10)) &&
              ((_DAT_005a365c < (float)param_1[0x88] || ((char)param_1[0x92] == '\x01')))) {
        param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a4428);
      }
    }
    else if (((param_1[0x17] == 0x28) ||
             (((char)param_1[0x1e7] != '\0' && (param_1[0x1e8] == 0x28)))) &&
            ((param_1[0x1d0] == 3 || (param_1[0x1d0] == 2)))) {
      if (DAT_005ccfa0 == 1) {
        param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a3660);
      }
      else {
        param_1[0x8a] = (int)((float)param_1[0x8a] + _DAT_005a365c);
      }
    }
  }
  if (param_1[0x222] == 0) {
    fVar9 = (float10)FUN_00527baf();
    param_1[0xc] = (int)(float)fVar9;
  }
  if (param_1[0x222] == 0) {
    (**(code **)(*param_1 + 0x44))(param_1[10],param_1[0xc],param_1[0xb]);
  }
  else {
    (**(code **)(*param_1 + 0x44))
              ((float)param_1[10] + (float)param_1[0x223],param_1[0xc],
               (float)param_1[0xb] + (float)param_1[0x224]);
  }
  local_44 = *(int **)(DAT_0067cf38 + 0x4c);
  if ((param_1 != local_44) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) {
    local_9c = DAT_0067cf38;
    local_a0 = *(int *)(DAT_0067cf38 + 0x2872c);
    if (local_a0 != 0) {
      for (local_a4 = 0; local_a4 < *(int *)(local_a0 + 0x210); local_a4 = local_a4 + 1) {
        local_a8 = *(int *)(local_a0 + 0x214 + local_a4 * 4);
        if (*(int *)(local_a8 + 0xe50) == param_1[8]) {
          FUN_00493c60(&local_b0,param_1 + 10);
          fVar10 = (float10)FUN_005910b0((double)local_b0,(double)local_ac);
          fVar9 = (float10)_DAT_005a43dc;
          iVar3 = (**(code **)(**(int **)(local_a8 + 0xe64) + 0x84))();
          *(float *)(iVar3 + 0x18) = (float)(fVar10 + fVar9);
          break;
        }
      }
    }
  }
  local_20 = (float)param_1[local_50 * 2 + 0xa4];
  local_1c = (float)param_1[local_50 * 2 + 0xa5];
  local_2c = (float)param_1[local_50 * 2 + 0xa6];
  local_28 = (float)param_1[local_50 * 2 + 0xa7];
  if ((char)param_1[0x1dd] != '\x02') {
    iVar3 = __ftol();
    local_20 = (float)iVar3;
    iVar3 = __ftol();
    local_1c = (float)iVar3;
    iVar3 = __ftol();
    local_2c = (float)iVar3;
    iVar3 = __ftol();
    local_28 = (float)iVar3;
  }
  if ((local_20 == local_2c) && (local_1c == local_28)) {
    if (((short)param_1[0x114] < 1) || ((param_1[0x1d0] != 2 && (param_1[0x1d0] != 3)))) {
      if (((short)param_1[0x114] < 1) && ((param_1[0x1d0] != 0xb && (param_1[0x95] == 4)))) {
        FUN_0051dba4(0xc,1);
      }
      else if (((short)param_1[0x114] < 1) &&
              (((param_1[0x1d0] != 0xb && (param_1[0x1d0] != 0xc)) && (param_1[0x95] == 1)))) {
        FUN_0051dba4(0xb,0);
      }
      else if ((param_1[0x1d0] == 0) || (param_1[0x1d0] == 1)) {
        if ((*(char *)((int)param_1 + 0x246) == '\x01') &&
           ((param_1[0x8f] == 1 && (local_c8 = 0, _DAT_005a3794 < (float)param_1[0x88])))) {
          for (local_cc = 0; local_cc < 0x30; local_cc = local_cc + 1) {
            FUN_0040bd30();
          }
        }
        param_1[0x1da] = 0;
        param_1[0x1db] = 0;
        if (*(char *)((int)param_1 + 0x246) == '\x01') {
          FUN_004fa8a1();
        }
        *(undefined1 *)((int)param_1 + 0x246) = 0;
        param_1[0x8f] = 0;
      }
    }
    else {
      FUN_0051dba4(1,1);
      uVar7 = local_50;
      if ((float)param_1[0x88] <= _DAT_005a3670) {
        while (local_b8 = uVar7 + 1, (int)local_b8 < 0x30) {
          FUN_0040bd30();
          uVar7 = local_b8;
        }
      }
      else {
        for (local_b4 = 0; local_b4 < 0x30; local_b4 = local_b4 + 1) {
          FUN_0040bd30();
        }
      }
      *(undefined1 *)((int)param_1 + 0x246) = 0;
      FUN_004fa8a1();
      if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (param_1[0x1da] != 0)) {
        local_bc = DAT_0067cf38;
        if ((*(int *)(DAT_0067cf38 + 0x24) == 30000) &&
           (local_c0 = DAT_0067cf38, *(char *)(DAT_0067cf38 + 0x26eb4) == '\x01')) {
          *(int *)(DAT_0067cf38 + 0x26e90) = param_1[0x1da];
        }
        FUN_0051a939();
        param_1[0x1da] = 0;
        ExceptionList = local_10;
        return 1;
      }
      if ((param_1 == *(int **)(DAT_0067cf38 + 0x4c)) && (param_1[0x1db] != 0)) {
        if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
          local_c4 = local_48;
          uVar17 = CONCAT44(param_1[0x1db],1);
          puVar4 = (undefined4 *)FUN_0049bcb8(local_188);
          uVar21 = __ftol(*puVar4,puVar4[1],puVar4[2],uVar2,uVar17);
          uVar21 = __ftol(uVar21);
          FUN_004595ec(uVar21);
        }
        param_1[0x1db] = 0;
        ExceptionList = local_10;
        return 1;
      }
    }
  }
  else if ((((param_1[0x1d0] != 2) || (param_1[0x1d0] != 3)) ||
           ((param_1[0x1d0] < 4 && (9 < param_1[0x1d0])))) && (param_1[0x8f] == 0)) {
    local_d0 = 2;
    if (((param_1[0x17] == 0x1f) && (_DAT_005a4160 < (float)param_1[0xe])) ||
       (((char)param_1[0x1e7] == '\x01' &&
        ((param_1[0x1e8] == 0x1f && (_DAT_005a4160 < (float)param_1[0x1e9])))))) {
      local_d0 = 3;
    }
    if (((param_1[0x1e8] == 0x28) || (param_1[0x1e8] == 0x14)) || (param_1[0x1e8] == 0x27)) {
      local_d0 = 3;
    }
    if ((float)local_d0 < (float)param_1[0x88]) {
      FUN_0051dba4(3,1);
    }
    else {
      FUN_0051dba4(2,1);
    }
  }
  if ((((param_1[0x1d0] == -1) || (param_1[0x1d0] == 0)) || (param_1[0x1d0] == 1)) &&
     (param_1[0x95] == 0)) {
    FUN_004f9b6f();
    ExceptionList = local_10;
    return 1;
  }
  local_18 = FUN_0049c3c6(param_1[10],param_1[0xb]);
  if (*(int *)(local_48 + 0x24) == 30000) {
    if (local_18 == 8) {
      local_18 = 8;
    }
    else if (local_18 != 0) {
      iVar3 = __ftol();
      FUN_005399cb(param_1[10],param_1[0xb]);
      iVar5 = __ftol();
      if (iVar3 != iVar5) {
        local_18 = 0;
      }
    }
  }
  if ((param_1[0x14c] != 0) && (*(char *)((int)param_1 + 0x795) == '\0')) {
    if (local_18 == 1) {
      *(undefined4 *)(param_1[0x14c] + 0x24) = 0;
    }
    else {
      *(undefined4 *)(param_1[0x14c] + 0x24) = 1;
    }
  }
  if (local_18 == 0xb) {
    local_18 = 1;
  }
  if ((param_1[0x93] != 0x2d) || (*(char *)((int)param_1 + 0x795) == '\0')) {
    local_d4 = 1;
    if (*(int *)(local_48 + 0x24) == 30000) {
      if (DAT_005d03e4 == 1) {
LAB_005146f1:
        bVar1 = true;
      }
      else {
        iVar3 = __ftol();
        if ((((iVar3 >> 7 < 0x1b) || (iVar3 = __ftol(), 0x1e < iVar3 >> 7)) ||
            (iVar3 = __ftol(), iVar3 >> 7 < 0x15)) || (iVar3 = __ftol(), 0x18 < iVar3 >> 7)) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) goto LAB_005146f1;
        bVar1 = false;
      }
      if (bVar1) {
        local_d4 = 0;
      }
    }
    if ((*(char *)((int)param_1 + 0x79a) == '\x01') && (*(char *)((int)param_1 + 0x245) == '\0')) {
      local_d8 = 0;
      if (local_18 == 8) {
        local_d8 = 0xc1;
      }
      local_dc = (float)param_1[0xe] * *(float *)(&DAT_0092e7d0 + param_1[0x17] * 8) * 0.0392;
      if ((((char)param_1[0x1e7] == '\0') && (param_1[0x17] != 0x28)) ||
         (((char)param_1[0x1e7] != '\0' && (param_1[0x1e8] != 0x28)))) {
        pvVar6 = operator_new(0x180);
        local_8 = 2;
        if (pvVar6 == (void *)0x0) {
          local_270 = (int *)0x0;
        }
        else {
          local_270 = (int *)FUN_004c8f70(10,0x1c2,0x3dcccccd,0x3dcccccd,0x3dcccccd,local_dc,1,0x50)
          ;
        }
        local_8 = 0xffffffff;
        local_e4 = local_270;
        if (local_270 != (int *)0x0) {
          local_270[0x40] = 1;
          local_270[0x1b] = 1;
          FUN_00430f50();
          FUN_004310a0();
          FUN_0054ac09();
          param_1[0x18c] = uVar2;
        }
        local_e0 = 0x30101010;
        (**(code **)(*local_e4 + 0x3c))();
      }
    }
    else if (((local_d4 != 0) &&
             ((((*(int *)(DAT_005ccf9c + 0x2a39c) == 0 && (*(int *)(DAT_005ccf9c + 0x2a3a0) == 0))
               && ((param_1[0x1d0] == 3 && (*(char *)((int)param_1 + 0x795) == '\0')))) ||
              (((param_1[0x93] == 0x16 || (param_1[0x93] == 0x14)) && (param_1[0x1d0] == 2)))))) &&
            (iVar3 = param_1[0x18c], uVar7 = __ftol(), uVar7 < uVar2 - iVar3)) {
      local_e8 = 0;
      if (local_18 == 8) {
        local_e8 = 0xc1;
      }
      local_ec = (float)param_1[0xe] * *(float *)(&DAT_0092e7d0 + param_1[0x17] * 8) * 0.0012;
      if ((((char)param_1[0x1e7] == '\0') && (param_1[0x17] != 0x28)) ||
         (((char)param_1[0x1e7] != '\0' && (param_1[0x1e8] != 0x28)))) {
        pvVar6 = operator_new(0x180);
        local_8 = 3;
        if (pvVar6 == (void *)0x0) {
          local_274 = 0;
        }
        else {
          local_274 = FUN_004c8f70(local_e8,2000,0x3f000000,0x3f000000,0x3f000000,local_ec,1,0x50);
        }
        local_8 = 0xffffffff;
        local_f0 = local_274;
        if (local_274 != 0) {
          *(undefined4 *)(local_274 + 0x100) = 1;
          FUN_00430f50();
          FUN_004310a0();
          FUN_0054ac09();
          param_1[0x18c] = uVar2;
        }
      }
    }
  }
  if ((param_1[0x93] == 0x2d) || (*(char *)((int)param_1 + 0x795) != '\0')) goto LAB_00514f62;
  local_f4 = __ftol();
  if ((0xe6 < local_f4) && (local_f4 < 0x15e)) {
    local_f4 = 0xe6;
  }
  if (0x154 < local_f4) {
    local_f4 = 0x154;
  }
  if (param_1[0x96] == 3) {
    local_f4 = 200;
  }
  if (param_1[0x17] == 0xb) {
    local_f4 = local_f4 << 1;
  }
  if (param_1[0x17] == 0x27) {
    local_f4 = local_f4 << 1;
  }
  if (((uVar2 - param_1[0x18d] <= local_f4) || (param_1[0x96] < 1)) ||
     ((param_1[0x1d0] != 2 && (param_1[0x1d0] != 3)))) goto LAB_00514f62;
  if (local_18 == 8) {
    iVar3 = __ftol();
    if ((((iVar3 >> 7 < 0x1b) || (iVar3 = __ftol(), 0x1e < iVar3 >> 7)) ||
        (iVar3 = __ftol(), iVar3 >> 7 < 0x15)) || (iVar3 = __ftol(), 0x18 < iVar3 >> 7)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_00514ce3;
    bVar1 = true;
  }
  else {
LAB_00514ce3:
    bVar1 = false;
  }
  if ((bVar1) || (local_18 == 9)) {
    local_fc = 0xffff8866;
    if (local_18 == 9) {
      local_fc = 0x990000ff;
    }
    local_f8 = 2;
    local_104 = 0xc2;
    if ('\0' < (char)param_1[0x1e7]) {
      local_104 = 0xc3;
    }
    if (param_1[0x96] == 2) {
      local_104 = 0xc3;
      if (_DAT_005a3f2c < (float)param_1[0xe]) {
        local_f8 = 3;
      }
    }
    else if (param_1[0x96] == 3) {
      local_104 = 0xc4;
      local_f8 = 3;
      local_fc = 0xffffaa88;
    }
    else if (param_1[0x96] == 4) {
      local_104 = 0xc5;
      local_f8 = 4;
    }
    else if (param_1[0x96] == 5) {
      local_104 = 0xc6;
      local_f8 = 4;
    }
    if ((local_104 == 0xc2) && (_DAT_005a430c < (float)param_1[0xe])) {
      local_f8 = 3;
    }
    if (param_1[0x17] == 0x27) {
      local_f8 = 4;
    }
    pvVar6 = operator_new(0xc0);
    local_8 = 4;
    if (pvVar6 == (void *)0x0) {
      local_280 = (int *)0x0;
    }
    else {
      local_280 = (int *)FUN_004df250();
    }
    local_8 = 0xffffffff;
    local_100 = local_280;
    if (local_280 != (int *)0x0) {
      local_280[0x2e] = 1;
      local_280[0x2d] = 0;
      local_280[0x28] = 3000;
      local_280[0x26] = (int)((float)param_1[0xd] - _DAT_005a4380);
      (**(code **)(*local_280 + 0x44))();
      (**(code **)(*local_100 + 0x3c))();
      FUN_0054ac09();
      param_1[0x18d] = uVar2;
    }
  }
LAB_00514f62:
  if ((param_1[0x17] == 0x14) &&
     ((*(short *)((int)param_1 + 0x1d6) == 2 ||
      ((*(short *)((int)param_1 + 0x1d6) == 4 && ((short)param_1[0x76] == 1)))))) {
    local_110 = 1000;
    local_10c = 2;
    local_108 = 0xffff0000;
    if ((*(short *)((int)param_1 + 0x1d6) != 2) || (_DAT_005a414c <= (float)param_1[0xe])) {
      if (*(short *)((int)param_1 + 0x1d6) == 4) {
        local_10c = 4;
        local_108 = 0xffff9900;
        if ((float)param_1[0xe] < _DAT_005a414c) {
          local_110 = 800;
          local_10c = 6;
        }
      }
    }
    else {
      local_110 = 0x4b0;
      local_10c = 6;
    }
    if (local_110 < uVar2 - param_1[0x18a]) {
      for (local_114 = 1; local_114 < 10 - local_10c; local_114 = local_114 + 1) {
        pvVar6 = operator_new(0x180);
        local_8 = 5;
        if (pvVar6 == (void *)0x0) {
          local_284 = (int *)0x0;
        }
        else {
          local_284 = (int *)FUN_004c8f70(0x2c,2000,(float)param_1[0xe] * _DAT_005a3834,
                                          (float)param_1[0xe] * _DAT_005a3834,
                                          (float)param_1[0xe] * _DAT_005a3834,0x3a03126f,1,0x50);
        }
        local_8 = 0xffffffff;
        local_118 = local_284;
        if (local_284 != (int *)0x0) {
          (**(code **)(*local_284 + 0x3c))();
          local_118[0x1b] = 1;
          local_118[0x41] = 1;
          FUN_004310a0();
          FUN_0054ac09();
        }
      }
      param_1[0x18a] = uVar2;
    }
  }
  if (param_1[0x18] != 0) {
    if ((param_1[0x12] == 0) && (param_1[0x1d9] == 1)) {
      local_124 = *(int *)(param_1[0x18] + 0x2e4);
      local_11c = param_1[0x17];
      local_120 = *(int *)(param_1[0x18] + 0x2d0);
      local_128 = *(int *)(&DAT_0065a4a8 + local_120 * 4 + local_11c * 900);
      if ((uint)(param_1[0x180] + local_124 * local_128 * 4) < uVar2) {
        if (param_1[0x17a] == 0) {
          if ((param_1[0x1d0] != 0xb) && (param_1[0x1d9] == 1)) {
            param_1[0x1d9] = 0;
          }
        }
        else if (param_1[0x17a] == 1) {
          if ((param_1[0x1d0] < 4) || (9 < param_1[0x1d0])) {
            param_1[0x1d9] = 0;
          }
          else if ((*(int *)(DAT_005d0504 + 0x1c) == 0) &&
                  ((uint)(param_1[0x180] + *(int *)(param_1[0x18] + 0x2e4) * local_128 * 4) < uVar2)
                  ) {
            FUN_0051dba4(1,1);
            param_1[0x1d9] = 0;
          }
        }
      }
    }
    else if (param_1[0x12] == 1) {
      FUN_005161af();
      local_134 = *(int *)(param_1[0x18] + 0x2e4);
      local_12c = param_1[0x17];
      local_130 = *(int *)(param_1[0x18] + 0x2d0);
      if ((((char)param_1[0x1e7] != '\0') && (param_1[0x67] != 0)) &&
         (FUN_004bda22(), param_1[0x1d0] == 0xb)) {
        local_12c = param_1[0x1e8];
        local_134 = *(int *)(param_1[0x67] + 0x2e4);
        local_130 = *(int *)(param_1[0x67] + 0x2d0);
      }
      FUN_004bda22();
      if (('\0' < *(char *)((int)param_1 + 0x79b)) && (param_1[0x66] != 0)) {
        FUN_004bda22();
      }
      local_138 = *(uint *)(&DAT_0065a4a8 + local_130 * 4 + local_12c * 900);
      if (2 < local_138) {
        local_138 = local_138 - 2;
      }
      if ((((uint)param_1[8] < 1000) && (param_1[0x1d9] == 1)) && (param_1[0x1d0] == 3)) {
        param_1[0x1d9] = 0;
      }
      if (param_1[0x180] + local_134 * local_138 * 4 < uVar2) {
        if (param_1[0x17a] == 0) {
          if (param_1[0x1d0] == 0xb) {
            FUN_0051dba4(0xc,1);
            if ((param_1[0x93] == 0x40) && ((short)param_1[0x94] == 0x18d)) {
              *(undefined1 *)((int)param_1 + 0x795) = 1;
              pvVar6 = operator_new(0x90);
              local_8 = 6;
              if (pvVar6 == (void *)0x0) {
                local_28c = 0;
              }
              else {
                uVar21 = 1000;
                puVar8 = (undefined8 *)FUN_00430f50(0,0,0);
                uVar17 = *puVar8;
                uVar19 = CONCAT44(uVar21,*(undefined4 *)(puVar8 + 1));
                uVar16 = 1;
                uVar15 = 0x3f800000;
                uVar14 = 0x38;
                uVar13 = 1;
                uVar12 = 0xffff3333;
                uVar11 = 0x41200000;
                uVar23 = 8;
                uVar21 = 0;
                puVar4 = (undefined4 *)FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
                local_28c = FUN_004d4de0(*puVar4,puVar4[1],puVar4[2],uVar21,uVar23,uVar11,uVar12,
                                         uVar13,uVar14,uVar15,uVar16,uVar17,uVar19);
              }
              local_8 = 0xffffffff;
              local_13c = local_28c;
              if (local_28c != 0) {
                FUN_0054ac09();
              }
            }
            if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
              local_140 = DAT_0067cf38;
              iVar3 = (**(code **)(*param_1 + 0x50))();
              if (iVar3 == 0) {
                uVar17 = 0xb;
                puVar18 = &DAT_00a34ef8;
                (**(code **)(**(int **)(local_140 + 0x8c) + 0x8c))();
                (**(code **)(**(int **)(local_140 + 0x8c) + 0x60))(1,puVar18,uVar17);
              }
            }
            else if (999 < (uint)param_1[8]) {
              if (param_1[0x17e] == 0) {
                param_1[0x17e] = uVar2;
              }
              if ((char)param_1[0x91] == '\0') {
                param_1[0x95] = 1;
              }
            }
            param_1[0x1d9] = 0;
          }
          else if (param_1[0x1d0] == 0xd) {
            FUN_0051dba4(0x18,1);
          }
          else if (param_1[0x1d0] == 0xf) {
            FUN_0051dba4(0x1a,1);
          }
          else if (param_1[0x1d9] == 1) {
            if ((param_1[0x93] == 0x21) && (param_1[0x1d0] == 4)) {
              *(undefined4 *)(param_1[0x18] + 0x2d0) = 0;
              FUN_0051dba4(4,0);
            }
            else if (param_1[0x1d0] == 4) {
              *(undefined4 *)(param_1[0x18] + 0x2d0) = 0;
              FUN_0051dba4(5,0);
            }
            else if (param_1[0x1d0] == 5) {
              *(undefined4 *)(param_1[0x18] + 0x2d0) = 0;
              FUN_0051dba4(6,0);
            }
            else if (param_1[0x1d0] == 6) {
              *(undefined4 *)(param_1[0x18] + 0x2d0) = 0;
              FUN_0051dba4(4,0);
            }
            param_1[0x1d9] = 0;
          }
          else if ((((param_1[0x1d8] == 1) && (-1 < param_1[0x1d5])) && (param_1[0x1d5] < 3)) &&
                  (param_1[param_1[0x1d5] + 0x1d2] != -1)) {
            param_1[0x1d5] = param_1[0x1d5] + 1;
            FUN_0051dba4(param_1[param_1[0x1d5] + 0x1d1],0);
          }
          else {
            if ((param_1[0x1d0] == 0x17) || (param_1[0x1d0] == 0x13)) {
              local_148 = 1.0;
              if ((char)param_1[0x1e7] == '\x01') {
                local_148 = 1.5;
              }
              if ((param_1[0x19c] == 1) && (*(int *)(param_1[0x18] + 0x2f8) != 0)) {
                if ((param_1[0x58] == 0x29) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
                  *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                       local_148 * (float)param_1[0x19f];
                }
                else {
                  *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                       local_148 * (float)param_1[0x19e];
                }
                *(uint *)(*(int *)(param_1[0x18] + 0x2f8) + 0x80) = uVar2;
              }
              if ((param_1[0x19d] == 1) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
                *(float *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28e8) =
                     local_148 * (float)param_1[0x19f];
                *(uint *)(*(int *)(param_1[0x18] + 0x2fc) + 0x80) = uVar2;
              }
            }
            *(undefined1 *)(param_1 + 0x1e0) = 0;
            param_1[0x1d8] = 0;
            param_1[0x1d5] = -1;
            for (local_144 = 0; local_144 < 4; local_144 = local_144 + 1) {
              param_1[local_144 + 0x1d1] = -1;
            }
            if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
              *(undefined1 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28f0) = 0;
            }
            if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
              *(undefined1 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28f0) = 0;
            }
            if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
              *(undefined1 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28f4) = 0;
            }
            if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
              *(undefined1 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28f4) = 0;
            }
            FUN_0051dba4(1,1);
            param_1[0x1d9] = 0;
          }
        }
        else if (param_1[0x17a] == 1) {
          if ((param_1[0x1d0] < 4) || (6 < param_1[0x1d0])) {
            if ((param_1[0x1d0] < 7) || (9 < param_1[0x1d0])) {
              if (param_1[0x1d0] == 1) {
                param_1[0x1d9] = 0;
              }
              else {
                param_1[0x1d9] = 0;
              }
            }
            else {
              local_150 = 1.0;
              if ((char)param_1[0x1e7] == '\x01') {
                local_150 = 1.5;
              }
              if (param_1[0x19c] == 1) {
                if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
                  if ((param_1[0x58] == 0x29) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
                    *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                         local_150 * (float)param_1[0x19f];
                  }
                  else {
                    *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                         local_150 * (float)param_1[0x19e];
                  }
                }
                if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
                  *(uint *)(*(int *)(param_1[0x18] + 0x2f8) + 0x80) = uVar2;
                }
              }
              if (param_1[0x19d] == 1) {
                if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
                  *(float *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28e8) =
                       local_150 * (float)param_1[0x19f];
                }
                if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
                  *(uint *)(*(int *)(param_1[0x18] + 0x2fc) + 0x80) = uVar2;
                }
              }
              if ((*(int *)(DAT_005d0504 + 0x20) == 0) &&
                 (param_1[0x180] + *(int *)(param_1[0x18] + 0x2e4) * local_138 * 4 < uVar2)) {
                FUN_0051dba4(1,1);
                param_1[0x1d9] = 0;
              }
            }
          }
          else {
            local_14c = 1.0;
            if ((char)param_1[0x1e7] == '\x01') {
              local_14c = 1.5;
            }
            if ((param_1[0x19c] == 1) && (*(int *)(param_1[0x18] + 0x2f8) != 0)) {
              if ((param_1[0x58] == 0x29) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
                *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                     local_14c * (float)param_1[0x19f];
              }
              else {
                *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                     local_14c * (float)param_1[0x19e];
              }
              *(uint *)(*(int *)(param_1[0x18] + 0x2f8) + 0x80) = uVar2;
            }
            if ((param_1[0x19d] == 1) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
              *(float *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28e8) =
                   local_14c * (float)param_1[0x19f];
              *(uint *)(*(int *)(param_1[0x18] + 0x2fc) + 0x80) = uVar2;
            }
            if ((*(int *)(DAT_005d0504 + 0x1c) == 0) &&
               (param_1[0x180] + *(int *)(param_1[0x18] + 0x2e4) * local_138 * 4 < uVar2)) {
              FUN_0051dba4(1,1);
              param_1[0x1d9] = 0;
            }
          }
        }
      }
      FUN_00517231();
      if (param_1[0x14c] != 0) {
        if ((char)param_1[0x1e7] == '\0') {
          if (param_1[0x18] != 0) {
            FUN_00430f20();
            (**(code **)(*(int *)param_1[0x14c] + 0x3c))();
          }
        }
        else if (param_1[0x67] != 0) {
          FUN_00430f20();
          (**(code **)(*(int *)param_1[0x14c] + 0x3c))();
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

