// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489300 | Name: FUN_00489300


undefined4 __thiscall FUN_00489300(int param_1,short *param_2)

{
  if (*param_2 == 0x10) {
    *(short *)(DAT_013b71e8 + 0xf10) = param_2[6];
    if (*(int *)(param_2 + 6) == 0) {
      *(undefined2 *)(DAT_013b71e8 + 0xf10) = 0xffff;
    }
  }
  else if (*param_2 == 0x14) {
    *(short *)(DAT_013b71e8 + 0xf10) = param_2[6];
    if (*(int *)(param_2 + 6) == 0) {
      *(undefined2 *)(DAT_013b71e8 + 0xf10) = 0xffff;
    }
    *(char *)(param_1 + 0x26ebc) = (char)param_2[8];
  }
  else if (*param_2 == 0x18) {
    *(short *)(DAT_013b71e8 + 0xf10) = param_2[6];
    if (*(int *)(param_2 + 6) == 0) {
      *(undefined2 *)(DAT_013b71e8 + 0xf10) = 0xffff;
    }
    *(char *)(param_1 + 0x26ebc) = (char)param_2[8];
    *(short *)(DAT_013b71e8 + 0xf12) = param_2[10];
  }
  return 1;
}

