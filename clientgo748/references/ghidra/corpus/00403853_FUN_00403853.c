// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403853 | Name: FUN_00403853


void __fastcall FUN_00403853(int param_1)

{
  if (*(int *)(param_1 + 0x2c) == 0) {
    *(undefined4 *)(param_1 + 0xa4) = 4;
    if (*(int *)(param_1 + 0x1f8) != 0) {
      *(undefined4 *)(param_1 + 0x94) = 0xff7f7f7f;
    }
  }
  else if (*(int *)(param_1 + 0x1e8) == 0) {
    if (*(int *)(param_1 + 0x34) == 1) {
      if (*(int *)(param_1 + 500) == 1) {
        *(undefined4 *)(param_1 + 0xa4) = 2;
      }
      else {
        *(undefined4 *)(param_1 + 0xa4) = 1;
      }
    }
    else {
      *(undefined4 *)(param_1 + 0xa4) = 0;
      if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x1b0) != 0)) {
        *(undefined **)(param_1 + 0x94) = &DAT_01010101;
      }
      else {
        *(undefined4 *)(param_1 + 0x94) = *(undefined4 *)(param_1 + 0x1fc);
      }
    }
  }
  else {
    *(undefined4 *)(param_1 + 0xa4) = 3;
  }
  return;
}

