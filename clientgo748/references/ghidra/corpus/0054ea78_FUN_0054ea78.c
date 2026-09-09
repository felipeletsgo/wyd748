// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ea78 | Name: FUN_0054ea78


short * FUN_0054ea78(int param_1,int param_2,int param_3,int param_4)

{
  short *local_8;
  
  if (param_3 == 0) {
    if ((param_4 < 0) || (0xf < param_4)) {
      return (short *)0x0;
    }
    local_8 = (short *)(param_1 + 0x5c + param_4 * 8);
  }
  else if (param_3 == 1) {
    if ((param_4 < 0) || (0x3f < param_4)) {
      return (short *)0x0;
    }
    local_8 = (short *)(param_1 + 0xdc + param_4 * 8);
  }
  else {
    if (param_3 != 2) {
      return (short *)0x0;
    }
    if ((param_4 < 0) || (0x7f < param_4)) {
      return (short *)0x0;
    }
    local_8 = (short *)(param_2 + param_4 * 8);
  }
  if ((*local_8 < 1) || (0x1964 < *local_8)) {
    local_8 = (short *)0x0;
  }
  return local_8;
}

