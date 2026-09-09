// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005558e1 | Name: FUN_005558e1


int FUN_005558e1(int param_1)

{
  undefined4 local_8;
  
  local_8 = (int)*(short *)(param_1 + 0x14) / 2 + 0x32;
  if (100 < local_8) {
    local_8 = 100;
  }
  return local_8;
}

