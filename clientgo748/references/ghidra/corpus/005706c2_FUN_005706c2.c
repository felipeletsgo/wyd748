// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005706c2 | Name: FUN_005706c2


undefined4 FUN_005706c2(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xc) == 0) {
    uVar1 = 0x80004005;
  }
  else {
    (**(code **)(**(int **)(param_1 + 8) + 0xd8))
              (*(int **)(param_1 + 8),*(undefined4 *)(param_1 + 0x10));
    *(undefined4 *)(param_1 + 0xc) = 0;
    uVar1 = 0;
  }
  return uVar1;
}

