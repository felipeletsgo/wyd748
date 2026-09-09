// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005588b2 | Name: FUN_005588b2


bool FUN_005588b2(char param_1,byte param_2)

{
  return (1 << (param_2 & 0x1f) & (int)param_1) != 0;
}

