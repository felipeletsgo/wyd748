// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595813 | Name: FUN_00595813


undefined4 FUN_00595813(int param_1)

{
  int iVar1;
  
  DAT_013b8604 = HeapCreate((uint)(param_1 == 0),0x1000,0);
  if (DAT_013b8604 != (HANDLE)0x0) {
    DAT_013b8608 = FUN_005956cb();
    if (DAT_013b8608 == 3) {
      iVar1 = FUN_00595870(0x3f8);
    }
    else {
      if (DAT_013b8608 != 2) {
        return 1;
      }
      iVar1 = FUN_005963b7();
    }
    if (iVar1 != 0) {
      return 1;
    }
    HeapDestroy(DAT_013b8604);
  }
  return 0;
}

