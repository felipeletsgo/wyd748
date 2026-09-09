// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054fc3d | Name: FUN_0054fc3d


void FUN_0054fc3d(char *param_1)

{
  size_t sVar1;
  uint local_8;
  
  local_8 = 0;
  while( true ) {
    sVar1 = _strlen(param_1);
    if (sVar1 <= local_8) break;
    if (param_1[local_8] == ' ') {
      param_1[local_8] = '_';
    }
    local_8 = local_8 + 1;
  }
  return;
}

