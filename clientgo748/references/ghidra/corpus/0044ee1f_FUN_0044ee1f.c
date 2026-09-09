// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ee1f | Name: FUN_0044ee1f


void __thiscall FUN_0044ee1f(int param_1,undefined1 param_2)

{
  int *piVar1;
  
  *(undefined1 *)(param_1 + 0x26ebe) = param_2;
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x136);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x8c))(*(char *)(param_1 + 0x26ebe) == '\0');
  }
  return;
}

