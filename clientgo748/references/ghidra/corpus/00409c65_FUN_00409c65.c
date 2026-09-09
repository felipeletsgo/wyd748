// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409c65 | Name: FUN_00409c65


void __fastcall FUN_00409c65(int param_1)

{
  if (*(int *)(param_1 + 0x1e4) == 1) {
    *(undefined4 *)(param_1 + 0xa4) = 1;
    *(undefined4 *)(param_1 + 0x94) = *(undefined4 *)(param_1 + 0x1ec);
  }
  else {
    *(undefined4 *)(param_1 + 0xa4) = 0;
    *(undefined4 *)(param_1 + 0x94) = *(undefined4 *)(param_1 + 0x1f0);
  }
  return;
}

