// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054abe3 | Name: FUN_0054abe3


void __fastcall FUN_0054abe3(int param_1)

{
  *(undefined1 *)(param_1 + 0x14) = 1;
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0054abe3();
  }
  return;
}

