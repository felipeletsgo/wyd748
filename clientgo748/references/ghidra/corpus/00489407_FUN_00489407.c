// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489407 | Name: FUN_00489407


undefined4 __thiscall FUN_00489407(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x287f4) != 0) {
    if (*(int *)(param_2 + 0xc) == 1) {
      (**(code **)(**(int **)(param_1 + 0x287f4) + 0x8c))(1);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x287f4) + 0x8c))(0);
    }
  }
  return 1;
}

