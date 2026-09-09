// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004efbdd | Name: FUN_004efbdd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004efbdd(int param_1,undefined4 *param_2)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  float10 fVar8;
  float10 fVar9;
  int local_54;
  int local_50;
  int local_48;
  int local_40;
  float local_3c;
  float local_34;
  int local_30;
  float local_2c;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  float local_18;
  int local_10;
  int local_c;
  undefined4 local_8;
  
  if ((((*(int *)(param_1 + 0x68) != 0) &&
       (iVar4 = (**(code **)(*DAT_0092e654 + 8))(), 1000 < (uint)(iVar4 - *(int *)(param_1 + 0x68)))
       ) && (*(int *)(param_1 + 0x6c) != 10)) ||
     (((*(int *)(param_1 + 0x68) != 0 &&
       (iVar4 = (**(code **)(*DAT_0092e654 + 8))(), 5000 < (uint)(iVar4 - *(int *)(param_1 + 0x68)))
       ) && (*(int *)(param_1 + 0x6c) == 10)))) {
    *(undefined4 *)(param_1 + 0x68) = 0;
    *(undefined4 *)(param_1 + 0x6c) = 0;
    *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_1 + 0x70);
    *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x74);
  }
  fVar1 = _DAT_005a3534 + *(float *)(param_1 + 0x28);
  local_18 = 0.0;
  if (*(int *)(param_1 + 0x8c) == 0) {
    if (*(int *)(param_1 + 0x54) != 0) {
      *(float *)(param_1 + 0x78) =
           *(float *)(*(int *)(param_1 + 0x54) + 0x28) - *(float *)(param_1 + 0x44);
      *(undefined4 *)(param_1 + 0x7c) = *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x2c);
      local_18 = *(float *)(*(int *)(param_1 + 0x54) + 0x30);
    }
    fVar2 = *(float *)(param_1 + 0x78) - _DAT_0092e674;
    fVar8 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x2c));
    fVar9 = (float10)FUN_00493bf0(fVar1);
    *(float *)(param_1 + 0x48) =
         (float)((float10)fVar2 -
                fVar9 * (float10)(float)fVar8 * (float10)*(float *)(param_1 + 0x34) *
                (float10)_DAT_005a4678);
    fVar2 = _DAT_0092e674 + *(float *)(param_1 + 0x7c);
    fVar8 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x2c));
    fVar9 = (float10)FUN_00423b00(fVar1);
    *(float *)(param_1 + 0x50) =
         (float)((float10)fVar2 -
                fVar9 * (float10)(float)fVar8 * (float10)*(float *)(param_1 + 0x34) *
                (float10)_DAT_005a4678);
    fVar8 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x2c));
    *(float *)(param_1 + 0x4c) =
         (float)((-fVar8 * (float10)*(float *)(param_1 + 0x34) + (float10)*(float *)(param_1 + 0x30)
                  + (float10)local_18 + (float10)*(float *)(param_1 + 0x3c)) -
                (float10)_DAT_005a365c);
  }
  if (1 < *(int *)(param_1 + 0x8c)) {
    fVar2 = *(float *)(param_1 + 0x78) - _DAT_0092e674;
    fVar8 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x2c));
    fVar9 = (float10)FUN_00493bf0(fVar1);
    *(float *)(param_1 + 0x48) =
         (float)((float10)fVar2 -
                fVar9 * (float10)(float)fVar8 * (float10)*(float *)(param_1 + 0x34) *
                (float10)_DAT_005a4678);
    fVar2 = _DAT_0092e674 + *(float *)(param_1 + 0x7c);
    fVar8 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x2c));
    fVar9 = (float10)FUN_00423b00(fVar1);
    *(float *)(param_1 + 0x50) =
         (float)((float10)fVar2 -
                fVar9 * (float10)(float)fVar8 * (float10)*(float *)(param_1 + 0x34) *
                (float10)_DAT_005a4678);
    fVar8 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x2c));
    *(float *)(param_1 + 0x4c) =
         (float)((-fVar8 * (float10)*(float *)(param_1 + 0x34) + (float10)*(float *)(param_1 + 0x30)
                  + (float10)local_18 + (float10)*(float *)(param_1 + 0x3c)) -
                (float10)_DAT_005a365c);
  }
  local_2c = *(float *)(param_1 + 0x34);
  local_3c = *(float *)(param_1 + 0x48);
  local_34 = *(float *)(param_1 + 0x50);
  if ((*(int *)(param_1 + 0x54) != 0) && (*(int *)(param_1 + 0x8c) == 0)) {
    local_3c = *(float *)(*(int *)(param_1 + 0x54) + 0x28) * _DAT_005a414c +
               local_3c * _DAT_005a4154;
    local_34 = *(float *)(*(int *)(param_1 + 0x54) + 0x2c) * _DAT_005a414c +
               local_34 * _DAT_005a4154;
  }
  if (_DAT_005a45ec < *(float *)(param_1 + 0x2c)) {
    local_2c = 11.0;
  }
  iVar4 = __ftol();
  uVar5 = (iVar4 + -1) / 2 & 0x80000001;
  if ((int)uVar5 < 0) {
    uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
  }
  *(uint *)(param_1 + 0x94) = uVar5;
  if (iVar4 == 0) {
    *(undefined4 *)(param_1 + 0x94) = 1;
  }
  local_30 = __ftol();
  local_50 = __ftol();
  local_40 = __ftol();
  local_54 = __ftol();
  if ((iVar4 == 1) || (iVar4 == 2)) {
    *(float *)(param_1 + 0x98) = local_3c - _DAT_005a3834;
    *(float *)(param_1 + 0xa0) = local_2c * _DAT_005a415c + local_3c;
    *(float *)(param_1 + 0x9c) = local_34 - _DAT_005a3834;
    *(float *)(param_1 + 0xa4) = local_2c * _DAT_005a415c + local_34;
    local_30 = __ftol();
    local_40 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  else if ((iVar4 == 5) || (iVar4 == 6)) {
    *(float *)(param_1 + 0x98) = local_3c - local_2c * _DAT_005a415c;
    *(float *)(param_1 + 0xa0) = local_3c + _DAT_005a3834;
    *(float *)(param_1 + 0x9c) = local_34 - _DAT_005a3834;
    *(float *)(param_1 + 0xa4) = local_2c * _DAT_005a415c + local_34;
    local_30 = __ftol();
    local_40 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  else if ((iVar4 == 9) || (iVar4 == 10)) {
    *(float *)(param_1 + 0x98) = local_3c - local_2c * _DAT_005a415c;
    *(float *)(param_1 + 0xa0) = local_3c;
    *(float *)(param_1 + 0x9c) = local_34 - local_2c * _DAT_005a415c;
    *(float *)(param_1 + 0xa4) = local_34 + _DAT_005a3834;
    local_30 = __ftol();
    local_40 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  else if ((iVar4 == 0xd) || (iVar4 == 0xe)) {
    *(float *)(param_1 + 0x98) = local_3c - _DAT_005a3834;
    *(float *)(param_1 + 0xa0) = local_2c * _DAT_005a415c + local_3c;
    *(float *)(param_1 + 0x9c) = local_34 - local_2c * _DAT_005a415c;
    *(float *)(param_1 + 0xa4) = local_34 + _DAT_005a3834;
    local_30 = __ftol();
    local_40 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  if ((iVar4 == 0) || (iVar4 == 0xf)) {
    *(float *)(param_1 + 0xa8) = local_2c * _DAT_005a430c + local_3c;
    *(float *)(param_1 + 0xac) = local_34;
    local_30 = __ftol();
    local_40 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  else if ((iVar4 == 3) || (iVar4 == 4)) {
    *(float *)(param_1 + 0xa8) = local_3c;
    *(float *)(param_1 + 0xac) = local_2c * _DAT_005a430c + local_34;
    local_30 = __ftol();
    local_40 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  else if ((iVar4 == 7) || (iVar4 == 8)) {
    *(float *)(param_1 + 0xa8) = local_3c - local_2c * _DAT_005a430c;
    *(float *)(param_1 + 0xac) = local_34;
    local_40 = __ftol();
    local_30 = __ftol();
    local_50 = __ftol();
    local_54 = __ftol();
  }
  else if ((iVar4 == 0xb) || (iVar4 == 0xc)) {
    *(float *)(param_1 + 0xa8) = local_3c;
    *(float *)(param_1 + 0xac) = local_34 - local_2c * _DAT_005a430c;
    local_30 = __ftol();
    local_40 = __ftol();
    local_54 = __ftol();
    local_50 = __ftol();
  }
  if (local_30 < 0) {
    local_30 = 0;
  }
  if (local_40 < 0) {
    local_40 = 0;
  }
  if (local_50 < 0) {
    local_50 = 0;
  }
  if (local_54 < 0) {
    local_54 = 0;
  }
  iVar7 = local_30 >> 5;
  local_10 = local_40 >> 5;
  iVar4 = local_50 >> 5;
  local_1c = local_54 >> 5;
  if (iVar7 == local_10) {
    if (local_40 < local_30) {
      local_10 = iVar7 + -1;
    }
    else {
      local_10 = iVar7 + 1;
    }
  }
  if (iVar4 == local_1c) {
    if (local_54 < local_50) {
      local_1c = iVar4 + -1;
    }
    else {
      local_1c = iVar4 + 1;
    }
  }
  local_48 = iVar7;
  local_c = iVar4;
  if (*(int *)(param_1 + 0x54) != 0) {
    iVar6 = __ftol();
    if ((iVar6 >> 7 == 0x1f) && (iVar6 = __ftol(), iVar6 >> 7 == 0x1f)) {
      bVar3 = true;
    }
    else {
      bVar3 = false;
    }
    if (bVar3) {
      bVar3 = true;
    }
    else {
      iVar6 = __ftol();
      if ((((iVar6 >> 7 < 0x1b) || (iVar6 = __ftol(), 0x1e < iVar6 >> 7)) ||
          (iVar6 = __ftol(), iVar6 >> 7 < 0x15)) || (iVar6 = __ftol(), 0x18 < iVar6 >> 7)) {
        bVar3 = false;
      }
      else {
        bVar3 = true;
      }
    }
    if ((bVar3) && (_DAT_005a5360 < *(float *)(param_1 + 0x2c))) {
      local_48 = local_10 * 2 - iVar7;
      local_c = local_1c * 2 - iVar4;
    }
  }
  DAT_005bdef0 = local_48 + local_c * 0x10000;
  DAT_005bdef4 = local_48 + local_1c * 0x10000;
  DAT_005bdef8 = local_10 + local_c * 0x10000;
  DAT_005bdefc = local_10 + local_1c * 0x10000;
  DAT_005bdf00 = local_48 + iVar4 * 0x10000;
  DAT_005bdf04 = local_10 + iVar4 * 0x10000;
  DAT_005bdf08 = iVar7 + local_c * 0x10000;
  DAT_005bdf0c = iVar7 + local_1c * 0x10000;
  DAT_005bdf10 = iVar7 + iVar4 * 0x10000;
  FUN_00430f20(*(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x50));
  local_8 = 0;
  if (DAT_0067cf38 != 0) {
    iVar4 = FUN_0049cf38(local_24,local_20,*(undefined4 *)(param_1 + 0x4c),&local_8);
    if (iVar4 == 1) {
      *(undefined4 *)(param_1 + 0x90) = 1;
    }
    else {
      *(undefined4 *)(param_1 + 0x90) = 0;
    }
    if (*(int *)(DAT_0067cf38 + 0x4c) != 0) {
      iVar4 = *(int *)(DAT_0067cf38 + 0x4c);
      if (*(char *)(iVar4 + 0x79c) == '\x01') {
        *(undefined4 *)(param_1 + 0xbc) = 0x40200000;
        if (((*(int *)(iVar4 + 0x7a0) == 0x27) || (*(int *)(iVar4 + 0x7a0) == 0x28)) ||
           ((*(int *)(iVar4 + 0x7a0) == 0x14 && (*(short *)(iVar4 + 0x1a0) != 3)))) {
          *(undefined4 *)(param_1 + 0xbc) = 0x40966666;
        }
        else if (*(int *)(iVar4 + 0x7a0) == 0x26) {
          *(undefined4 *)(param_1 + 0xbc) = 0x40600000;
        }
        *(float *)(param_1 + 0xbc) =
             (float)(int)*(short *)(iVar4 + 0x45a) * _DAT_005a41ec + *(float *)(param_1 + 0xbc);
      }
      else {
        *(undefined4 *)(param_1 + 0xbc) = 0x3fc00000;
      }
    }
  }
  *param_2 = *(undefined4 *)(param_1 + 0x48);
  param_2[1] = *(undefined4 *)(param_1 + 0x4c);
  param_2[2] = *(undefined4 *)(param_1 + 0x50);
  return param_2;
}

