// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00549dd6 | Name: FUN_00549dd6


void __thiscall FUN_00549dd6(int param_1,int param_2)

{
  *(int *)(param_1 + 0x24) = param_2;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined4 *)(param_1 + 400) = 0;
  if ((*(uint *)(param_1 + 0x24) < 0x14b) || (0x156 < *(uint *)(param_1 + 0x24))) {
    if ((0x15e < *(uint *)(param_1 + 0x24)) && (*(uint *)(param_1 + 0x24) < 0x191)) {
      *(int *)(param_1 + 0x5c) = (param_2 + -0x15f) / 2 + 0x47;
    }
  }
  else {
    *(int *)(param_1 + 0x5c) = (param_2 + -0x14b) / 2 + 0x3f;
  }
  _memset((void *)(param_1 + 0x160),0,0x20);
  _memset((void *)(param_1 + 0x180),0,0x10);
  if (((((DAT_0092fee8 == 1) && (*(int *)(param_1 + 0x24) != 0x167)) &&
       (*(int *)(param_1 + 0x24) != 0x51)) &&
      ((*(int *)(param_1 + 0x24) != 0x173 && (*(int *)(param_1 + 0x24) != 0x171)))) &&
     ((*(int *)(param_1 + 0x24) != 0x162 &&
      ((*(int *)(param_1 + 0x24) != 0x179 && (*(int *)(param_1 + 0x24) != 0x17a)))))) {
    *(undefined4 *)(param_1 + 0x5c) = 0x57;
  }
  else {
    if (param_2 == 0x156) {
      *(undefined2 *)(param_1 + 0x160) = 1;
    }
    if (param_2 == 0x162) {
      *(undefined2 *)(param_1 + 0x162) = 1;
    }
    if (param_2 == 0x169) {
      *(undefined2 *)(param_1 + 0x160) = 0;
      *(undefined2 *)(param_1 + 0x162) = 1;
    }
    if (param_2 == 0x16a) {
      *(undefined2 *)(param_1 + 0x160) = 1;
      *(undefined2 *)(param_1 + 0x162) = 0;
    }
    if (param_2 == 0x179) {
      *(undefined2 *)(param_1 + 0x160) = 0;
      *(undefined2 *)(param_1 + 0x162) = 1;
    }
    if (param_2 == 0x177) {
      *(undefined2 *)(param_1 + 0x160) = 0;
      *(undefined2 *)(param_1 + 0x162) = 1;
    }
    if (param_2 == 0x175) {
      *(undefined2 *)(param_1 + 0x160) = 0;
      *(undefined2 *)(param_1 + 0x162) = 1;
    }
  }
  return;
}

