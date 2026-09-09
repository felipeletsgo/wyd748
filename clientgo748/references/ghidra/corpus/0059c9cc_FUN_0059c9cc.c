// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059c9cc | Name: FUN_0059c9cc


undefined4 FUN_0059c9cc(byte param_1,uint param_2,byte param_3)

{
  if ((*(byte *)((int)&DAT_013b84e0 + param_1 + 1) & param_3) == 0) {
    if (param_2 == 0) {
      param_2 = 0;
    }
    else {
      param_2 = *(ushort *)(&DAT_005cc11a + (uint)param_1 * 2) & param_2;
    }
    if (param_2 == 0) {
      return 0;
    }
  }
  return 1;
}

