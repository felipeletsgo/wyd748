// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054bfb7 | Name: FUN_0054bfb7


int FUN_0054bfb7(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 local_8;
  
  param_3 = param_3 / 2;
  if (7 < param_3) {
    param_3 = 7;
  }
  iVar1 = _rand();
  local_8 = ((param_1 - param_2 / 2) * (iVar1 % (0xc - param_3) + 99 + param_3)) / 100;
  if (local_8 < -0x32) {
    local_8 = 0;
  }
  else if ((local_8 < -0x32) || (-1 < local_8)) {
    if ((-1 < local_8) && (local_8 < 0x33)) {
      local_8 = ((int)(local_8 * 5 + (local_8 * 5 >> 0x1f & 3U)) >> 2) + 7;
    }
  }
  else {
    local_8 = (local_8 + 0x32) / 7;
  }
  if (local_8 < 1) {
    local_8 = 1;
  }
  return local_8;
}

