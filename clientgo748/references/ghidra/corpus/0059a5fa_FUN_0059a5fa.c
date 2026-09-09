// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a5fa | Name: FUN_0059a5fa


int FUN_0059a5fa(int param_1)

{
  bool bVar1;
  
  if (DAT_013b82e4 == 0) {
    if ((0x40 < param_1) && (param_1 < 0x5b)) {
      return param_1 + 0x20;
    }
  }
  else {
    InterlockedIncrement((LONG *)&DAT_013b8728);
    bVar1 = DAT_013b8724 != 0;
    if (bVar1) {
      InterlockedDecrement((LONG *)&DAT_013b8728);
      FUN_00596aad(0x13);
    }
    param_1 = FUN_0059a669(param_1);
    if (bVar1) {
      FUN_00596b0e(0x13);
    }
    else {
      InterlockedDecrement((LONG *)&DAT_013b8728);
    }
  }
  return param_1;
}

