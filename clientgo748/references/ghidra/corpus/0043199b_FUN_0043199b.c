// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0043199b | Name: FUN_0043199b


void __fastcall FUN_0043199b(int param_1)

{
  if (*(int *)(param_1 + 0x114) != 0) {
    (**(code **)(**(int **)(param_1 + 0x114) + 0x24))(*(undefined4 *)(param_1 + 0x114));
  }
  *(undefined4 *)(param_1 + 0x10c) = 0;
  *(undefined4 *)(param_1 + 0x108) = 0;
  FUN_00431a05();
  *(undefined1 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0x10c) = 3;
  return;
}

