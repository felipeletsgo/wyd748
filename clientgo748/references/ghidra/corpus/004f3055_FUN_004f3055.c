// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f3055 | Name: FUN_004f3055


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_004f3055(int param_1)

{
  int iVar1;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  undefined1 local_a4 [12];
  undefined1 local_98 [12];
  undefined1 local_8c [12];
  undefined1 local_80 [12];
  undefined1 local_74 [12];
  undefined1 local_68 [12];
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  float local_44;
  float local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  int local_2c;
  undefined1 local_28 [12];
  int local_1c;
  float local_18;
  undefined1 local_14 [12];
  int local_8;
  
  if ((0xc48 < *(short *)(param_1 + 0x160)) && (*(short *)(param_1 + 0x160) < 0xc4e)) {
    iVar1 = FUN_004f6cde();
    return iVar1;
  }
  if ((0x3dc < *(int *)(param_1 + 0x2f8)) && (*(int *)(param_1 + 0x2f8) < 0x3e1)) {
    *(undefined4 *)(param_1 + 0x1b4) = 0;
    return 0;
  }
  if (*(int *)(param_1 + 0x2f8) == 0x6d5) {
    *(undefined4 *)(param_1 + 0x1b4) = 0;
    return 0;
  }
  if ((*(float *)(param_1 + 0x308) < (float)_DAT_005a54f0) &&
     ((float)_DAT_005a54e8 < *(float *)(param_1 + 0x308))) {
    *(undefined4 *)(param_1 + 0x1b4) = 0;
    return 0;
  }
  if ((*(int *)(param_1 + 0x2e8) != 3) && (*(int *)(param_1 + 0x2f8) != 0xae0)) {
    *(undefined4 *)(param_1 + 0x1b4) = 0;
    return 0;
  }
  FUN_00430f10();
  FUN_00430f10();
  (**(code **)(*DAT_005ccf9c + 0x50))(local_14,local_28);
  iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
  local_3c = *(undefined4 *)(iVar1 + 0x48);
  local_38 = *(undefined4 *)(iVar1 + 0x4c);
  local_34 = *(undefined4 *)(iVar1 + 0x50);
  local_1c = *(int *)(DAT_0067cf38 + 0x80);
  local_8 = *(int *)(DAT_0067cf38 + 0x4c);
  local_44 = 2.0;
  local_40 = 0.0;
  local_18 = 0.0;
  if (*(int *)(param_1 + 0x2f8) == 0x6d5) {
    local_44 = 6.0;
  }
  else if (*(int *)(param_1 + 0x2f8) == 0xae0) {
    local_18 = 1.3;
  }
  local_2c = 0;
  local_30 = __ftol();
  if ((local_30 == 0) || (local_30 == 2)) {
    FUN_00430f50(*(float *)(param_1 + 0x28) - *(float *)(param_1 + 0x304),
                 local_44 + *(float *)(param_1 + 0x30),local_40 + *(float *)(param_1 + 0x2c));
    FUN_00430f50(*(float *)(param_1 + 0x28) + *(float *)(param_1 + 0x308),
                 local_44 + *(float *)(param_1 + 0x30),local_40 + *(float *)(param_1 + 0x2c));
    FUN_00430f50(*(float *)(param_1 + 0x28) - *(float *)(param_1 + 0x304),
                 *(float *)(param_1 + 0x30) + _DAT_005a3534,local_40 + *(float *)(param_1 + 0x2c));
    FUN_00430f50(*(float *)(param_1 + 0x28) + *(float *)(param_1 + 0x308),
                 *(float *)(param_1 + 0x30) + _DAT_005a3534,local_40 + *(float *)(param_1 + 0x2c));
    iVar1 = FUN_005649c7(local_74,local_5c,local_50,local_14,local_28,0,0,0);
    if (iVar1 == 1) {
      if ((local_8 == 0) || (*(int *)(param_1 + 0x2f8) == 0xae0)) {
        *(int *)(DAT_0067cf38 + 0x80) = param_1;
      }
      local_2c = 1;
    }
    if ((local_2c == 0) &&
       (iVar1 = FUN_005649c7(local_5c,local_68,local_50,local_14,local_28,0,0,0), iVar1 == 1)) {
      if ((local_8 == 0) || (*(int *)(param_1 + 0x2f8) == 0xae0)) {
        *(int *)(DAT_0067cf38 + 0x80) = param_1;
      }
      local_2c = 1;
    }
  }
  else {
    FUN_00430f50(local_18 + *(float *)(param_1 + 0x28),local_44 + *(float *)(param_1 + 0x30),
                 *(float *)(param_1 + 0x2c) - *(float *)(param_1 + 0x30c));
    FUN_00430f50(local_18 + *(float *)(param_1 + 0x28),local_44 + *(float *)(param_1 + 0x30),
                 *(float *)(param_1 + 0x2c) + *(float *)(param_1 + 0x310));
    FUN_00430f50(local_18 + *(float *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3534,
                 *(float *)(param_1 + 0x2c) - *(float *)(param_1 + 0x30c));
    FUN_00430f50(local_18 + *(float *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3534,
                 *(float *)(param_1 + 0x2c) + *(float *)(param_1 + 0x310));
    if ((local_2c == 0) &&
       (iVar1 = FUN_005649c7(local_a4,local_8c,local_80,local_14,local_28,0,0,0), iVar1 == 1)) {
      if ((local_8 == 0) || (*(int *)(param_1 + 0x2f8) == 0xae0)) {
        *(int *)(DAT_0067cf38 + 0x80) = param_1;
      }
      local_2c = 1;
    }
    if ((local_2c == 0) &&
       (iVar1 = FUN_005649c7(local_8c,local_98,local_80,local_14,local_28,0,0,0), iVar1 == 1)) {
      if ((local_8 == 0) || (*(int *)(param_1 + 0x2f8) == 0xae0)) {
        *(int *)(DAT_0067cf38 + 0x80) = param_1;
      }
      local_2c = 1;
    }
  }
  if ((local_8 != 0) && (local_2c == 1)) {
    if (local_1c != 0) {
      if (*(float *)(local_1c + 0x2c) - *(float *)(local_8 + 0x2c) <= _DAT_005a3534) {
        local_ac = -(*(float *)(local_1c + 0x2c) - *(float *)(local_8 + 0x2c));
      }
      else {
        local_ac = *(float *)(local_1c + 0x2c) - *(float *)(local_8 + 0x2c);
      }
      if (*(float *)(param_1 + 0x2c) - *(float *)(local_8 + 0x2c) <= _DAT_005a3534) {
        local_b0 = -(*(float *)(param_1 + 0x2c) - *(float *)(local_8 + 0x2c));
      }
      else {
        local_b0 = *(float *)(param_1 + 0x2c) - *(float *)(local_8 + 0x2c);
      }
      if (local_ac < local_b0) goto LAB_004f3787;
    }
    if (*(float *)(param_1 + 0x28) - *(float *)(local_8 + 0x28) <= _DAT_005a3534) {
      local_b4 = -(*(float *)(param_1 + 0x28) - *(float *)(local_8 + 0x28));
    }
    else {
      local_b4 = *(float *)(param_1 + 0x28) - *(float *)(local_8 + 0x28);
    }
    if (local_b4 <= _DAT_005a365c) {
      if (*(float *)(param_1 + 0x2c) - *(float *)(local_8 + 0x2c) <= _DAT_005a3534) {
        local_b8 = -(*(float *)(param_1 + 0x2c) - *(float *)(local_8 + 0x2c));
      }
      else {
        local_b8 = *(float *)(param_1 + 0x2c) - *(float *)(local_8 + 0x2c);
      }
      if (local_b8 <= _DAT_005a365c) {
        *(int *)(DAT_0067cf38 + 0x80) = param_1;
      }
    }
  }
LAB_004f3787:
  *(int *)(param_1 + 0x1b4) = local_2c;
  return local_2c;
}

