// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ed63 | Name: FUN_0044ed63


void __fastcall FUN_0044ed63(int param_1)

{
  int *piVar1;
  
  *(uint *)(param_1 + 0x50) = (uint)(*(int *)(param_1 + 0x50) == 0);
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x133);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x8c))(*(int *)(param_1 + 0x50) == 0);
  }
  return;
}

