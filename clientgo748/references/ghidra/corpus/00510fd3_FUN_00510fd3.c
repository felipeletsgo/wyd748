// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00510fd3 | Name: FUN_00510fd3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00510fd3(int param_1)

{
  void *pvVar1;
  undefined4 uVar2;
  int local_90;
  int local_8c;
  int local_88;
  int local_84;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  int local_34;
  int local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1ee7;
  local_10 = ExceptionList;
  if (*(char *)(param_1 + 0x7e2) == '\x01') {
    ExceptionList = &local_10;
    for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                   (float)local_14 * _DAT_005a34a0 + *(float *)(param_1 + 0x30),
                   *(undefined4 *)(param_1 + 0x2c));
      pvVar1 = operator_new(0x94);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_84 = 0;
      }
      else {
        local_84 = FUN_004c5cc0(local_24,local_20,local_1c,local_24,local_20,local_1c,0,0x2712,0,0,0
                               );
      }
      local_8 = 0xffffffff;
      if (local_84 == 0) {
        ExceptionList = local_10;
        return;
      }
      *(int *)(local_84 + 0x24) = *(int *)(local_84 + 0x24) + local_14 * 0x96;
      if ((DAT_0067cf38 != 0) && (local_84 != 0)) {
        FUN_0054ac09(local_84);
      }
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x02') {
    ExceptionList = &local_10;
    for (local_28 = 0; local_28 < 0xf; local_28 = local_28 + 1) {
      pvVar1 = operator_new(0xcc);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_88 = 0;
      }
      else {
        local_88 = FUN_004d2230(0xb16,0x33555555,*(undefined4 *)(param_1 + 0x34),3);
      }
      local_8 = 0xffffffff;
      if (local_88 == 0) {
        ExceptionList = local_10;
        return;
      }
      *(undefined4 *)(local_88 + 0xa4) = 0x19d;
      *(int *)(local_88 + 0xb0) = local_28 * 0x1e + 1000;
      *(int *)(local_88 + 0xb8) = local_28 * -0x1e + 1000;
      if (*(char *)(param_1 + 0x79c) == '\x01') {
        uVar2 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                             ((*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                              *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                             *(float *)(param_1 + 0x54)) - _DAT_005a40ec,
                             *(undefined4 *)(param_1 + 0x58));
        FUN_004310a0(uVar2);
      }
      else {
        uVar2 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                             (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                             *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                             *(float *)(param_1 + 0x30) + _DAT_005a40ec,
                             *(undefined4 *)(param_1 + 0x2c));
        FUN_004310a0(uVar2);
      }
      *(float *)(local_88 + 0x98) = (float)local_28 * _DAT_005a40ec + _DAT_005a3660;
      *(float *)(local_88 + 0x9c) = (float)local_28 * _DAT_005a40ec + _DAT_005a3660;
      *(undefined4 *)(local_88 + 0x6c) = 1;
      *(undefined1 *)(local_88 + 0xac) = 1;
      FUN_0054ac09(local_88);
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x03') {
    ExceptionList = &local_10;
    FUN_00430f50(*(undefined4 *)(param_1 + 0x4c0),*(undefined4 *)(param_1 + 0x4c4),
                 *(undefined4 *)(param_1 + 0x4c8));
    for (local_34 = 0; local_34 < 3; local_34 = local_34 + 1) {
      pvVar1 = operator_new(0xb0);
      local_8 = 2;
      if (pvVar1 == (void *)0x0) {
        local_8c = 0;
      }
      else {
        local_8c = FUN_004e3ba0(local_40,local_3c,local_38,1,0,1);
      }
      local_8 = 0xffffffff;
      if (local_8c == 0) {
        ExceptionList = local_10;
        return;
      }
      if (local_8c != 0) {
        FUN_0054ac09(local_8c);
      }
      *(int *)(local_8c + 0x84) = local_34 * 500 + 1000;
    }
    pvVar1 = operator_new(0xb0);
    local_8 = 3;
    if (pvVar1 == (void *)0x0) {
      local_90 = 0;
    }
    else {
      local_90 = FUN_004e3ba0(local_40,local_3c,local_38,1,0,1);
    }
    local_8 = 0xffffffff;
    if (local_90 != 0) {
      FUN_0054ac09(local_90);
      *(undefined4 *)(local_90 + 0x84) = 0x5dc;
    }
  }
  ExceptionList = local_10;
  return;
}

