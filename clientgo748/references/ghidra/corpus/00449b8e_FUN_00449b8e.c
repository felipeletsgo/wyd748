// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00449b8e | Name: FUN_00449b8e


void __thiscall FUN_00449b8e(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  DAT_005b12bc = 0xffff;
  iVar1 = FUN_0040c0f0();
  if (iVar1 == 1) {
    FUN_00447594();
  }
  if (((param_2 == 1) && (*(int *)(param_1 + 0x27c84) != 0)) &&
     (*(int *)(*(int *)(param_1 + 0x27c84) + 0x28) == 1)) {
    FUN_0044c53f();
  }
  if (((param_2 == 1) && (*(int *)(param_1 + 0x27c7c) != 0)) &&
     (*(int *)(*(int *)(param_1 + 0x27c7c) + 0x28) == 1)) {
    FUN_00447c73();
  }
  if (((param_2 == 1) && (*(int *)(param_1 + 0x285d8) != 0)) &&
     (*(int *)(*(int *)(param_1 + 0x285d8) + 0x28) == 1)) {
    FUN_004484f3(1);
  }
  if (((param_2 == 1) && (*(int *)(param_1 + 0x27b38) != 0)) &&
     (*(int *)(*(int *)(param_1 + 0x27b38) + 0x28) == 1)) {
    FUN_0044ae38(0,0);
  }
  if (DAT_005b892c == 2) {
    if (param_2 == 1) {
      if ((*(int *)(param_1 + 0x27b34) != 0) && (*(int *)(*(int *)(param_1 + 0x27b34) + 0x28) == 0))
      {
        FUN_00447691();
      }
      (**(code **)(**(int **)(param_1 + 0x27be8) + 0x60))(1);
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      (**(code **)(*DAT_005ccec0 + 0x98))();
      *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0x13;
      FUN_00447f6f(0);
    }
    else {
      if ((*(int *)(param_1 + 0x27b34) != 0) && (*(int *)(*(int *)(param_1 + 0x27b34) + 0x28) == 1))
      {
        FUN_00447691();
      }
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      (**(code **)(**(int **)(param_1 + 0x27be8) + 0x60))(0);
      FUN_00448e0b();
    }
  }
  else if (param_2 == 1) {
    if ((*(int *)(param_1 + 0x27b34) != 0) && (*(int *)(*(int *)(param_1 + 0x27b34) + 0x28) == 0)) {
      FUN_00447691();
    }
    (**(code **)(**(int **)(param_1 + 0x27be8) + 0x60))(1);
    uVar2 = __ftol();
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar2;
    (**(code **)(*DAT_005ccec0 + 0x98))();
    *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0x13;
    FUN_00447f6f(0);
  }
  else {
    if ((*(int *)(param_1 + 0x27b34) != 0) && (*(int *)(*(int *)(param_1 + 0x27b34) + 0x28) == 1)) {
      FUN_00447691();
    }
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    (**(code **)(**(int **)(param_1 + 0x27be8) + 0x60))(0);
    FUN_00448e0b();
  }
  return;
}

