// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594e4d | Name: FUN_00594e4d


undefined4 FUN_00594e4d(uint param_1)

{
  undefined4 *puVar1;
  
  if ((param_1 < DAT_013b8720) &&
     ((*(byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24) & 1) != 0)) {
    return *(undefined4 *)((&DAT_013b8620)[(int)param_1 >> 5] + (param_1 & 0x1f) * 0x24);
  }
  puVar1 = (undefined4 *)FUN_00594c1d();
  *puVar1 = 9;
  puVar1 = (undefined4 *)FUN_00594c26();
  *puVar1 = 0;
  return 0xffffffff;
}

