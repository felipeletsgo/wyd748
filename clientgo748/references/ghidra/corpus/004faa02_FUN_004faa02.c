// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004faa02 | Name: FUN_004faa02


void __thiscall FUN_004faa02(int param_1,undefined4 param_2)

{
  if ((*(int *)(param_1 + 0x214) == 0) &&
     (*(undefined4 *)(param_1 + 0x778) = param_2, *(int *)(param_1 + 0x60) != 0)) {
    if ((*(int *)(param_1 + 0x670) == 1) && (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0)) {
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) =
           *(undefined4 *)(param_1 + 0x778);
    }
    else if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) = 0;
    }
    if ((*(int *)(param_1 + 0x674) == 1) && (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) =
           *(undefined4 *)(param_1 + 0x778);
    }
    else if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) = 0;
    }
  }
  return;
}

