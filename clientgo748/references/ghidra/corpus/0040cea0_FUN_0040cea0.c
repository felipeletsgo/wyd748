// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040cea0 | Name: FUN_0040cea0


int FUN_0040cea0(short param_1)

{
  int iVar1;
  uint local_14;
  short local_10 [4];
  int local_8;
  
  _memset(local_10,0,8);
  local_10[0] = param_1;
  local_14 = FUN_0054cd07(local_10,0x11);
  iVar1 = FUN_0054cd07(local_10,0x12);
  local_8 = 0;
  if (((0x27 < *(short *)(&DAT_00d44990 + param_1 * 0x8c)) &&
      (*(short *)(&DAT_00d44990 + param_1 * 0x8c) < 0x32)) &&
     (((local_14 & 4) == 4 ||
      ((((local_14 & 8) == 8 || ((local_14 & 0x10) == 0x10)) || ((local_14 & 0x20) == 0x20)))))) {
    if (*(short *)(&DAT_00d44990 + param_1 * 0x8c) == 0x28) {
      return 0;
    }
    if (iVar1 == 1) {
      local_8 = 0;
    }
    else if (iVar1 == 4) {
      local_8 = 1;
    }
    else if (iVar1 == 2) {
      local_8 = 2;
    }
    else if (iVar1 == 8) {
      local_8 = 3;
    }
    if ((local_14 & 4) == 4) {
      local_14 = 0;
    }
    else if ((local_14 & 8) == 8) {
      local_14 = 1;
    }
    else if ((local_14 & 0x10) == 0x10) {
      local_14 = 2;
    }
    else if ((local_14 & 0x20) == 0x20) {
      local_14 = 3;
    }
    return local_14 + 0x579 + (*(short *)(&DAT_00d44990 + param_1 * 0x8c) + -0x29 + local_8) * 4;
  }
  if (iVar1 == 1) {
    local_8 = 0;
  }
  else if (iVar1 == 2) {
    local_8 = 200;
  }
  else if (iVar1 == 4) {
    local_8 = 0x14;
  }
  else if (iVar1 == 8) {
    local_8 = 0xdc;
  }
  if (iVar1 < 9) {
    if ((local_14 & 2) == 2) {
      return *(short *)(&DAT_00d44990 + param_1 * 0x8c) + 0x3e9 + local_8;
    }
    if ((local_14 & 4) == 4) {
      return *(short *)(&DAT_00d44990 + param_1 * 0x8c) + 0x411 + local_8;
    }
    if ((local_14 & 8) == 8) {
      return *(short *)(&DAT_00d44990 + param_1 * 0x8c) + 0x439 + local_8;
    }
    if ((local_14 & 0x10) == 0x10) {
      return *(short *)(&DAT_00d44990 + param_1 * 0x8c) + 0x461 + local_8;
    }
    if ((local_14 & 0x20) == 0x20) {
      return *(short *)(&DAT_00d44990 + param_1 * 0x8c) + 0x489 + local_8;
    }
  }
  return (int)*(short *)(&DAT_00d44990 + param_1 * 0x8c);
}

