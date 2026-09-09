// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f9a46 | Name: FUN_004f9a46


undefined4 __fastcall FUN_004f9a46(int param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  if ((((*(int *)(param_1 + 0x24c) == 0x19) && (*(short *)(param_1 + 0x1d2) == 3)) &&
      (*(short *)(param_1 + 0x1d4) == 8)) ||
     ((*(int *)(param_1 + 0x24c) == 0x19 && (*(short *)(param_1 + 0x1d2) == 0xc)))) {
    local_8 = 0x59;
  }
  if ((*(int *)(param_1 + 0x24c) == 0x1c) && (*(short *)(param_1 + 0x1d2) == 2)) {
    local_8 = 0x59;
  }
  if ((*(int *)(param_1 + 0x24c) == 0x10) && (*(short *)(param_1 + 0x1d2) == 6)) {
    local_8 = 0x59;
  }
  if (*(int *)(param_1 + 0x5c) == 0x1a) {
    local_8 = 0x59;
  }
  else if (*(int *)(param_1 + 0x5c) == 0x23) {
    local_8 = 0x59;
  }
  else if (*(int *)(param_1 + 0x5c) == 0x24) {
    local_8 = 0x59;
  }
  else if (*(int *)(param_1 + 0x5c) == 0xb) {
    local_8 = 0x59;
  }
  if ((*(int *)(param_1 + 0x24c) == 0x2c) || (*(int *)(param_1 + 0x24c) == 0x2d)) {
    local_8 = 0x38;
  }
  return local_8;
}

