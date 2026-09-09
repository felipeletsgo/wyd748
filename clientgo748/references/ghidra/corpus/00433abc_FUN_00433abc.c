// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00433abc | Name: FUN_00433abc


int FUN_00433abc(int param_1,int param_2,int param_3,int param_4)

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
  if (local_c < local_8) {
    local_c = local_8;
  }
  return local_c + 1;
}

