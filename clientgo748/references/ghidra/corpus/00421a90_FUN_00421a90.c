// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00421a90 | Name: FUN_00421a90


undefined4 FUN_00421a90(char *param_1,int param_2)

{
  size_t sVar1;
  int local_c;
  
  sVar1 = _strlen(param_1);
  local_c = 0;
  do {
    if ((int)sVar1 <= local_c) {
      return 1;
    }
    if (param_1[local_c] < '\0') {
      if (local_c == param_2) {
        return 0;
      }
      if (local_c == param_2 + 1) {
        return 1;
      }
      local_c = local_c + 1;
    }
    else if (param_2 <= local_c) {
      return 1;
    }
    local_c = local_c + 1;
  } while( true );
}

