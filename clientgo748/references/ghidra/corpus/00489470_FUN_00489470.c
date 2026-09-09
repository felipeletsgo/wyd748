// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489470 | Name: FUN_00489470


undefined4 __thiscall FUN_00489470(int param_1,int param_2)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 0x27d70) = *(undefined4 *)(param_2 + 0x10);
  *(undefined2 *)(param_1 + 0x27d74) = *(undefined2 *)(param_2 + 0xc);
  FUN_004729d4(1,0);
  if (*(short *)(DAT_013b71e8 + 0x7a8) == 0x1fd) {
    (**(code **)(**(int **)(param_1 + 0x27d80) + 0x60))(1);
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27d80) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x27c88) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27c88) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x27d38) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27d38) + 0x60))(1);
  }
  if (*(int *)(param_1 + 0x27d40) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27d40) + 0x60))(1);
  }
  if (*(int *)(param_1 + 0x27d44) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27d44) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x27d48) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27d48) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x27d58) != 0) {
    FUN_00408bed();
  }
  if (*(int *)(param_1 + 0x27d4c) != 0) {
    *(undefined4 *)(*(int *)(param_1 + 0x27d4c) + 0x1fc) = 0xffffffff;
  }
  if (*(int *)(param_1 + 0x27b34) != 0) {
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 1) {
      FUN_00447691();
    }
  }
  return 1;
}

