// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c544 | Name: FUN_0040c544


void __thiscall FUN_0040c544(int param_1,int param_2)

{
  int iVar1;
  
  if ((DAT_013b722c != 1) || ((param_2 != 0 && (*(int *)(param_2 + 0x40) == 0xd)))) {
    if (*(int *)(param_1 + 300) != 0) {
      (**(code **)(**(int **)(param_1 + 300) + 0x68))(0);
    }
    *(int *)(param_1 + 300) = param_2;
    if (*(int *)(param_1 + 300) != 0) {
      (**(code **)(**(int **)(param_1 + 300) + 0x68))(1);
    }
    if (DAT_0067cf38 != 0) {
      if ((*(int *)(param_1 + 300) == 0) || (*(int *)(*(int *)(param_1 + 300) + 0x40) != 0xd)) {
        (**(code **)(**(int **)(DAT_0067cf38 + 0x60) + 0x60))(0);
      }
      else {
        (**(code **)(**(int **)(DAT_0067cf38 + 0x60) + 0x60))(1);
        FUN_004af545();
      }
      if ((*(int *)(param_1 + 300) == 0) ||
         (iVar1 = (**(code **)(**(int **)(param_1 + 300) + 0x40))(), iVar1 == 0)) {
        if (DAT_005d0504 != 0) {
          FUN_004aeaeb();
        }
      }
      else if (DAT_005d0504 != 0) {
        FUN_004aea8a();
      }
    }
  }
  return;
}

