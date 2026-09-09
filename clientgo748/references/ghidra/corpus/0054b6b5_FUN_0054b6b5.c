// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b6b5 | Name: FUN_0054b6b5


undefined4 FUN_0054b6b5(int param_1,undefined4 param_2)

{
  byte bVar1;
  short sVar2;
  undefined4 uVar3;
  int local_2c;
  int local_28;
  uint local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_10;
  undefined2 local_8;
  
  bVar1 = *(byte *)(param_1 + 0x14);
  local_20 = FUN_00559ef5(param_2);
  if ((local_20 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
    local_20 = 2;
  }
  if (local_20 == 2) {
    local_1c = 0x640;
  }
  else if (local_20 == 1) {
    sVar2 = *(short *)(param_1 + 0x40);
    local_28 = FUN_0054cd07(param_1 + 0x5c,0x56);
    if (0x38 < local_28) {
      local_28 = 0x38;
    }
    local_1c = sVar2 * 4 + local_28 * 3;
    if (0x162 < *(short *)(param_1 + 0x40)) {
      local_1c = local_1c - (*(short *)(param_1 + 0x40) * 2 + -0x2c4);
    }
  }
  else {
    local_1c = *(short *)(param_1 + 0x40) * 3;
    local_2c = *(short *)(param_1 + 0x40) + -199;
    if (0x162 < *(short *)(param_1 + 0x40)) {
      local_2c = local_2c - (*(short *)(param_1 + 0x40) + -0x162);
    }
    if (0 < local_2c) {
      local_1c = local_1c + local_2c;
    }
  }
  if ((*(ushort *)(param_1 + 0x16) & 1) != 0) {
    local_1c = local_1c + 9;
  }
  if (bVar1 < 4) {
    local_18 = 0;
    local_24 = 1;
    for (local_10 = 0; local_10 < 0x18; local_10 = local_10 + 1) {
      if ((*(uint *)(param_1 + 0x2dc) & local_24) != 0) {
        local_18 = local_18 + *(int *)(&DAT_0092fef8 + ((uint)bVar1 * 0x18 + local_10) * 0x60);
      }
      local_24 = local_24 << 1;
    }
    for (local_10 = 0x18; local_10 < 0x20; local_10 = local_10 + 1) {
      if ((*(uint *)(param_1 + 0x2dc) & local_24) != 0) {
        local_18 = local_18 + *(int *)(&DAT_0092fef8 + (local_10 + 0x48) * 0x60);
      }
      local_24 = local_24 << 1;
    }
    if (local_1c - local_18 == 0) {
      uVar3 = 1;
    }
    else {
      local_8 = (undefined2)(local_1c - local_18);
      *(undefined2 *)(param_1 + 0x2e4) = local_8;
      uVar3 = 0;
    }
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

