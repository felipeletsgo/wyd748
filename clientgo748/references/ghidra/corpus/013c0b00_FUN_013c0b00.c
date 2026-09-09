// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0b00 | Name: FUN_013c0b00


undefined4 FUN_013c0b00(ushort *param_1)

{
  if (0x2f < *param_1) {
    DAT_013c1f34 = *(undefined4 *)(param_1 + 0x12);
    DAT_013c1f38 = *(undefined4 *)(param_1 + 0x14);
    DAT_013c1f3c = *(undefined4 *)(param_1 + 0x16);
  }
  FUN_0052d93d(param_1);
  return 1;
}

