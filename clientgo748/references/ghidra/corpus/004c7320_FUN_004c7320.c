// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c7320 | Name: FUN_004c7320


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c7320(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_a0;
  undefined1 local_9c [12];
  float local_90 [27];
  int local_24;
  int local_20;
  undefined1 local_1c [8];
  float local_14;
  int local_10;
  int local_c;
  float local_8;
  
  local_24 = *(int *)(DAT_013b71e8 + 0x1b084);
  if (*(float *)(local_24 + 0x2c) < _DAT_005a45ec) {
    local_a0 = 0;
    local_8 = 1.0;
    FUN_00430f10();
    FUN_00430f10();
    FUN_00421910(local_90,0xc,9,FUN_00430f10);
    for (local_20 = 0; local_20 < 9; local_20 = local_20 + 1) {
      local_90[local_20 * 3] = *(float *)(param_1 + 0x40);
      local_90[local_20 * 3 + 1] = *(float *)(param_1 + 0x44);
      local_90[local_20 * 3 + 2] = *(float *)(param_1 + 0x48);
    }
    local_90[3] = local_90[3] - local_8;
    local_90[6] = local_90[6] + local_8;
    local_90[0xb] = local_90[0xb] - local_8;
    local_90[0xe] = local_90[0xe] + local_8;
    local_90[0xf] = local_90[0xf] - local_8;
    local_90[0x11] = local_90[0x11] - local_8;
    local_90[0x12] = local_90[0x12] - local_8;
    local_90[0x14] = local_90[0x14] + local_8;
    local_90[0x15] = local_90[0x15] + local_8;
    local_90[0x17] = local_90[0x17] - local_8;
    local_90[0x18] = local_90[0x18] + local_8;
    local_90[0x1a] = local_90[0x1a] + local_8;
    for (local_20 = 0; local_20 < 9; local_20 = local_20 + 1) {
      thunk_FUN_00561462(local_9c,local_90 + local_20 * 3,DAT_005ccf9c + 0x2a62c);
      thunk_FUN_00561462(local_1c,local_9c,DAT_005ccf9c + 0x2a5ec);
      if ((_DAT_005a4738 <= local_14) && (local_14 < _DAT_005a3660)) {
        iVar1 = *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4);
        uVar2 = 0;
        local_c = __ftol();
        local_10 = __ftol(*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8),0,
                          iVar1,uVar2);
        if ((-0x1e < local_c) &&
           (((local_c < (*(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4)) + 0x1e
             && (-0x1e < local_10)) &&
            (local_10 < (*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8)) + 0x1e
            )))) {
          return 1;
        }
      }
    }
  }
  else {
    local_a0 = 1;
  }
  return local_a0;
}

