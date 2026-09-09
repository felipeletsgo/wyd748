// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c3a1b | Name: FUN_004c3a1b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004c3a1b(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined1 local_88 [4];
  undefined4 local_84;
  uint local_78;
  float local_74;
  float local_70;
  undefined4 local_6c;
  float local_68;
  float local_64;
  undefined4 local_60;
  undefined1 local_5c [16];
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  undefined4 local_3c;
  float local_38;
  float local_34;
  float local_30;
  undefined4 local_2c;
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  float local_18;
  float local_14;
  float local_10;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0xc0) != 0) {
    FUN_0058f220(&local_48,*(int *)(param_1 + 0xc0) + 0x288,0x44);
    if (((0 < *(short *)(param_1 + 0xc4)) && (*(char *)(*(int *)(param_1 + 0xc0) + 0x306) == '\x01')
        ) && (*(int *)(DAT_005ccf9c + 0x2a390) == 0)) {
      local_28 = local_48 / _DAT_005a40f0;
      local_24 = local_44 / _DAT_005a46f0;
      local_20 = local_40 / _DAT_005a46ec;
      local_2c = local_3c;
      local_1c = local_3c;
      local_48 = local_28;
      local_44 = local_24;
      local_40 = local_20;
      local_38 = local_28;
      local_34 = local_24;
      local_30 = local_20;
      if (6 < *(short *)(param_1 + 0xc4)) {
        if ((*(short *)(param_1 + 0xc6) == 4) && (*(short *)(param_1 + 0xc4) < 10)) {
          local_48 = local_28 / _DAT_005a365c;
          local_44 = local_24 / _DAT_005a46e8;
          local_40 = local_20 / _DAT_005a4524;
          local_38 = local_48;
          local_34 = local_44;
          local_30 = local_40;
        }
        else {
          local_48 = local_28 / _DAT_005a3834;
          local_44 = local_24 / _DAT_005a46e4;
          local_40 = local_20 / _DAT_005a46e0;
          local_38 = local_48 / _DAT_005a3f2c;
          local_34 = local_44 / _DAT_005a3f2c;
          local_30 = local_40 / _DAT_005a3f2c;
        }
      }
    }
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0xa8))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),&local_48);
    local_78 = (**(code **)(*DAT_0092e654 + 8))();
    local_74 = (float)(local_78 % 10000) / _DAT_005a420c;
    FUN_00430f80(0x3f800000,0,local_74,0x443f40a4);
    FUN_00430f10();
    uVar1 = FUN_00430f80(0x3e800000,0x3e800000,0x3e800000,0x3f800000);
    uVar2 = FUN_004c5bd0(&local_38);
    FUN_004c5c70(local_5c,uVar2,uVar1);
    if (((*(short *)(param_1 + 0xc6) < 0x74) || (0x7d < *(short *)(param_1 + 0xc6))) ||
       (*(short *)(param_1 + 0xc4) < 1)) {
      uVar1 = FUN_004c5bd0(&local_18);
      FUN_004c5c10(uVar1);
    }
    else {
      FUN_004c5bd0(&local_18);
      local_8c = 0x3f666666;
      if (local_18 < _DAT_005a4144) {
        local_98 = 0x3f0ccccd;
      }
      if (local_14 < _DAT_005a4144) {
        local_94 = 0x3f0ccccd;
      }
      if (local_10 < _DAT_005a4144) {
        local_90 = 0x3f0ccccd;
      }
      local_3c = 0x3f666666;
      if (_DAT_005a4214 < local_48) {
        local_48 = 0.1;
      }
      if (_DAT_005a4214 < local_44) {
        local_44 = 0.1;
      }
      if (_DAT_005a4214 < local_40) {
        local_40 = 0.1;
      }
      local_c8 = local_98;
      local_c4 = local_94;
      local_c0 = local_90;
      local_bc = 0x3f666666;
      FUN_004c5c10(&local_c8);
    }
    local_84 = local_8;
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x13c))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),8,&local_48,1);
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x13c))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),7,local_5c,1);
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x13c))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),0,local_88,1);
    local_4c = *(float *)(DAT_005ccf9c + 0x2b048);
    if (**(int **)(param_1 + 0xc0) == 0x3d) {
      local_4c = local_4c * _DAT_005a4520;
    }
    local_70 = local_4c - *(float *)(DAT_005ccf9c + 0x2b044);
    if (local_70 == _DAT_005a3534) {
      local_70 = 0.01;
    }
    local_6c = 0x3f800000;
    local_68 = local_4c;
    local_64 = _DAT_005a3660 / local_70;
    local_60 = 0;
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x13c))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),6,&local_6c,1);
  }
  return;
}

