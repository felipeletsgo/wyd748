// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00526cf4 | Name: FUN_00526cf4


int __thiscall FUN_00526cf4(int param_1,int param_2)

{
  if ((*(short *)(param_1 + 0x7ba) < 0) || (10 < *(short *)(param_1 + 0x7ba))) {
    *(undefined2 *)(param_1 + 0x7ba) = 0;
  }
  if ((((*(short *)(param_1 + 0x7ba) != 0) && (param_2 != 0x22)) && (param_2 != 0x23)) &&
     (param_2 != 0x24)) {
    if (param_2 == 0x13) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x27;
    }
    else if (param_2 == 3) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x31;
    }
    else if (param_2 == 2) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x3b;
    }
    else if (param_2 == 6) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x45;
    }
    else if (param_2 == 1) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x4f;
    }
    else if (param_2 == 0) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x59;
    }
    else if (param_2 == 7) {
      param_2 = *(short *)(param_1 + 0x7ba) + 99;
    }
    else if (param_2 == 0x19) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x6d;
    }
    else if (param_2 == 0x18) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x77;
    }
    else if (param_2 == 0x1a) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x81;
    }
    else if (param_2 == 0x1c) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x8b;
    }
    else if (param_2 == 0x1b) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x95;
    }
    else if (param_2 == 0x1d) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0x9f;
    }
    else if (param_2 == 0x20) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0xa9;
    }
    else if (param_2 == 0x1f) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0xb3;
    }
    else if (param_2 == 0x21) {
      param_2 = *(short *)(param_1 + 0x7ba) + 0xbd;
    }
  }
  return param_2;
}

