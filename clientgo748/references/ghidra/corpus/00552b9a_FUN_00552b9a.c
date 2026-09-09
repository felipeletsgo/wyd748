// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552b9a | Name: FUN_00552b9a


int FUN_00552b9a(short *param_1)

{
  int iVar1;
  uint local_10;
  uint local_c;
  
  local_10 = 0;
  local_c = 0;
  if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
    if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
      if ((((char)param_1[1] == '>') || ((char)param_1[2] == '>')) || ((char)param_1[3] == '>')) {
        if ((char)param_1[1] == '>') {
          local_10 = (uint)*(byte *)((int)param_1 + 3);
        }
        else if ((char)param_1[2] == '>') {
          local_10 = (uint)*(byte *)((int)param_1 + 5);
        }
        else {
          local_10 = (uint)*(byte *)((int)param_1 + 7);
        }
      }
      if ((((char)param_1[1] == '?') || ((char)param_1[2] == '?')) || ((char)param_1[3] == '?')) {
        if ((char)param_1[1] == '?') {
          local_c = (uint)*(byte *)((int)param_1 + 3);
        }
        else if ((char)param_1[2] == '?') {
          local_c = (uint)*(byte *)((int)param_1 + 5);
        }
        else {
          local_c = (uint)*(byte *)((int)param_1 + 7);
        }
      }
      iVar1 = local_10 * 0x100 + local_c;
    }
    else {
      iVar1 = 0;
    }
  }
  else {
    iVar1 = 0;
  }
  return iVar1;
}

