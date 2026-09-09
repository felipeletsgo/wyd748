// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ee5b | Name: FUN_0054ee5b


int FUN_0054ee5b(int param_1)

{
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = 0;
  _memset((void *)(param_1 + 0x24),0,0x1c);
  for (local_c = 0; local_c < 0x10; local_c = local_c + 1) {
    local_8 = local_8 + *(short *)(param_1 + 0x5c + local_c * 8) * local_c;
  }
  for (local_c = 0; local_c < 0x40; local_c = local_c + 1) {
    local_8 = local_8 + *(short *)(param_1 + 0xdc + local_c * 8) * local_c;
  }
  return local_8;
}

