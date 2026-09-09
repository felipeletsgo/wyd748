// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004062a7 | Name: FUN_004062a7


void __fastcall FUN_004062a7(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x6c) < 0) {
    *(undefined4 *)(param_1 + 0x6c) = 0;
  }
  if (*(int *)(param_1 + 0x70) < *(int *)(param_1 + 0x6c)) {
    *(undefined4 *)(param_1 + 0x6c) = *(undefined4 *)(param_1 + 0x70);
  }
  if (0 < *(int *)(param_1 + 0x70)) {
    uVar1 = __ftol();
    *(undefined4 *)(param_1 + 0x74) = uVar1;
    if (*(int *)(param_1 + 0x68) == 0) {
      (**(code **)(**(int **)(param_1 + 0x90) + 0x70))
                (*(undefined4 *)(*(int *)(param_1 + 0x90) + 0x4c),
                 (float)*(int *)(param_1 + 0x74) + *(float *)(param_1 + 0x80));
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x90) + 0x70))
                ((float)*(int *)(param_1 + 0x74) + *(float *)(param_1 + 0x80),0);
    }
    if (*(int *)(param_1 + 0x5c) != 0) {
      (**(code **)**(undefined4 **)(param_1 + 0x5c))
                (*(undefined4 *)(param_1 + 0x44),*(undefined4 *)(param_1 + 0x6c));
    }
  }
  return;
}

