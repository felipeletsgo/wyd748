// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d7fb1 | Name: FUN_004d7fb1


void __fastcall FUN_004d7fb1(int param_1)

{
  if (*(int *)(param_1 + 0x60) != 0) {
    FUN_004be10c();
  }
  if (*(int *)(param_1 + 0x21c) != 0) {
    FUN_004be10c();
  }
  return;
}

