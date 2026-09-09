// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005563a0 | Name: FUN_005563a0


int FUN_005563a0(int param_1,int param_2,int param_3,int param_4)

{
  int local_c;
  int local_8;
  
  if (param_3 < param_1) {
    local_8 = param_1 - param_3;
  }
  else {
    local_8 = param_3 - param_1;
  }
  if (param_4 < param_2) {
    local_c = param_2 - param_4;
  }
  else {
    local_c = param_4 - param_2;
  }
  if ((local_8 < 7) && (local_c < 7)) {
    local_8 = *(int *)(&DAT_005bec8c + local_8 * 4 + local_c * 0x1c);
  }
  else if (local_c < local_8) {
    local_8 = local_8 + 1;
  }
  else {
    local_8 = local_c + 1;
  }
  return local_8;
}

