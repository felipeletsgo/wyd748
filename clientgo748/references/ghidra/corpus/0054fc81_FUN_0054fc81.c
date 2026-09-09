// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054fc81 | Name: FUN_0054fc81


void FUN_0054fc81(char *param_1)

{
  size_t local_10;
  undefined1 *local_c;
  
  local_10 = _strlen(param_1);
  local_c = (undefined1 *)FUN_005923dc(param_1,0x5f);
  do {
    if (local_c == (undefined1 *)0x0) {
      return;
    }
    *local_c = 0x20;
    local_c = (undefined1 *)FUN_005923dc(local_c,0x5f);
    local_10 = local_10 - 1;
  } while (-1 < (int)local_10);
  return;
}

