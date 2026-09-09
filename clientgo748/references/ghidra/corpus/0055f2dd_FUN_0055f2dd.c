// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055f2dd | Name: FUN_0055f2dd


void FUN_0055f2dd(int param_1,undefined4 param_2)

{
  if (((param_1 != 0) &&
      (((DAT_0092e65c + 1000U < DAT_0092e658 || (*(short *)(param_1 + 4) != 0x366)) ||
       (DAT_013b71ec != 0x366)))) &&
     (((DAT_0092e65c + 1000U < DAT_0092e658 ||
       (((*(short *)(param_1 + 4) != 0x36c && (*(short *)(param_1 + 4) != 0x39d)) &&
        (*(short *)(param_1 + 4) != 0x39e)))) ||
      (((DAT_013b71ec != 0x36c && (DAT_013b71ec != 0x39d)) && (DAT_013b71ec != 0x39e)))))) {
    FUN_0042550e(param_1,param_2);
    DAT_013b71ec = *(short *)(param_1 + 4);
    FUN_0055f3d6(param_1);
  }
  return;
}

