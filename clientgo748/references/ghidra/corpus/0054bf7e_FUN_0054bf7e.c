// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054bf7e | Name: FUN_0054bf7e


byte FUN_0054bf7e(int param_1)

{
  byte bVar1;
  
  bVar1 = *(byte *)(param_1 + 7) & 0xf;
  if ((*(byte *)(param_1 + 7) & 0xf) == 0) {
    bVar1 = 1;
  }
  if (7 < bVar1) {
    bVar1 = 7;
  }
  return bVar1;
}

