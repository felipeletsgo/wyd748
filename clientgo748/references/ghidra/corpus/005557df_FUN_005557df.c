// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005557df | Name: FUN_005557df


int FUN_005557df(short *param_1,short *param_2)

{
  int local_8;
  
  if ((param_2[10] == 0) || (*param_2 == 0)) {
    local_8 = 0x5f;
  }
  else if ((((param_1[10] < 0) || (param_2[10] < 0)) || (*param_1 < 0)) || (*param_2 < 0)) {
    local_8 = 0;
  }
  else {
    local_8 = (((param_1[10] * 0x3c) / ((int)param_1[10] + (int)param_2[10] / 2)) *
              ((((int)*param_1 + (int)*param_2) * 100) / (int)*param_2)) / 100;
    if (0x5e < local_8) {
      local_8 = 0x5f;
    }
    if (local_8 < 0x28) {
      local_8 = 0x28;
    }
  }
  return local_8;
}

