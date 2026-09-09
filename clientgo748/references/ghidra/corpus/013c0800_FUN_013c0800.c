// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0800 | Name: FUN_013c0800


undefined4 __thiscall FUN_013c0800(int param_1,ushort *param_2)

{
  if (((0x23 < *param_2) && (DAT_0067cf38 != 0)) && (param_1 == *(int *)(DAT_0067cf38 + 0x4c))) {
    DAT_013c1f8c = 1;
    DAT_013c1f14 = *(undefined4 *)(param_2 + 10);
    DAT_013c1f18 = *(undefined4 *)(param_2 + 0xc);
    DAT_013c1f0c = *(undefined4 *)(param_2 + 0xe);
    DAT_013c1f10 = *(undefined4 *)(param_2 + 0x10);
  }
  FUN_0052c5eb(param_2);
  return 1;
}

