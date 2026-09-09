// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005555ed | Name: FUN_005555ed


void FUN_005555ed(short *param_1,short *param_2,int param_3)

{
  int local_8;
  
  *param_1 = *(short *)(param_3 + 0xc);
  *param_2 = *(short *)(param_3 + 0xe);
  for (local_8 = 0; (local_8 < 0x18 && (*(char *)(param_3 + local_8 + 0x1c) != '\0'));
      local_8 = local_8 + 1) {
    if (*(char *)(param_3 + local_8 + 0x1c) == '6') {
      *param_1 = *param_1 + 1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '4') {
      *param_1 = *param_1 + -1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '8') {
      *param_2 = *param_2 + -1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '2') {
      *param_2 = *param_2 + 1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '9') {
      *param_1 = *param_1 + 1;
      *param_2 = *param_2 + -1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '3') {
      *param_1 = *param_1 + 1;
      *param_2 = *param_2 + 1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '7') {
      *param_1 = *param_1 + -1;
      *param_2 = *param_2 + -1;
    }
    if (*(char *)(param_3 + local_8 + 0x1c) == '1') {
      *param_1 = *param_1 + -1;
      *param_2 = *param_2 + 1;
    }
  }
  return;
}

