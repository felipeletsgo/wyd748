// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054e93b | Name: FUN_0054e93b


int FUN_0054e93b(int param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  if (((*(char *)(param_1 + 2) == '=') || (*(char *)(param_1 + 4) == '=')) ||
     (*(char *)(param_1 + 6) == '=')) {
    if (*(char *)(param_1 + 2) == '=') {
      local_8 = (int)*(char *)(param_1 + 3);
    }
    else if (*(char *)(param_1 + 4) == '=') {
      local_8 = (int)*(char *)(param_1 + 5);
    }
    else {
      local_8 = (int)*(char *)(param_1 + 7);
    }
  }
  return local_8;
}

