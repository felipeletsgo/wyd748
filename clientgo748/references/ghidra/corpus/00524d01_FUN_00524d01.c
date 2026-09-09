// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00524d01 | Name: FUN_00524d01


void __thiscall FUN_00524d01(int param_1,int param_2)

{
  if ((((((param_2 == 0) || (param_2 == 2)) || (param_2 == 8)) ||
       ((param_2 == 9 || (param_2 == 0xb)))) || (param_2 == 0x18)) ||
     (((param_2 == 0x1b || (param_2 == 0x1f)) || (param_2 == 0x22)))) {
    *(undefined1 *)(param_1 + 0x79b) = 1;
  }
  else if ((((((param_2 == 1) || (param_2 == 3)) || (param_2 == 0xc)) ||
            ((param_2 == 0xd || (param_2 == 0xe)))) ||
           (((param_2 == 0x19 || ((param_2 == 0x1c || (param_2 == 0x1e)))) || (param_2 == 0x20))))
          || (param_2 == 0x23)) {
    *(undefined1 *)(param_1 + 0x79b) = 2;
  }
  else if ((((param_2 == 6) || (param_2 == 7)) || (param_2 == 0xf)) ||
          ((((param_2 == 0x10 || (param_2 == 0x11)) ||
            ((param_2 == 0x13 || ((param_2 == 0x1a || (param_2 == 0x1d)))))) ||
           ((param_2 == 0x21 || (param_2 == 0x24)))))) {
    *(undefined1 *)(param_1 + 0x79b) = 3;
  }
  else {
    *(undefined1 *)(param_1 + 0x79b) = 4;
  }
  return;
}

