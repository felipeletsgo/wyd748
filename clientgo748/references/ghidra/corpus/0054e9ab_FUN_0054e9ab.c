// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054e9ab | Name: FUN_0054e9ab


void FUN_0054e9ab(int param_1,undefined1 param_2)

{
  if (((*(char *)(param_1 + 2) == '=') || (*(char *)(param_1 + 4) == '=')) ||
     (*(char *)(param_1 + 6) == '=')) {
    if (*(char *)(param_1 + 2) == '=') {
      *(undefined1 *)(param_1 + 3) = param_2;
    }
    else if (*(char *)(param_1 + 4) == '=') {
      *(undefined1 *)(param_1 + 5) = param_2;
    }
    else {
      *(undefined1 *)(param_1 + 7) = param_2;
    }
  }
  else if (((*(char *)(param_1 + 2) == '\0') || (*(char *)(param_1 + 4) == '\0')) ||
          (*(char *)(param_1 + 6) == '\0')) {
    if (*(char *)(param_1 + 2) == '\0') {
      *(undefined1 *)(param_1 + 2) = 0x3d;
      *(undefined1 *)(param_1 + 3) = param_2;
    }
    else if (*(char *)(param_1 + 4) == '\0') {
      *(undefined1 *)(param_1 + 4) = 0x3d;
      *(undefined1 *)(param_1 + 5) = param_2;
    }
    else {
      *(undefined1 *)(param_1 + 6) = 0x3d;
      *(undefined1 *)(param_1 + 7) = param_2;
    }
  }
  return;
}

