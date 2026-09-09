// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c167b | Name: FUN_004c167b


void __thiscall FUN_004c167b(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x8c) == 0) {
    *(int *)(param_1 + 0x8c) = param_2;
  }
  else {
    *(undefined4 *)(param_2 + 0x88) = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x88);
    *(int *)(*(int *)(param_1 + 0x8c) + 0x88) = param_2;
  }
  return;
}

