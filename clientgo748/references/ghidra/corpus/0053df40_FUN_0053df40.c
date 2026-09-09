// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053df40 | Name: FUN_0053df40


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0053df40(float param_1,float param_2,float param_3,float param_4)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_9c;
  undefined1 local_98 [12];
  float local_8c [27];
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  float local_10;
  int local_c;
  int local_8;
  
  local_20 = *(int *)(DAT_013b71e8 + 0x1b084);
  if (*(float *)(local_20 + 0x2c) < _DAT_005a45ec) {
    local_9c = 0;
    FUN_00430f10();
    FUN_00430f10();
    FUN_00421910(local_8c,0xc,9,FUN_00430f10);
    for (local_1c = 0; local_1c < 9; local_1c = local_1c + 1) {
      local_8c[local_1c * 3] = param_1;
      local_8c[local_1c * 3 + 1] = param_2;
      local_8c[local_1c * 3 + 2] = param_3;
    }
    local_8c[3] = local_8c[3] - param_4;
    local_8c[6] = local_8c[6] + param_4;
    local_8c[0xb] = local_8c[0xb] - param_4;
    local_8c[0xe] = local_8c[0xe] + param_4;
    local_8c[0xf] = local_8c[0xf] - param_4;
    local_8c[0x11] = local_8c[0x11] - param_4;
    local_8c[0x12] = local_8c[0x12] - param_4;
    local_8c[0x14] = local_8c[0x14] + param_4;
    local_8c[0x15] = local_8c[0x15] + param_4;
    local_8c[0x17] = local_8c[0x17] - param_4;
    local_8c[0x18] = local_8c[0x18] + param_4;
    local_8c[0x1a] = local_8c[0x1a] + param_4;
    for (local_1c = 0; local_1c < 9; local_1c = local_1c + 1) {
      thunk_FUN_00561462(local_98,local_8c + local_1c * 3,DAT_005ccf9c + 0x2a62c);
      thunk_FUN_00561462(local_18,local_98,DAT_005ccf9c + 0x2a5ec);
      if ((_DAT_005a4738 <= local_10) && (local_10 < _DAT_005a3660)) {
        iVar1 = *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4);
        uVar2 = 0;
        local_8 = __ftol();
        local_c = __ftol(*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8),0,iVar1
                         ,uVar2);
        if ((-0x1e < local_8) &&
           (((local_8 < (*(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4)) + 0x1e
             && (-0x1e < local_c)) &&
            (local_c < (*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8)) + 0x1e)
            ))) {
          return 1;
        }
      }
    }
  }
  else {
    local_9c = 1;
  }
  return local_9c;
}

