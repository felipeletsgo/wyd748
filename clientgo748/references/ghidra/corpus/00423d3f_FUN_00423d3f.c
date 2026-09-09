// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423d3f | Name: FUN_00423d3f


void __fastcall FUN_00423d3f(int param_1)

{
  if (*(int *)(param_1 + 4) != 0) {
    FUN_0058ffd8(*(undefined4 *)(param_1 + 4));
  }
  if (*(int *)(param_1 + 8) != 0) {
    FUN_0058ffd8(*(undefined4 *)(param_1 + 8));
  }
  return;
}

