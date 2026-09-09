// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0050494d | Name: FUN_0050494d


void __fastcall FUN_0050494d(int param_1)

{
  (**(code **)(**(int **)(param_1 + 0x434) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x438) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x418) + 0x60))(0);
  if (*(int *)(param_1 + 0x7e4) != 0) {
    (**(code **)(**(int **)(param_1 + 0x7e4) + 0x60))(0);
  }
  (**(code **)(**(int **)(param_1 + 0x428) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x424) + 0x60))(0);
  if (DAT_005b892c == 2) {
    (**(code **)(**(int **)(param_1 + 0x420) + 0x60))(0);
  }
  (**(code **)(**(int **)(param_1 + 0x41c) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x42c) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x43c) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x440) + 0x60))(0);
  return;
}

