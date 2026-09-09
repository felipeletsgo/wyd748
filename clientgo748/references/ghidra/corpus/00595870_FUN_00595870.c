// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595870 | Name: FUN_00595870


undefined4 FUN_00595870(undefined4 param_1)

{
  DAT_013b85fc = HeapAlloc(DAT_013b8604,0,0x140);
  if (DAT_013b85fc == (LPVOID)0x0) {
    return 0;
  }
  DAT_013b85f4 = 0;
  DAT_013b85f8 = 0;
  DAT_013b85f0 = DAT_013b85fc;
  DAT_013b8600 = param_1;
  DAT_013b85e8 = 0x10;
  return 1;
}

