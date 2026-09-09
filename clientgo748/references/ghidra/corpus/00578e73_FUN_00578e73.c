// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578e73 | Name: FUN_00578e73


void FUN_00578e73(int param_1)

{
  if (*(int *)(param_1 + 4) != 0) {
    (**(code **)(*(int *)(param_1 + 4) + 0x28))(param_1);
  }
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0x10) = 0;
  return;
}

