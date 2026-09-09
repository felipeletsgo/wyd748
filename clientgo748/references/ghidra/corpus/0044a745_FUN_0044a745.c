// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044a745 | Name: FUN_0044a745


void __thiscall FUN_0044a745(int param_1,int param_2,char param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  DAT_005b12bc = 0xffff;
  if (((param_2 != 0) || (*(int *)(*(int *)(param_1 + 0x27be4) + 0x28) != 1)) ||
     ((*(int *)(*(int *)(param_1 + 0x289a0) + 0x388) == 0 &&
      (*(char *)(*(int *)(param_1 + 0x289a0) + 0x369) != '\x01')))) {
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
      FUN_004484f3(0);
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x27b38) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x27b38) + 0x28) == 1)) {
      FUN_0044ae38(0,0);
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x27b34) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x27b34) + 0x28) == 1)) {
      FUN_00447691();
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x285d4) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x285d4) + 0x28) == 1)) {
      FUN_004481c5(0);
    }
    if (param_2 == 1) {
      if (*(int *)(param_1 + 0x27be4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27be4) + 0x60))(1);
        if (param_3 == '\x01') {
          (**(code **)(**(int **)(param_1 + 0x2899c) + 0x60))(1);
          (**(code **)(**(int **)(param_1 + 0x289a0) + 0x60))(0);
        }
        else if (param_3 == '\x02') {
          (**(code **)(**(int **)(param_1 + 0x2899c) + 0x60))(0);
          (**(code **)(**(int **)(param_1 + 0x289a0) + 0x60))(1);
        }
      }
      if (DAT_005b892c != 2) {
        uVar2 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar2;
      }
      *(undefined4 *)(*(int *)(param_1 + 0x28740) + 0x400) = 8;
      FUN_00447f6f(0);
    }
    else {
      if (*(int *)(param_1 + 0x27be4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27be4) + 0x60))
                  (0,param_1,*(undefined4 *)(param_1 + 0x27be4));
      }
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    }
  }
  return;
}

