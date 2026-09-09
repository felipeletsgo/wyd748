// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004be75b | Name: FUN_004be75b


undefined4 __fastcall FUN_004be75b(int param_1)

{
  if (*(int *)(param_1 + 0xb4) != 0) {
    if (*(int *)(param_1 + 0xb4) != 0) {
      FUN_004c5ae0(1);
    }
    *(undefined4 *)(param_1 + 0xb4) = 0;
  }
  return 0;
}

