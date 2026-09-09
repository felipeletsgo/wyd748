// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040ac07 | Name: FUN_0040ac07


void __thiscall FUN_0040ac07(int param_1,char param_2)

{
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  *(undefined4 *)(param_1 + 0x38c) = 1;
  if (param_2 < '\x03') {
    for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
      *(undefined1 *)(param_1 + 0x360 + param_2 * 3 + local_8) = 1;
    }
  }
  else if (param_2 == '\x03') {
    for (local_c = 0; local_c < 3; local_c = local_c + 1) {
      *(undefined1 *)(param_1 + 0x360 + local_c * 4) = 1;
    }
  }
  else if (param_2 == '\x04') {
    for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
      *(undefined1 *)(param_1 + local_10 * -2 + 0x366) = 1;
    }
  }
  return;
}

