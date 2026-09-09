// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057569e | Name: FUN_0057569e


void FUN_0057569e(int param_1,int param_2,undefined1 param_3)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    *(byte *)(param_2 + 9) = *(byte *)(param_2 + 9) | 8;
    *(undefined1 *)(param_2 + 0x2c) = param_3;
  }
  return;
}

