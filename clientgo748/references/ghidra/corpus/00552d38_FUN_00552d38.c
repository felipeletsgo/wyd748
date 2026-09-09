// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552d38 | Name: FUN_00552d38


int FUN_00552d38(int param_1)

{
  undefined4 local_10;
  undefined4 local_c;
  
  local_10 = 0;
  local_c = 0;
  if (((*(char *)(param_1 + 2) == '8') || (*(char *)(param_1 + 4) == '8')) ||
     (*(char *)(param_1 + 6) == '8')) {
    if (*(char *)(param_1 + 2) == '8') {
      local_10 = (uint)*(byte *)(param_1 + 3);
    }
    else if (*(char *)(param_1 + 4) == '8') {
      local_10 = (uint)*(byte *)(param_1 + 5);
    }
    else {
      local_10 = (uint)*(byte *)(param_1 + 7);
    }
  }
  if (((*(char *)(param_1 + 2) == '9') || (*(char *)(param_1 + 4) == '9')) ||
     (*(char *)(param_1 + 6) == '9')) {
    if (*(char *)(param_1 + 2) == '9') {
      local_c = (uint)*(byte *)(param_1 + 3);
    }
    else if (*(char *)(param_1 + 4) == '9') {
      local_c = (uint)*(byte *)(param_1 + 5);
    }
    else {
      local_c = (uint)*(byte *)(param_1 + 7);
    }
  }
  return local_10 * 0x100 + local_c;
}

