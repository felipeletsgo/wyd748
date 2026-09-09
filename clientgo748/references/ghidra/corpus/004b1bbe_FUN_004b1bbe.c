// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1bbe | Name: FUN_004b1bbe


int FUN_004b1bbe(char *param_1)

{
  int local_10;
  int local_8;
  
  local_10 = (int)*param_1;
  if ((local_10 < 0x30) || (0x39 < local_10)) {
    if ((local_10 < 0x61) || (0x66 < local_10)) {
      if ((0x40 < local_10) && (local_10 < 0x47)) {
        local_10 = local_10 + -0x37;
      }
    }
    else {
      local_10 = local_10 + -0x57;
    }
  }
  else {
    local_10 = local_10 + -0x30;
  }
  local_8 = (int)param_1[1];
  if ((local_8 < 0x30) || (0x39 < local_8)) {
    if ((local_8 < 0x61) || (0x66 < local_8)) {
      if ((0x40 < local_8) && (local_8 < 0x47)) {
        local_8 = local_8 + -0x37;
      }
    }
    else {
      local_8 = local_8 + -0x57;
    }
  }
  else {
    local_8 = local_8 + -0x30;
  }
  return local_8 + local_10 * 0x10;
}

