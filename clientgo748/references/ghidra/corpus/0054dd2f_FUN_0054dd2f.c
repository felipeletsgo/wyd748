// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054dd2f | Name: FUN_0054dd2f


int FUN_0054dd2f(short *param_1,byte param_2)

{
  short sVar1;
  int local_14;
  int local_10;
  int local_c;
  
  if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
    if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
      local_c = 0;
      sVar1 = *param_1;
      if ((sVar1 < 1) || (0x1964 < sVar1)) {
        local_c = 0;
      }
      else if ((sVar1 < 0xc80) || (0xce4 < sVar1)) {
        for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
          if ((int)(char)param_1[local_10 + 1] == (uint)param_2) {
            local_14 = (int)*(char *)((int)param_1 + local_10 * 2 + 3);
            if ((param_2 == 0x1a) && (local_14 == 1)) {
              local_14 = 10;
            }
            local_c = local_c + local_14;
          }
        }
      }
      else {
        local_c = 0;
      }
    }
    else {
      local_c = 0;
    }
  }
  else {
    local_c = 0;
  }
  return local_c;
}

