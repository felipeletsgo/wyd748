// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d8587 | Name: FUN_004d8587


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d8587(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined1 local_64 [48];
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  float local_18;
  float local_14;
  undefined4 local_10;
  float local_c;
  int local_8;
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  if ((param_1[0x2f] == 0) || ((uint)(iVar1 - param_1[0x2e]) <= (uint)param_1[0x2f])) {
    CSimpleArray<>();
    CSimpleArray<>();
    local_24 = 0.0;
    if (param_1[7] != 0) {
      uVar2 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),
                           *(float *)(param_1[7] + 0x30) + _DAT_005a3660,
                           *(undefined4 *)(param_1[7] + 0x2c));
      FUN_004310a0(uVar2);
    }
    FUN_004310a0(param_1 + 0x30);
    for (local_8 = 0; local_8 < param_1[0x36]; local_8 = local_8 + 1) {
      local_24 = (float)(local_8 + 1) / (float)param_1[0x36];
      uVar2 = FUN_00430f50(local_24 * (float)param_1[0x33] +
                           (_DAT_005a3660 - local_24) * (float)param_1[0x30],
                           local_24 * (float)param_1[0x34] +
                           (_DAT_005a3660 - local_24) * (float)param_1[0x31],
                           local_24 * (float)param_1[0x35] +
                           (_DAT_005a3660 - local_24) * (float)param_1[0x32]);
      FUN_004310a0(uVar2);
      if (local_8 < param_1[0x36] + -1) {
        local_34 = (float)(param_1[0x36] - local_8) * _DAT_005a442c;
        if (_DAT_005a40ec < local_34) {
          local_34 = 0.5;
        }
        iVar1 = _rand();
        local_30 = (float)(iVar1 % 9 + -5) * local_34 * (float)param_1[0x21];
        iVar1 = _rand();
        local_28 = (float)(iVar1 % 9 + -5) * local_34 * (float)param_1[0x21];
        iVar1 = _rand();
        local_2c = (float)(iVar1 % 9 + -5) * local_34 * (float)param_1[0x21];
        uVar2 = FUN_00430f50(local_30,local_28,local_2c);
        uVar2 = FUN_00493d50(local_64,uVar2);
        FUN_004310a0(uVar2);
      }
      if (((param_1[local_8 + 0x22] != 0) &&
          (FUN_004ccfed(local_20,local_1c,local_18,local_14,local_10,local_c), param_1[7] != 0)) &&
         (param_1[local_8 + 0x27] != 0)) {
        puVar3 = (undefined4 *)
                 FUN_00430f20((local_20 + local_14) / _DAT_005a365c,
                              (local_18 + local_c) / _DAT_005a365c);
        (**(code **)(*(int *)param_1[local_8 + 0x27] + 0x3c))(*puVar3,puVar3[1]);
      }
      FUN_004310a0(&local_14);
    }
  }
  else {
    param_1[10] = 0;
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  return 1;
}

