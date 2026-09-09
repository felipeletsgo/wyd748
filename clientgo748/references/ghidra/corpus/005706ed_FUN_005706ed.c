// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005706ed | Name: FUN_005706ed


undefined4 FUN_005706ed(int param_1)

{
  int *piVar1;
  
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 != (int *)0x0) {
    if (*(int *)(param_1 + 0x14) != 0) {
      (**(code **)(*piVar1 + 0xe0))(piVar1,*(int *)(param_1 + 0x14));
      *(undefined4 *)(param_1 + 0x14) = 0;
    }
    if (*(int *)(param_1 + 0x10) != 0) {
      (**(code **)(**(int **)(param_1 + 8) + 0xe0))(*(int **)(param_1 + 8),*(int *)(param_1 + 0x10))
      ;
      *(undefined4 *)(param_1 + 0x10) = 0;
    }
  }
  return 0;
}

