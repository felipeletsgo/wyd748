// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ee77 | Name: FUN_0044ee77


void __thiscall FUN_0044ee77(int param_1,undefined1 param_2)

{
  int *piVar1;
  
  *(undefined1 *)(param_1 + 0x26ebf) = param_2;
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x135);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x8c))(*(char *)(param_1 + 0x26ebf) == '\0');
  }
  return;
}

