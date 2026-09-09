// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cbeb6 | Name: FUN_004cbeb6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cbeb6(int *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  float10 fVar4;
  float local_104;
  undefined1 local_ec [64];
  float local_ac;
  float local_a8;
  float local_a4;
  undefined1 local_a0 [64];
  uint local_60;
  int local_5c;
  int local_58;
  int local_54;
  uint local_50;
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
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  if (param_1[0x3d] == 0) {
    param_1[0x3d] = uVar1;
  }
  if (uVar1 < (uint)param_1[0x3d]) {
    local_c = param_1[0x3c];
  }
  else {
    local_c = uVar1 - param_1[0x3d];
  }
  if ((param_1[0x3c] != 0) && (param_1[0x3c] < local_c)) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  if (param_1[7] != 0) {
    param_1[0x1d] = *(int *)(param_1[7] + 0x28);
    param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
    param_1[0x1e] = (int)(*(float *)(param_1[7] + 0x30) + (float)param_1[0x43]);
  }
  if (param_1[0x42] == 1) {
    local_10 = 0;
  }
  local_8 = 0;
  if (param_1[0x3c] != 0) {
    param_1[0x44] = (int)((float)local_c / (float)param_1[0x3c]);
  }
  if (param_1[0x3c] == 0) {
    if (param_1[0x1b] == 1) {
      iVar2 = __ftol();
      iVar3 = __ftol();
      param_1[0x44] =
           (int)((float)((((uint)param_1 % 10) * 300 + iVar3 + uVar1 + iVar2) % 1000) /
                _DAT_005a43c0);
    }
    else {
      param_1[0x44] = 0x3e99999a;
    }
  }
  if (param_1[0x41] == 1) {
    fVar4 = (float10)FUN_00423b00((float)param_1[0x44] * _DAT_005a4148);
    if (fVar4 <= (float10)_DAT_005a3534) {
      fVar4 = (float10)FUN_00423b00((float)param_1[0x44] * _DAT_005a4148);
      fVar4 = -fVar4;
    }
    else {
      fVar4 = (float10)FUN_00423b00((float)param_1[0x44] * _DAT_005a4148);
    }
    local_104 = (float)fVar4;
    local_28 = local_104;
    local_14 = __ftol();
    local_1c = __ftol();
    local_24 = __ftol();
    local_18 = __ftol();
    for (local_20 = 0; local_20 < 4; local_20 = local_20 + 1) {
      param_1[local_20 * 6 + 0x23] = local_14 << 0x18 | local_1c << 0x10 | local_24 << 8 | local_18;
    }
  }
  else if (param_1[0x41] == 2) {
    local_44 = (uint)param_1 % 100;
    local_2c = (float)((uVar1 + local_44 * 200) % 5000) / _DAT_005a4820;
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
      param_1[local_3c * 6 + 0x23] = local_30 << 0x18 | local_38 << 0x10 | local_40 << 8 | local_34;
    }
  }
  else if ((param_1[0x41] == 3) || (param_1[0x41] == 4)) {
    local_60 = (uint)param_1 % 100;
    local_48 = (float)((uVar1 + local_60 * 200) % 5000) / _DAT_005a4820;
    fVar4 = (float10)FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    if (fVar4 <= (float10)_DAT_005a3534) {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    else {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    local_4c = __ftol();
    fVar4 = (float10)FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    if (fVar4 <= (float10)_DAT_005a3534) {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    else {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    local_54 = __ftol();
    fVar4 = (float10)FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    if (fVar4 <= (float10)_DAT_005a3534) {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    else {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    local_5c = __ftol();
    fVar4 = (float10)FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    if (fVar4 <= (float10)_DAT_005a3534) {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    else {
      FUN_00423b00(local_48 * _DAT_005a4148 + local_48 * _DAT_005a4148);
    }
    local_50 = __ftol();
    for (local_58 = 0; local_58 < 4; local_58 = local_58 + 1) {
      param_1[local_58 * 6 + 0x23] = local_4c << 0x18 | local_54 << 0x10 | local_5c << 8 | local_50;
    }
  }
  if (param_1[0x3e] != 0) {
    param_1[0x40] =
         (int)(((float)(uVar1 % (uint)param_1[0x3e]) / (float)param_1[0x3e]) * _DAT_005a4290);
  }
  iVar2 = (**(code **)(*param_1 + 0x38))();
  if (iVar2 == 1) {
    FUN_00430f10();
    FUN_00430f10();
    local_ac = (float)param_1[0x38];
    local_a8 = (float)param_1[0x39];
    local_a4 = (float)param_1[0x3a];
    if (param_1[0x3c] == 0) {
      if (param_1[0x1b] == 1) {
        local_ac = (float)param_1[0x44] * _DAT_005a43cc * (float)param_1[0x3b] +
                   (float)param_1[0x38];
        local_a8 = (float)param_1[0x44] * _DAT_005a43cc * (float)param_1[0x3b] +
                   (float)param_1[0x39];
        local_a4 = (float)param_1[0x44] * _DAT_005a43cc * (float)param_1[0x3b] +
                   (float)param_1[0x3a];
      }
    }
    else {
      local_ac = (float)(uVar1 - param_1[0x3d]) * (float)param_1[0x3b] + (float)param_1[0x38];
      local_a8 = (float)(uVar1 - param_1[0x3d]) * (float)param_1[0x3b] + (float)param_1[0x39];
      local_a4 = (float)(uVar1 - param_1[0x3d]) * (float)param_1[0x3b] + (float)param_1[0x3a];
    }
    FUN_00430fc0(param_1 + 0xb);
    thunk_FUN_0056230d(local_ec,local_ac,local_a8,local_a4);
    thunk_FUN_0056239d(local_a0,param_1[0x1d],param_1[0x1e],param_1[0x1f]);
    thunk_FUN_00562821(param_1 + 0xb,_DAT_005a437c + (float)param_1[0x40],local_8,0);
    thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_ec);
    thunk_FUN_00561a5b(param_1 + 0xb,param_1 + 0xb,local_a0);
  }
  return 1;
}

