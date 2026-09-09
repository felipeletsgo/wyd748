// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ab7cd | Name: FUN_004ab7cd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004ab7cd(int param_1,int param_2,int param_3,uint param_4,int param_5)

{
  int iVar1;
  int local_2c;
  int local_28;
  int local_24;
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
    for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
      if (*(int *)(param_1 + 0x26e60 + local_10 * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x26e60 + local_10 * 4) + 0x94) = local_c;
      }
    }
    *(undefined4 *)(*(int *)(param_1 + 0x26e44) + 0x94) = local_c;
  }
  else if ((uint)(param_3 - param_2) < param_4) {
    iVar1 = __ftol(param_4,0);
    if (DAT_005b892c == 2) {
      for (local_24 = 0; local_24 < 3; local_24 = local_24 + 1) {
        if (*(int *)(param_1 + 0x26e60 + local_24 * 4) != 0) {
          *(undefined **)(*(int *)(param_1 + 0x26e60 + local_24 * 4) + 0x94) = &DAT_01010101;
        }
      }
      *(uint *)(*(int *)(param_1 + 0x26e44) + 0x94) = iVar1 << 0x18 | 0xffffff;
    }
    else {
      for (local_28 = 0; local_28 < 3; local_28 = local_28 + 1) {
        if (*(int *)(param_1 + 0x26e60 + local_28 * 4) != 0) {
          *(uint *)(*(int *)(param_1 + 0x26e60 + local_28 * 4) + 0x94) = iVar1 << 0x18 | 0xffffff;
        }
      }
      *(uint *)(*(int *)(param_1 + 0x26e44) + 0x94) = iVar1 << 0x18 | 0xffffff;
    }
  }
  else if (DAT_005b892c != 2) {
    for (local_2c = 0; local_2c < 3; local_2c = local_2c + 1) {
      if (*(int *)(param_1 + 0x26e60 + local_2c * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x26e60 + local_2c * 4) + 0x94) = local_8;
      }
    }
    *(undefined4 *)(*(int *)(param_1 + 0x26e44) + 0x94) = local_8;
  }
  return;
}

