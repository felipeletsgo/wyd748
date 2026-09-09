// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054e06c | Name: FUN_0054e06c


uint FUN_0054e06c(short *param_1)

{
  bool bVar1;
  uint local_c;
  uint local_8;
  
  if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
    if ((*param_1 < 0xc80) || (0xce3 < *param_1)) {
      if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
        local_c = 0;
        if (((char)param_1[1] == '+') || (((char)param_1[2] == '+' || ((char)param_1[3] == '+')))) {
          if ((char)param_1[1] == '+') {
            local_c = (uint)*(byte *)((int)param_1 + 3);
          }
          else if ((char)param_1[2] == '+') {
            local_c = (uint)*(byte *)((int)param_1 + 5);
          }
          else {
            local_c = (uint)*(byte *)((int)param_1 + 7);
          }
        }
        else {
          if (((char)param_1[1] < 's') || ('~' < (char)param_1[1])) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            local_c = (uint)*(byte *)((int)param_1 + 3);
          }
          else {
            if (((char)param_1[2] < 's') || ('~' < (char)param_1[2])) {
              bVar1 = false;
            }
            else {
              bVar1 = true;
            }
            if (bVar1) {
              local_c = (uint)*(byte *)((int)param_1 + 5);
            }
            else {
              if (((char)param_1[3] < 's') || ('~' < (char)param_1[3])) {
                bVar1 = false;
              }
              else {
                bVar1 = true;
              }
              if (bVar1) {
                local_c = (uint)*(byte *)((int)param_1 + 7);
              }
            }
          }
        }
        if (((*param_1 != 0x312) && (*param_1 != 0x790)) && (*param_1 != 0x791)) {
          if (local_c < 0xe6) {
            local_8 = local_c % 10;
          }
          else {
            local_8 = local_c - 0xdc;
          }
          local_c = local_8;
          if ((9 < (int)local_8) && ((int)local_8 < 0x24)) {
            local_c = ((int)((local_8 - 10) + ((int)(local_8 - 10) >> 0x1f & 3U)) >> 2) + 10;
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

