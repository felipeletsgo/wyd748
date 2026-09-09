// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004faaf4 | Name: FUN_004faaf4


float10 FUN_004faaf4(int param_1,int param_2)

{
  undefined4 local_8;
  
  local_8 = 1.0;
  if (param_1 == 0x1c) {
    local_8 = 1.45;
  }
  else if ((param_1 == 0x19) && (param_2 == 1)) {
    local_8 = 1.4;
  }
  else if ((param_1 == 0x14) && (param_2 == 7)) {
    local_8 = 0.6;
  }
  else if ((param_1 == 0x14) && (param_2 == 0)) {
    local_8 = 1.3;
  }
  return (float10)local_8;
}

