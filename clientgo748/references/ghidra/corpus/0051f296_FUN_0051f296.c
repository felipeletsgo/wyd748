// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051f296 | Name: FUN_0051f296


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_0051f296(int param_1)

{
  int iVar1;
  float10 fVar2;
  float10 fVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined1 local_cc [12];
  undefined1 local_c0 [12];
  undefined1 local_b4 [12];
  undefined1 local_a8 [12];
  undefined1 local_9c [12];
  undefined1 local_90 [12];
  undefined1 local_84 [12];
  undefined1 local_78 [12];
  undefined1 local_6c [12];
  undefined1 local_60 [12];
  undefined1 local_54 [12];
  undefined1 local_48 [12];
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  undefined1 local_2c [12];
  int local_20;
  undefined1 local_1c [12];
  int local_10;
  float local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x214) != 0) {
    return 0;
  }
  if (*(int *)(DAT_0067cf38 + 0x24) == 0x7534) {
    *(undefined4 *)(param_1 + 0x238) = 0;
    return 0;
  }
  if (*(int *)(DAT_0067cf38 + 0x24) == 0x7535) {
    *(undefined4 *)(param_1 + 0x238) = 0;
    return 0;
  }
  if (*(char *)(param_1 + 0x795) == '\x01') {
    *(undefined4 *)(param_1 + 0x238) = 0;
    return 0;
  }
  if ((*(int *)(DAT_0067cf38 + 0x24) == 30000) &&
     (*(char *)(DAT_013b71e8 + 0xef6 + (int)*(char *)(DAT_013b71e8 + 0xef5)) == '\x1f')) {
    if (((*(int *)(param_1 + 0x740) != 0xc) && (*(int *)(param_1 + 0x740) != 0xb)) &&
       (*(char *)(param_1 + 0x245) != '\x01')) {
      if (*(int *)(DAT_0067cf38 + 0x7c) == param_1) {
        *(undefined4 *)(DAT_0067cf38 + 0x7c) = 0;
      }
      *(undefined4 *)(param_1 + 0x238) = 0;
      return 0;
    }
  }
  else if ((*(int *)(param_1 + 0x740) == 0xc) ||
          ((*(int *)(param_1 + 0x740) == 0xb || (*(char *)(param_1 + 0x245) == '\x01')))) {
    if (*(int *)(DAT_0067cf38 + 0x7c) == param_1) {
      *(undefined4 *)(DAT_0067cf38 + 0x7c) = 0;
    }
    *(undefined4 *)(param_1 + 0x238) = 0;
    return 0;
  }
  FUN_00430f10();
  FUN_00430f10();
  (**(code **)(*DAT_005ccf9c + 0x50))(local_1c,local_2c);
  iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
  local_3c = *(undefined4 *)(iVar1 + 0x48);
  local_38 = *(undefined4 *)(iVar1 + 0x4c);
  local_34 = *(undefined4 *)(iVar1 + 0x50);
  local_20 = *(int *)(DAT_0067cf38 + 0x7c);
  local_10 = *(int *)(DAT_0067cf38 + 0x4c);
  local_30 = 0;
  local_8 = *(int *)(param_1 + 0x5c);
  if ('\0' < *(char *)(param_1 + 0x79c)) {
    local_8 = *(int *)(param_1 + 0x7a0);
  }
  local_c = *(float *)(&DAT_0092e7d0 + local_8 * 8) * *(float *)(param_1 + 0x38) + _DAT_005a4158;
  iVar1 = FUN_004ff3d0();
  if (iVar1 == 1) {
    local_c = local_c - _DAT_005a4158;
  }
  FUN_00430f50(*(float *)(param_1 + 0x28) - local_c,
               *(float *)(&DAT_0092e7d4 + local_8 * 8) * *(float *)(param_1 + 0x38) +
               *(float *)(param_1 + 0x30),*(undefined4 *)(param_1 + 0x2c));
  FUN_00430f50(local_c + *(float *)(param_1 + 0x28),
               *(float *)(&DAT_0092e7d4 + local_8 * 8) * *(float *)(param_1 + 0x38) +
               *(float *)(param_1 + 0x30),*(undefined4 *)(param_1 + 0x2c));
  FUN_00430f50(*(float *)(param_1 + 0x28) - local_c,*(undefined4 *)(param_1 + 0x30),
               *(undefined4 *)(param_1 + 0x2c));
  FUN_00430f50(local_c + *(float *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
               *(undefined4 *)(param_1 + 0x2c));
  iVar1 = FUN_005649c7(local_6c,local_54,local_48,local_1c,local_2c,0,0,0);
  if (iVar1 == 1) {
    if (local_10 == 0) {
      *(int *)(DAT_0067cf38 + 0x7c) = param_1;
      *(undefined4 *)(param_1 + 0x238) = 1;
    }
    local_30 = 1;
  }
  if ((local_30 == 0) &&
     (iVar1 = FUN_005649c7(local_54,local_60,local_48,local_1c,local_2c,0,0,0), iVar1 == 1)) {
    if (local_10 == 0) {
      *(int *)(DAT_0067cf38 + 0x7c) = param_1;
      *(undefined4 *)(param_1 + 0x238) = 1;
    }
    local_30 = 1;
  }
  if (local_30 == 0) {
    FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                 *(float *)(&DAT_0092e7d4 + local_8 * 8) * *(float *)(param_1 + 0x38) +
                 *(float *)(param_1 + 0x30),*(float *)(param_1 + 0x2c) - local_c);
    FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                 *(float *)(&DAT_0092e7d4 + local_8 * 8) * *(float *)(param_1 + 0x38) +
                 *(float *)(param_1 + 0x30),local_c + *(float *)(param_1 + 0x2c));
    FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(float *)(param_1 + 0x2c) - local_c);
    FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 local_c + *(float *)(param_1 + 0x2c));
    if ((local_30 == 0) &&
       (iVar1 = FUN_005649c7(local_9c,local_84,local_78,local_1c,local_2c,0,0,0), iVar1 == 1)) {
      if (local_10 == 0) {
        *(int *)(DAT_0067cf38 + 0x7c) = param_1;
        *(undefined4 *)(param_1 + 0x238) = 1;
      }
      local_30 = 1;
    }
    if ((local_30 == 0) &&
       (iVar1 = FUN_005649c7(local_84,local_90,local_78,local_1c,local_2c,0,0,0), iVar1 == 1)) {
      if (local_10 == 0) {
        *(int *)(DAT_0067cf38 + 0x7c) = param_1;
        *(undefined4 *)(param_1 + 0x238) = 1;
      }
      local_30 = 1;
    }
  }
  if (((local_30 == 0) && (*(int *)(param_1 + 0x24c) == 0x38)) && (*(short *)(param_1 + 0x1d2) == 0)
     ) {
    FUN_00430f50(*(float *)(param_1 + 0x28) - local_c,*(undefined4 *)(param_1 + 0x30),
                 *(float *)(param_1 + 0x2c) - local_c);
    FUN_00430f50(*(float *)(param_1 + 0x28) - local_c,*(undefined4 *)(param_1 + 0x30),
                 local_c + *(float *)(param_1 + 0x2c));
    FUN_00430f50(local_c + *(float *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(float *)(param_1 + 0x2c) - local_c);
    FUN_00430f50(local_c + *(float *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 local_c + *(float *)(param_1 + 0x2c));
    if ((local_30 == 0) &&
       (iVar1 = FUN_005649c7(local_cc,local_b4,local_a8,local_1c,local_2c,0,0,0), iVar1 == 1)) {
      if (local_10 == 0) {
        *(int *)(DAT_0067cf38 + 0x7c) = param_1;
        *(undefined4 *)(param_1 + 0x238) = 1;
      }
      local_30 = 1;
    }
    if ((local_30 == 0) &&
       (iVar1 = FUN_005649c7(local_b4,local_c0,local_a8,local_1c,local_2c,0,0,0), iVar1 == 1)) {
      if (local_10 == 0) {
        *(int *)(DAT_0067cf38 + 0x7c) = param_1;
        *(undefined4 *)(param_1 + 0x238) = 1;
      }
      local_30 = 1;
    }
  }
  if ((*(int *)(DAT_013b71e8 + 0x1b084) != 0) && (local_10 != param_1)) {
    if (local_30 == 1) {
      FUN_00430f20(local_3c,local_34);
      if (local_20 != 0) {
        fVar2 = (float10)FUN_004d0150(param_1 + 0x28);
        fVar3 = (float10)FUN_004d0150(local_20 + 0x28);
        if (fVar3 <= (float10)(float)fVar2) goto LAB_0051fad5;
      }
      if (*(int *)(DAT_0067cf38 + 0x7c) != 0) {
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x7c) + 0x238) = 0;
      }
      *(int *)(DAT_0067cf38 + 0x7c) = param_1;
      *(undefined4 *)(param_1 + 0x238) = 1;
    }
    else if (*(int *)(DAT_0067cf38 + 0x7c) == param_1) {
      *(undefined4 *)(DAT_0067cf38 + 0x7c) = 0;
      *(undefined4 *)(param_1 + 0x238) = 1;
    }
  }
LAB_0051fad5:
  if (local_30 == 1) {
    FUN_00430f20(local_3c,local_34);
    fVar2 = (float10)FUN_004d0150(param_1 + 0x28);
    if (_DAT_005a430c <= (float)fVar2) {
      if ((((local_20 != 0) && (local_10 != param_1)) && (*(uint *)(param_1 + 0x20) < 1000)) &&
         ((999 < *(uint *)(local_20 + 0x20) && (*(char *)(local_20 + 0x249) == '\x01')))) {
        if (*(int *)(DAT_0067cf38 + 0x7c) != 0) {
          *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x7c) + 0x238) = 0;
        }
        *(undefined4 *)(param_1 + 0x238) = 1;
        *(int *)(DAT_0067cf38 + 0x7c) = param_1;
      }
    }
    else {
      local_30 = 0;
      *(undefined4 *)(param_1 + 0x238) = 0;
      *(int *)(DAT_0067cf38 + 0x7c) = local_20;
    }
  }
  if (((*(int *)(param_1 + 0x238) == 0) && (local_30 == 1)) &&
     ((iVar1 = FUN_00494dcf(), iVar1 != 30000 &&
      ((DAT_005ccf98 != 0 && (iVar1 = FUN_00429a6d(0x34), iVar1 != 0)))))) {
    FUN_00429a6d(0x34);
    iVar1 = FUN_0042afd0();
    if (iVar1 == 0) {
      uVar5 = 0;
      uVar4 = 0;
      FUN_00429a6d(0x34);
      FUN_0042ad2b(uVar4,uVar5);
    }
  }
  *(int *)(param_1 + 0x238) = local_30;
  return local_30;
}

