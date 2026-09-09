// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f6cde | Name: FUN_004f6cde


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_004f6cde(int param_1)

{
  int iVar1;
  undefined1 local_9c [12];
  undefined1 local_90 [12];
  undefined1 local_84 [12];
  undefined1 local_78 [12];
  undefined1 local_6c [12];
  undefined1 local_60 [12];
  undefined1 local_54 [12];
  undefined1 local_48 [12];
  float local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  undefined1 local_28 [12];
  int local_1c;
  undefined1 local_18 [12];
  int local_c;
  float local_8;
  
  if (*(short *)(param_1 + 0x160) == 0) {
    *(undefined4 *)(param_1 + 0x1b4) = 0;
    local_2c = 0;
  }
  else {
    FUN_00430f10();
    FUN_00430f10();
    (**(code **)(*DAT_005ccf9c + 0x50))(local_18,local_28);
    iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
    local_38 = *(undefined4 *)(iVar1 + 0x48);
    local_34 = *(undefined4 *)(iVar1 + 0x4c);
    local_30 = *(undefined4 *)(iVar1 + 0x50);
    local_1c = *(int *)(DAT_0067cf38 + 0x80);
    local_c = *(int *)(DAT_0067cf38 + 0x4c);
    local_2c = 0;
    local_8 = 0.6;
    local_3c = 0.7;
    if (*(int *)(param_1 + 0x24) == 0x195) {
      *(undefined4 *)(param_1 + 0x1b4) = 0;
      local_2c = 0;
    }
    else {
      if (*(int *)(param_1 + 0x24) == 0x647) {
        local_8 = 1.0;
      }
      if ((0xc48 < *(short *)(param_1 + 0x160)) && (*(short *)(param_1 + 0x160) < 0xc4e)) {
        local_8 = 3.0;
        local_3c = 3.0;
      }
      FUN_00430f50(*(float *)(param_1 + 0x28) - local_8,local_3c + *(float *)(param_1 + 0x30),
                   *(undefined4 *)(param_1 + 0x2c));
      FUN_00430f50(local_8 + *(float *)(param_1 + 0x28),local_3c + *(float *)(param_1 + 0x30),
                   *(undefined4 *)(param_1 + 0x2c));
      FUN_00430f50(*(float *)(param_1 + 0x28) - local_8,*(float *)(param_1 + 0x30) + _DAT_005a3534,
                   *(undefined4 *)(param_1 + 0x2c));
      FUN_00430f50(local_8 + *(float *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3534,
                   *(undefined4 *)(param_1 + 0x2c));
      iVar1 = FUN_005649c7(local_6c,local_54,local_48,local_18,local_28,0,0,0);
      if (iVar1 == 1) {
        if (local_c == 0) {
          *(int *)(DAT_0067cf38 + 0x80) = param_1;
        }
        local_2c = 1;
      }
      if (local_2c == 0) {
        iVar1 = FUN_005649c7(local_54,local_60,local_48,local_18,local_28,0,0,0);
        if (iVar1 == 1) {
          if (local_c == 0) {
            *(int *)(DAT_0067cf38 + 0x80) = param_1;
          }
          local_2c = 1;
        }
      }
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),local_3c + *(float *)(param_1 + 0x30),
                   *(float *)(param_1 + 0x2c) - local_8);
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),local_3c + *(float *)(param_1 + 0x30),
                   local_8 + *(float *)(param_1 + 0x2c));
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3534,
                   *(float *)(param_1 + 0x2c) - local_8);
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3534,
                   local_8 + *(float *)(param_1 + 0x2c));
      if (local_2c == 0) {
        iVar1 = FUN_005649c7(local_9c,local_84,local_78,local_18,local_28,0,0,0);
        if (iVar1 == 1) {
          if (local_c == 0) {
            *(int *)(DAT_0067cf38 + 0x80) = param_1;
          }
          local_2c = 1;
        }
      }
      if (local_2c == 0) {
        iVar1 = FUN_005649c7(local_84,local_90,local_78,local_18,local_28,0,0,0);
        if (iVar1 == 1) {
          if (local_c == 0) {
            *(int *)(DAT_0067cf38 + 0x80) = param_1;
          }
          local_2c = 1;
        }
      }
      if (((local_c != 0) && (local_2c == 1)) && (local_1c == 0)) {
        *(int *)(DAT_0067cf38 + 0x80) = param_1;
      }
      *(int *)(param_1 + 0x1b4) = local_2c;
    }
  }
  return local_2c;
}

