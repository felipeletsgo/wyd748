// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431a05 | Name: FUN_00431a05


void __fastcall FUN_00431a05(int param_1)

{
  if (*(int *)(param_1 + 0x11c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x11c) + 0x4c))(*(undefined4 *)(param_1 + 0x11c),0);
  }
  if (*(int *)(param_1 + 0x118) != 0) {
    (**(code **)(**(int **)(param_1 + 0x118) + 0x34))(*(undefined4 *)(param_1 + 0x118),0,0,0);
  }
  if (*(int *)(param_1 + 0x118) != 0) {
    (**(code **)(**(int **)(param_1 + 0x118) + 8))(*(undefined4 *)(param_1 + 0x118));
    *(undefined4 *)(param_1 + 0x118) = 0;
  }
  if (*(int *)(param_1 + 0x114) != 0) {
    (**(code **)(**(int **)(param_1 + 0x114) + 8))(*(undefined4 *)(param_1 + 0x114));
    *(undefined4 *)(param_1 + 0x114) = 0;
  }
  if (*(int *)(param_1 + 0x120) != 0) {
    (**(code **)(**(int **)(param_1 + 0x120) + 8))(*(undefined4 *)(param_1 + 0x120));
    *(undefined4 *)(param_1 + 0x120) = 0;
  }
  if (*(int *)(param_1 + 0x11c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x11c) + 8))(*(undefined4 *)(param_1 + 0x11c));
    *(undefined4 *)(param_1 + 0x11c) = 0;
  }
  if (*(int *)(param_1 + 0x110) != 0) {
    (**(code **)(**(int **)(param_1 + 0x110) + 8))(*(undefined4 *)(param_1 + 0x110));
    *(undefined4 *)(param_1 + 0x110) = 0;
  }
  return;
}

