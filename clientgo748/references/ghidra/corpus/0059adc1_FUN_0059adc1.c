// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059adc1 | Name: FUN_0059adc1


byte FUN_0059adc1(uint param_1)

{
  if (DAT_013b8720 <= param_1) {
    return 0;
  }
  return *(byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24) & 0x40;
}

