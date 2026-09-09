// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054e75a | Name: FUN_0054e75a


undefined4 FUN_0054e75a(int param_1,int param_2,int param_3)

{
  bool bVar1;
  undefined4 uVar2;
  int local_c;
  char local_8;
  
  if (param_2 < 0) {
    param_2 = 0;
  }
  if (param_2 < 0x1a) {
    if (param_3 < 0) {
      param_3 = 0;
    }
    if (0x14 < param_3) {
      param_3 = 0x14;
    }
    local_c = 0;
    if (9 < param_2) {
      local_c = param_2 + -9;
      param_2 = 9;
    }
    if (local_c < 1) {
      local_8 = (char)param_3 * '\n' + (char)param_2;
    }
    else {
      local_8 = (char)local_c + -0x1b;
    }
    if (*(char *)(param_1 + 2) == '+') {
      *(char *)(param_1 + 3) = local_8;
    }
    else if (*(char *)(param_1 + 4) == '+') {
      *(char *)(param_1 + 5) = local_8;
    }
    else if (*(char *)(param_1 + 6) == '+') {
      *(char *)(param_1 + 7) = local_8;
    }
    else {
      if ((*(char *)(param_1 + 2) < 's') || ('~' < *(char *)(param_1 + 2))) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) {
        *(char *)(param_1 + 3) = local_8;
      }
      else {
        if ((*(char *)(param_1 + 4) < 's') || ('~' < *(char *)(param_1 + 4))) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) {
          *(char *)(param_1 + 5) = local_8;
        }
        else {
          if ((*(char *)(param_1 + 6) < 's') || ('~' < *(char *)(param_1 + 6))) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            *(char *)(param_1 + 7) = local_8;
          }
          else if (*(char *)(param_1 + 2) == '\0') {
            *(undefined1 *)(param_1 + 2) = 0x2b;
            *(char *)(param_1 + 3) = local_8;
          }
          else if (*(char *)(param_1 + 4) == '\0') {
            *(undefined1 *)(param_1 + 4) = 0x2b;
            *(char *)(param_1 + 5) = local_8;
          }
          else {
            if (*(char *)(param_1 + 6) != '\0') {
              return 0;
            }
            *(undefined1 *)(param_1 + 6) = 0x2b;
            *(char *)(param_1 + 7) = local_8;
          }
        }
      }
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

