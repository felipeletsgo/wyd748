// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044edbc | Name: FUN_0044edbc


void __thiscall FUN_0044edbc(int param_1,undefined1 param_2)

{
  int *piVar1;
  
  if (DAT_005b892c != 2) {
    *(undefined1 *)(param_1 + 0x26ebd) = param_2;
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x137);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x8c))(*(char *)(param_1 + 0x26ebd) == '\0');
    }
  }
  return;
}

