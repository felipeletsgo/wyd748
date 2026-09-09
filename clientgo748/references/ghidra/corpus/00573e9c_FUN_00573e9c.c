// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573e9c | Name: FUN_00573e9c


void FUN_00573e9c(int param_1)

{
  if (*(byte *)(param_1 + 0x117) < 8) {
    *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) | 4;
    *(undefined1 *)(param_1 + 0x118) = 8;
  }
  return;
}

