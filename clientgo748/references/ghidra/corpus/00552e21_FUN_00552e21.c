// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552e21 | Name: FUN_00552e21


int FUN_00552e21(int param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  if ((0x20d < param_1) && (param_1 < 0x21a)) {
    local_8 = (param_1 + -0x20e) % 3 + 1;
  }
  return local_8;
}

