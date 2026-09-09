// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00494d83 | Name: FUN_00494d83


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00494d83(int param_1)

{
  int local_8;
  
  _DAT_00e37d14 = 1;
  for (local_8 = 0; local_8 < 2; local_8 = local_8 + 1) {
    if (param_1 == *(int *)(&DAT_005c38f4 + local_8 * 4)) {
      _DAT_00e37d14 = 0;
    }
  }
  return;
}

