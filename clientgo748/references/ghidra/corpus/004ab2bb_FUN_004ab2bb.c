// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ab2bb | Name: FUN_004ab2bb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004ab2bb(int param_1,int param_2,int param_3,uint param_4,int param_5)

{
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  uint local_18;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0;
  local_8 = 0xffffffff;
  if (param_5 == 1) {
    local_c = 0xffffffff;
    local_8 = 0;
  }
  if (param_2 == 0) {
    for (local_10 = 0; local_10 < 6; local_10 = local_10 + 1) {
      if (*(int *)(param_1 + 0x26e28 + local_10 * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x26e28 + local_10 * 4) + 0x94) = local_c;
      }
    }
    *(undefined4 *)(*(int *)(param_1 + 0x26e40) + 0x94) = local_c;
  }
  else if ((uint)(param_3 - param_2) < param_4) {
    local_18 = __ftol(param_4,0);
    if (DAT_005b892c == 2) {
      if (DAT_005c4958 == 0) {
        local_18 = 0xff;
      }
      local_18 = local_18 << 1;
      if (0xff < local_18) {
        local_18 = 0xff;
      }
      for (local_20 = 0; local_20 < 6; local_20 = local_20 + 1) {
        if (*(int *)(param_1 + 0x26e28 + local_20 * 4) != 0) {
          *(undefined **)(*(int *)(param_1 + 0x26e28 + local_20 * 4) + 0x94) = &DAT_01010101;
        }
      }
      for (local_20 = 0; local_20 < 10; local_20 = local_20 + 1) {
        if (*(int *)(param_1 + 0x26ec8 + local_20 * 4) != 0) {
          *(uint *)(*(int *)(param_1 + 0x26ec8 + local_20 * 4) + 0x94) = local_18 << 0x18 | 0xffffff
          ;
        }
      }
      for (local_20 = 0; local_20 < 3; local_20 = local_20 + 1) {
        *(uint *)(*(int *)(param_1 + 0x26ea4 + local_20 * 4) + 0x94) = local_18 << 0x18 | 0xffffff;
        *(uint *)(*(int *)(param_1 + 0x26eb0) + 0x94) = local_18 << 0x18 | 0xffffff;
      }
      *(undefined **)(*(int *)(param_1 + 0x26e40) + 0x94) = &DAT_01010101;
      *(undefined **)(*(int *)(param_1 + 0x26e5c) + 0x94) = &DAT_01010101;
      *(uint *)(*(int *)(param_1 + 0x26ea0) + 0x94) = local_18 << 0x18 | 0xffffff;
      *(uint *)(*(int *)(param_1 + 0x26e28) + 0x94) = local_18 << 0x18 | 0xffffff;
      *(uint *)(*(int *)(param_1 + 0x26e2c) + 0x94) = local_18 << 0x18 | 0xffffff;
      if (*(int *)(param_1 + 0x26e38) != 0) {
        *(uint *)(*(int *)(param_1 + 0x26e38) + 0x94) = local_18 << 0x18 | 0xffffff;
      }
      if (*(int *)(param_1 + 0x26e3c) != 0) {
        *(uint *)(*(int *)(param_1 + 0x26e3c) + 0x94) = local_18 << 0x18 | 0xffffff;
      }
    }
    else {
      for (local_24 = 0; local_24 < 6; local_24 = local_24 + 1) {
        if (*(int *)(param_1 + 0x26e28 + local_24 * 4) != 0) {
          *(uint *)(*(int *)(param_1 + 0x26e28 + local_24 * 4) + 0x94) = local_18 << 0x18 | 0xffffff
          ;
        }
        if (*(int *)(param_1 + 0x26ec8 + local_24 * 4) != 0) {
          *(uint *)(*(int *)(param_1 + 0x26ec8 + local_24 * 4) + 0x94) = local_18 << 0x18 | 0xffffff
          ;
        }
      }
      *(uint *)(*(int *)(param_1 + 0x26e40) + 0x94) = local_18 << 0x18 | 0xffffff;
    }
  }
  else if (DAT_005b892c == 2) {
    for (local_28 = 0; local_28 < 6; local_28 = local_28 + 1) {
      if (*(int *)(param_1 + 0x26e28 + local_28 * 4) != 0) {
        *(undefined **)(*(int *)(param_1 + 0x26e28 + local_28 * 4) + 0x94) = &DAT_01010101;
      }
      if (*(int *)(param_1 + 0x26ec8 + local_28 * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x26ec8 + local_28 * 4) + 0x94) = local_8;
      }
    }
    for (local_28 = 0; local_28 < 3; local_28 = local_28 + 1) {
      *(undefined4 *)(*(int *)(param_1 + 0x26ea4 + local_28 * 4) + 0x94) = local_8;
      *(undefined4 *)(*(int *)(param_1 + 0x26eb0) + 0x94) = local_8;
    }
    *(undefined4 *)(*(int *)(param_1 + 0x26ea0) + 0x94) = local_8;
    if (*(int *)(param_1 + 0x26e28) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x26e28) + 0x94) = local_8;
    }
    if (*(int *)(param_1 + 0x26e2c) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x26e2c) + 0x94) = local_8;
    }
    if (*(int *)(param_1 + 0x26e38) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x26e38) + 0x94) = local_8;
    }
    if (*(int *)(param_1 + 0x26e3c) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x26e3c) + 0x94) = local_8;
    }
  }
  else {
    for (local_2c = 0; local_2c < 6; local_2c = local_2c + 1) {
      if (*(int *)(param_1 + 0x26e28 + local_2c * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x26e28 + local_2c * 4) + 0x94) = local_8;
      }
      if (*(int *)(param_1 + 0x26ec8 + local_2c * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x26ec8 + local_2c * 4) + 0x94) = local_8;
      }
    }
    *(undefined4 *)(*(int *)(param_1 + 0x26e40) + 0x94) = local_8;
  }
  return;
}

