// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044eecf | Name: FUN_0044eecf


void __fastcall FUN_0044eecf(int param_1)

{
  int *piVar1;
  
  *(char *)(param_1 + 0x26eb4) = '\x01' - (*(char *)(param_1 + 0x26eb4) != '\0');
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x138);
  if (*(char *)(param_1 + 0x26eb4) == '\0') {
    *(undefined4 *)(param_1 + 0x26e90) = 0;
  }
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x8c))((int)*(char *)(param_1 + 0x26eb4));
  }
  (**(code **)(**(int **)(param_1 + 0x27b24) + 0x60))((int)*(char *)(param_1 + 0x26eb4));
  return;
}

