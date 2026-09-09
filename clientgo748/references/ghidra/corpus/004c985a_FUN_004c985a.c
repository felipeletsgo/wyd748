// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c985a | Name: FUN_004c985a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004c985a(int *param_1,int param_2)

{
  uint uVar1;
  undefined4 *puVar2;
  int iVar3;
  float10 fVar4;
  undefined1 *puVar5;
  undefined4 uVar6;
  undefined1 *puVar7;
  undefined4 uVar8;
  undefined1 local_254 [12];
  undefined1 local_248 [12];
  undefined1 local_23c [12];
  undefined1 local_230 [12];
  undefined1 local_224 [12];
  undefined1 local_218 [12];
  undefined1 local_20c [12];
  undefined1 local_200 [12];
  undefined1 local_1f4 [12];
  undefined1 local_1e8 [12];
  undefined1 local_1dc [12];
  undefined1 local_1d0 [20];
  undefined1 local_1bc [64];
  float local_17c;
  float local_178;
  float local_174;
  int local_170;
  uint local_16c;
  float local_168;
  uint local_164;
  float local_160;
  float local_15c;
  int local_158;
  uint local_154;
  float local_150;
  uint local_14c;
  float local_148;
  float local_144;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  undefined1 local_12c [64];
  undefined1 local_ec [64];
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  int local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  int local_5c;
  int local_58;
  float local_54;
  int local_50;
  int local_4c;
  float local_48;
  uint local_44;
  int local_40;
  int local_3c;
  int local_38;
  uint local_34;
  int local_30;
  float local_2c;
  float local_28;
  int local_24;
  int local_20;
  int local_1c;
  uint local_18;
  int local_14;
  int local_10;
  uint local_c;
  uint local_8;
  
  if ((param_1[0x43] != 0) || (param_2 == 0)) {
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    if (param_1[0x3f] == 0) {
      param_1[0x3f] = uVar1;
    }
    if (uVar1 < (uint)param_1[0x3f]) {
      local_8 = param_1[0x3e];
    }
    else {
      local_8 = uVar1 - param_1[0x3f];
    }
    if ((param_1[0x3e] == 0) || ((int)local_8 < param_1[0x3e])) {
      if ((param_1[7] != 0) && (param_1[0x52] != 9)) {
        if (*(int *)(param_1[7] + 0x24) == 3) {
          local_10 = param_1[7];
          if (*(char *)(local_10 + 0x79c) == '\x01') {
            FUN_004310a0(param_1[7] + 0x50);
            param_1[0x1e] = (int)((float)param_1[0x39] / _DAT_005a365c + (float)param_1[0x1e]);
          }
          else {
            param_1[0x1d] = *(int *)(param_1[7] + 0x28);
            param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
            param_1[0x1e] =
                 (int)((float)param_1[0x39] / _DAT_005a365c + *(float *)(param_1[7] + 0x30));
          }
        }
        else {
          param_1[0x1d] = *(int *)(param_1[7] + 0x28);
          param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
          param_1[0x1e] =
               (int)((float)param_1[0x39] / _DAT_005a365c + *(float *)(param_1[7] + 0x30));
        }
      }
      local_c = param_1[0x49] * param_1[0x4a];
      if (local_c == 0) {
        local_c = 1;
      }
      if (param_1[0x4a] == 0) {
        param_1[0x48] = 0;
      }
      else {
        param_1[0x48] =
             (int)(((ulonglong)local_8 % (ulonglong)local_c) / (ulonglong)(uint)param_1[0x4a]);
      }
      if (param_1[0x3e] != 0) {
        param_1[0x46] = (int)((float)(int)local_8 / (float)param_1[0x3e]);
      }
      if ((float)param_1[0x46] < _DAT_005a3534) {
        param_1[0x46] = 0x3c23d70a;
      }
      if (_DAT_005a3660 < (float)param_1[0x46]) {
        param_1[0x46] = 0x3f800000;
      }
      if (param_1[0x41] == 1) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148);
        local_28 = (float)fVar4;
        local_14 = __ftol();
        local_1c = __ftol();
        local_24 = __ftol();
        local_18 = __ftol();
        for (local_20 = 0; local_20 < 4; local_20 = local_20 + 1) {
          param_1[local_20 * 6 + 0x23] =
               local_14 << 0x18 | local_1c << 0x10 | local_24 << 8 | local_18;
        }
      }
      else if (param_1[0x41] == 2) {
        local_44 = (uint)param_1 % 100;
        local_48 = _DAT_005a3660 - (float)param_1[0x47];
        local_2c = (float)((local_44 * 200 + uVar1) % 3000) / _DAT_005a35c0;
        fVar4 = (float10)FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        if (fVar4 <= (float10)_DAT_005a3534) {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        else {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        local_30 = __ftol();
        fVar4 = (float10)FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        if (fVar4 <= (float10)_DAT_005a3534) {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        else {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        local_38 = __ftol();
        fVar4 = (float10)FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        if (fVar4 <= (float10)_DAT_005a3534) {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        else {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        local_40 = __ftol();
        fVar4 = (float10)FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        if (fVar4 <= (float10)_DAT_005a3534) {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        else {
          FUN_00423b00(local_2c * _DAT_005a4148 + local_2c * _DAT_005a4148);
        }
        local_34 = __ftol();
        for (local_3c = 0; local_3c < 4; local_3c = local_3c + 1) {
          param_1[local_3c * 6 + 0x23] =
               local_30 << 0x18 | local_38 << 0x10 | local_40 << 8 | local_34;
        }
      }
      else if (param_1[0x41] == 3) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148);
        local_54 = (float)fVar4;
        local_4c = __ftol();
        for (local_50 = 0; local_50 < 4; local_50 = local_50 + 1) {
          param_1[local_50 * 6 + 0x23] =
               local_4c << 0x18 | param_1[0x4c] << 0x10 | param_1[0x4d] << 8 | param_1[0x4e];
        }
      }
      else if (param_1[0x41] == 4) {
        if (_DAT_005a40ec <= (float)param_1[0x46]) {
          local_60 = _DAT_005a3660 - ((float)param_1[0x46] - _DAT_005a40ec) * _DAT_005a47d4;
        }
        else {
          local_60 = (float)param_1[0x46] * _DAT_005a47d8;
        }
        local_58 = __ftol();
        for (local_5c = 0; local_5c < 4; local_5c = local_5c + 1) {
          param_1[local_5c * 6 + 0x23] =
               local_58 << 0x18 | param_1[0x4c] << 0x10 | param_1[0x4d] << 8 | param_1[0x4e];
        }
      }
      if (param_1[0x52] == 1) {
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
      }
      else if (param_1[0x52] == 2) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_64 = (float)fVar4;
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] = (int)(local_64 * (float)param_1[0x4f] + (float)param_1[0x53]);
      }
      else if (param_1[0x52] == 3) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_68 = (float)fVar4;
        fVar4 = (float10)FUN_00493bf0((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_6c = (float)fVar4;
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] = (int)(local_68 * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] = (int)(local_6c * (float)param_1[0x4f] + (float)param_1[0x55]);
      }
      else if (param_1[0x52] == 4) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_70 = (float)fVar4;
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] = (int)(local_70 * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x55]);
      }
      else if (param_1[0x52] == 5) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_74 = (float)fVar4;
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] = (int)(local_74 * (float)param_1[0x4f] + (float)param_1[0x55]);
      }
      else if (param_1[0x52] == 6) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_78 = (float)fVar4;
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a3794 * _DAT_005a4148 *
                                      (float)param_1[0x51]);
        local_7c = (float)fVar4;
        param_1[0x1e] = (int)(local_7c * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] = (int)(local_78 * (float)param_1[0x4f] + (float)param_1[0x55]);
      }
      else if (param_1[0x52] == 7) {
        fVar4 = (float10)FUN_00493bf0((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_84 = (float)fVar4;
        fVar4 = (float10)FUN_00493bf0((float)param_1[0x46] * _DAT_005a3794 * _DAT_005a4148 *
                                      (float)param_1[0x51]);
        local_80 = (float)fVar4;
        param_1[0x1e] = (int)(local_80 * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] = (int)(local_84 * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x55]);
      }
      else if (param_1[0x52] == 8) {
        fVar4 = (float10)FUN_00423b00((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_88 = (float)fVar4;
        fVar4 = (float10)FUN_00493bf0((float)param_1[0x46] * _DAT_005a4148 * (float)param_1[0x51]);
        local_90 = (float)fVar4;
        fVar4 = (float10)FUN_00493bf0((float)param_1[0x46] * _DAT_005a3794 * _DAT_005a4148 *
                                      (float)param_1[0x51]);
        local_8c = (float)fVar4;
        param_1[0x1e] = (int)(local_8c * (float)param_1[0x50] + (float)param_1[0x54]);
        param_1[0x1d] =
             (int)(local_90 * (float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] =
             (int)(local_88 * (float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x55]);
      }
      else if (param_1[0x52] == 9) {
        if (param_1[7] != 0) {
          if (*(int *)(param_1[7] + 0x24) == 3) {
            local_94 = param_1[7];
            if (*(char *)(local_94 + 0x79c) == '\x01') {
              FUN_004310a0(param_1[7] + 0x50);
              param_1[0x1e] =
                   (int)((*(float *)(param_1[7] + 0x54) + _DAT_005a3660) * (float)param_1[0x46] +
                        (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x54]);
              param_1[0x1d] =
                   (int)(*(float *)(param_1[7] + 0x50) * (float)param_1[0x46] +
                        (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x53]);
              param_1[0x1f] =
                   (int)(*(float *)(param_1[7] + 0x54) * (float)param_1[0x46] +
                        (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x55]);
            }
            else {
              param_1[0x1e] =
                   (int)((*(float *)(param_1[7] + 0x30) + _DAT_005a3660) * (float)param_1[0x46] +
                        (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x54]);
              param_1[0x1d] =
                   (int)(*(float *)(param_1[7] + 0x28) * (float)param_1[0x46] +
                        (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x53]);
              param_1[0x1f] =
                   (int)(*(float *)(param_1[7] + 0x2c) * (float)param_1[0x46] +
                        (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x55]);
            }
          }
          else {
            param_1[0x1e] =
                 (int)((*(float *)(param_1[7] + 0x30) + _DAT_005a3660) * (float)param_1[0x46] +
                      (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x54]);
            param_1[0x1d] =
                 (int)(*(float *)(param_1[7] + 0x28) * (float)param_1[0x46] +
                      (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x53]);
            param_1[0x1f] =
                 (int)(*(float *)(param_1[7] + 0x2c) * (float)param_1[0x46] +
                      (_DAT_005a3660 - (float)param_1[0x46]) * (float)param_1[0x55]);
          }
        }
        if (param_1[0x56] != 0) {
          puVar2 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
          (**(code **)(*(int *)param_1[0x56] + 0x3c))(*puVar2,puVar2[1]);
        }
      }
      else if (param_1[0x52] == 10) {
        local_9c = (float)(uVar1 % 2000) / _DAT_005a441c;
        fVar4 = (float10)FUN_00423b00(local_9c * _DAT_005a4148 + local_9c * _DAT_005a4148);
        local_98 = (float)fVar4;
        fVar4 = (float10)FUN_00493bf0(local_9c * _DAT_005a4148 + local_9c * _DAT_005a4148);
        local_a0 = (float)fVar4;
        param_1[0x1d] = (int)(local_98 * (float)param_1[0x4f] + (float)param_1[0x1d]);
        param_1[0x1e] = (int)(local_a0 * (float)param_1[0x50] + (float)param_1[0x1e]);
        param_1[0x1f] = (int)(local_a0 * (float)param_1[0x4f] + (float)param_1[0x1f]);
      }
      else if (param_1[0x52] == 0xb) {
        param_1[0x1d] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
      }
      else if (param_1[0x52] == 0xc) {
        param_1[0x1d] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x53]);
        param_1[0x1f] = (int)((float)param_1[0x46] * (float)param_1[0x4f] + (float)param_1[0x55]);
        param_1[0x1e] = (int)((float)param_1[0x46] * (float)param_1[0x50] + (float)param_1[0x54]);
      }
      else if (param_1[0x52] == 0xd) {
        if (_DAT_005a4158 <= (float)param_1[0x46]) {
          uVar8 = 0x3e99999a;
          puVar7 = local_224;
          FUN_004c8c60(local_218,param_1[0x46]);
          uVar8 = FUN_004c8c60(puVar7,uVar8);
          puVar7 = local_230;
          uVar6 = 0x41200000;
          puVar5 = local_200;
          FUN_004c8c60(local_1f4,0x3e4ccccd);
          uVar6 = FUN_004c8c60(puVar5,uVar6);
          FUN_00493d50(local_20c,uVar6);
          uVar8 = FUN_00493d50(puVar7,uVar8);
          FUN_004310a0(uVar8);
        }
        else {
          uVar8 = 0x41200000;
          puVar7 = local_1dc;
          FUN_004c8c60(local_1d0,param_1[0x46]);
          uVar8 = FUN_004c8c60(puVar7,uVar8);
          uVar8 = FUN_00493d50(local_1e8,uVar8);
          FUN_004310a0(uVar8);
        }
      }
      else if (param_1[0x52] == 0xe) {
        param_1[0x5f] = (int)((float)param_1[0x46] * _DAT_005a4214 * _DAT_005a4148);
      }
      else if (param_1[0x52] == 0xf) {
        param_1[0x5f] = (int)(-(float)param_1[0x46] * _DAT_005a4214 * _DAT_005a4148);
      }
      else if (param_1[0x52] == 0x10) {
        local_a8 = (float)((int)local_8 % 2000) / _DAT_005a441c;
        fVar4 = (float10)FUN_00423b00(local_a8 * _DAT_005a4148 + local_a8 * _DAT_005a4148);
        local_a4 = (float)fVar4;
        fVar4 = (float10)FUN_00493bf0(local_a8 * _DAT_005a4148 + local_a8 * _DAT_005a4148);
        local_ac = (float)fVar4;
        param_1[0x1d] = (int)(local_a4 * (float)param_1[0x4f] + (float)param_1[0x1d]);
        param_1[0x1e] = (int)(local_ac * (float)param_1[0x50] + (float)param_1[0x1e]);
        param_1[0x1f] = (int)(local_ac * (float)param_1[0x4f] + (float)param_1[0x1f]);
      }
      else if (param_1[0x52] == 0x11) {
        uVar8 = 0x40a00000;
        puVar7 = local_248;
        FUN_004c8c60(local_23c,param_1[0x46]);
        uVar8 = FUN_004c8c60(puVar7,uVar8);
        uVar8 = FUN_00493d50(local_254,uVar8);
        FUN_004310a0(uVar8);
      }
      iVar3 = (**(code **)(*param_1 + 0x38))();
      if (iVar3 == 1) {
        FUN_00430f10();
        FUN_00430f10();
        if (param_1[0x42] == 1) {
          local_140 = -*(float *)(DAT_013b71e8[0x6c21] + 0x2c);
          local_130 = *(float *)(DAT_013b71e8[0x6c21] + 0x28);
          local_13c = (float)param_1[0x38];
          local_138 = (float)param_1[0x39];
          local_134 = (float)param_1[0x3a];
          if (param_1[0x3e] != 0) {
            local_13c = (float)(uVar1 - param_1[0x3f]) * (float)param_1[0x3b] + (float)param_1[0x38]
            ;
            local_138 = (float)(uVar1 - param_1[0x3f]) * (float)param_1[0x3c] + (float)param_1[0x39]
            ;
            local_134 = (float)(uVar1 - param_1[0x3f]) * (float)param_1[0x3d] + (float)param_1[0x3a]
            ;
          }
          if (param_1[0x57] == 1) {
            local_150 = (float)(uVar1 % 2000) / _DAT_005a441c;
            fVar4 = (float10)FUN_00423b00(local_150 * _DAT_005a4148);
            local_148 = (float)fVar4;
            local_144 = (float)(fVar4 * (float10)_DAT_005a4158);
            local_13c = local_13c + local_144;
            local_138 = local_138 + local_144;
            iVar3 = __ftol();
            local_14c = 0xff - iVar3;
            local_154 = local_14c * 0x1000000 | local_14c * 0x10000 | local_14c * 0x100 | local_14c;
            for (local_158 = 0; local_158 < 4; local_158 = local_158 + 1) {
              param_1[local_158 * 6 + 0x23] = local_154;
            }
            param_1[0x4b] = local_14c;
            param_1[0x4c] = local_14c;
            param_1[0x4d] = local_14c;
            param_1[0x4e] = local_14c;
          }
          else if ((param_1[0x57] == 2) || (param_1[0x57] == 3)) {
            local_168 = (float)(uVar1 % 4000) / _DAT_005a4420;
            fVar4 = (float10)FUN_00423b00(local_168 * _DAT_005a4148);
            local_160 = (float)fVar4;
            local_15c = (float)(fVar4 * (float10)_DAT_005a40f0);
            local_13c = local_13c + local_15c;
            local_138 = local_138 + local_15c;
            iVar3 = __ftol();
            local_164 = 0xff - iVar3;
            local_16c = local_164 * 0x1000000 | local_164 * 0x10000 | local_164 * 0x100 | local_164;
            for (local_170 = 0; local_170 < 4; local_170 = local_170 + 1) {
              param_1[local_170 * 6 + 0x23] = local_16c;
            }
            param_1[0x4b] = local_164;
            param_1[0x4c] = local_164;
            param_1[0x4d] = local_164;
            param_1[0x4e] = local_164;
            if ((_DAT_005a47cc <= local_168) && (param_1[0x57] == 2)) {
              param_1[0x57] = 1;
            }
          }
          FUN_00430fc0(param_1 + 0xb);
          thunk_FUN_0056230d(local_12c,local_13c,local_138,local_134);
          if (param_1[0x40] == 1) {
            thunk_FUN_0056239d(local_ec,param_1[0x1d],
                               local_138 / _DAT_005a365c + (float)param_1[0x1e],param_1[0x1f]);
          }
          else {
            thunk_FUN_0056239d(local_ec,param_1[0x1d],param_1[0x1e],param_1[0x1f]);
          }
          if (param_1[0x57] == 0) {
            thunk_FUN_0056242d(param_1 + 0xb,local_140);
            thunk_FUN_00562821(param_1 + 0xb,_DAT_005a4380 - local_130,local_140,param_1[0x45]);
          }
          thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_12c);
          thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_ec);
        }
        else {
          local_17c = (float)param_1[0x38];
          local_178 = (float)param_1[0x39];
          local_174 = (float)param_1[0x3a];
          if (param_1[0x3e] != 0) {
            local_17c = (float)(uVar1 - param_1[0x3f]) * (float)param_1[0x3b] + (float)param_1[0x38]
            ;
            local_178 = (float)(uVar1 - param_1[0x3f]) * (float)param_1[0x3c] + (float)param_1[0x39]
            ;
            local_174 = (float)(uVar1 - param_1[0x3f]) * (float)param_1[0x3d] + (float)param_1[0x3a]
            ;
          }
          FUN_00430fc0(param_1 + 0xb);
          thunk_FUN_0056230d(local_12c,local_17c,local_178,local_174);
          if (param_1[0x40] == 1) {
            thunk_FUN_0056239d(local_ec,param_1[0x1d],
                               local_178 / _DAT_005a365c + (float)param_1[0x1e],param_1[0x1f]);
          }
          else {
            thunk_FUN_0056239d(local_ec,param_1[0x1d],param_1[0x1e],param_1[0x1f]);
          }
          thunk_FUN_005624c9(param_1 + 0xb,param_1[0x45]);
          if ((param_1[0x52] == 0xe) || (param_1[0x52] == 0xf)) {
            FUN_00430f10();
            thunk_FUN_00562604(local_1bc,param_1 + 0x5c,param_1[0x5f]);
            thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_1bc);
          }
          thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_12c);
          thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_ec);
        }
      }
    }
    else {
      param_1[10] = 0;
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
  }
  return 1;
}

