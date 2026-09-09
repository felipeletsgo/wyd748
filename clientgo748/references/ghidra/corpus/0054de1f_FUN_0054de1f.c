// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054de1f | Name: FUN_0054de1f


void FUN_0054de1f(int param_1,int param_2)

{
  bool bVar1;
  
  if (((param_2 < 0x7e) && (0x73 < param_2)) || (param_2 == 0x2b)) {
    if (((*(char *)(param_1 + 2) == '+') || (*(char *)(param_1 + 4) == '+')) ||
       (*(char *)(param_1 + 6) == '+')) {
      if (*(char *)(param_1 + 2) == '+') {
        *(undefined1 *)(param_1 + 2) = (undefined1)param_2;
      }
      else if (*(char *)(param_1 + 4) == '+') {
        *(undefined1 *)(param_1 + 4) = (undefined1)param_2;
      }
      else {
        *(undefined1 *)(param_1 + 6) = (undefined1)param_2;
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
        *(undefined1 *)(param_1 + 2) = (undefined1)param_2;
      }
      else {
        if ((*(char *)(param_1 + 4) < 's') || ('~' < *(char *)(param_1 + 4))) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) {
          *(undefined1 *)(param_1 + 4) = (undefined1)param_2;
        }
        else {
          if ((*(char *)(param_1 + 6) < 's') || ('~' < *(char *)(param_1 + 6))) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            *(undefined1 *)(param_1 + 6) = (undefined1)param_2;
          }
        }
      }
    }
  }
  return;
}

