// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00447469 | Name: FUN_00447469


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00447469(int param_1,int param_2)

{
  *(int *)(param_1 + 0x27ae4) = param_2;
  if (*(int *)(param_1 + 0x27b24) != 0) {
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(param_1 + 0x27b24) + 0x70))
                (_DAT_005a4350 -
                 ((float)param_2 * _DAT_005a3660 + (float)(param_2 + -4) * _DAT_005a3f84),
                 *(undefined4 *)(*(int *)(param_1 + 0x27b24) + 0x50));
      (**(code **)(**(int **)(param_1 + 0x27b24) + 0x74))
                ((float)param_2 * _DAT_005a3660 + (float)*(int *)(param_1 + 0x27ae4) * _DAT_005a3f84
                 ,0x40800000);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x27b24) + 0x70))
                ((float)((8 - *(int *)(param_1 + 0x27ae4)) * 0x18) + _DAT_005a434c,0x440d8000);
      (**(code **)(**(int **)(param_1 + 0x27b24) + 0x74))
                ((float)*(int *)(param_1 + 0x27ae4) * _DAT_005a418c,0x40000000);
    }
  }
  return;
}

