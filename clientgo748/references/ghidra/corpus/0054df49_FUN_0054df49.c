// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054df49 | Name: FUN_0054df49


int FUN_0054df49(int param_1)

{
  bool bVar1;
  undefined4 local_8;
  
  local_8 = 0;
  if (((*(char *)(param_1 + 2) == '+') || (*(char *)(param_1 + 4) == '+')) ||
     (*(char *)(param_1 + 6) == '+')) {
    if (*(char *)(param_1 + 2) == '+') {
      local_8 = (int)*(char *)(param_1 + 2);
    }
    else if (*(char *)(param_1 + 4) == '+') {
      local_8 = (int)*(char *)(param_1 + 4);
    }
    else {
      local_8 = (int)*(char *)(param_1 + 6);
    }
  }
  else {
    if ((*(char *)(param_1 + 2) < 's') || ('~' < *(char *)(param_1 + 2))) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      local_8 = (int)*(char *)(param_1 + 2);
    }
    else {
      if ((*(char *)(param_1 + 4) < 's') || ('~' < *(char *)(param_1 + 4))) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) {
        local_8 = (int)*(char *)(param_1 + 4);
      }
      else {
        if ((*(char *)(param_1 + 6) < 's') || ('~' < *(char *)(param_1 + 6))) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) {
          local_8 = (int)*(char *)(param_1 + 6);
        }
      }
    }
  }
  return local_8;
}

