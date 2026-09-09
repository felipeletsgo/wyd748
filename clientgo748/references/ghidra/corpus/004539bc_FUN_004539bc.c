// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004539bc | Name: FUN_004539bc


undefined4 __thiscall FUN_004539bc(int param_1,char param_2)

{
  undefined4 uVar1;
  
  if (DAT_005b892c == 2) {
    if ((param_2 == 'a') || (param_2 == 'A')) {
      if (DAT_005d03f4 != 1) {
        *(undefined4 *)(*(int *)(param_1 + 0x27c9c) + 0x2c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x27ca0) + 0x2c) = 0;
      }
      else {
        *(undefined4 *)(*(int *)(param_1 + 0x27c9c) + 0x2c) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x27ca0) + 0x2c) = 0;
      }
      DAT_005d03f4 = (uint)(DAT_005d03f4 != 1);
      (**(code **)(**(int **)(param_1 + 0x27c9c) + 0x54))();
      (**(code **)(**(int **)(param_1 + 0x27ca0) + 0x54))();
      uVar1 = 1;
    }
    else if ((param_2 == 'd') || (param_2 == 'D')) {
      if (DAT_005d03f4 == 2) {
        DAT_005d03f4 = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x27c9c) + 0x2c) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x27ca0) + 0x2c) = 0;
      }
      else {
        DAT_005d03f4 = 2;
        *(undefined4 *)(*(int *)(param_1 + 0x27c9c) + 0x2c) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x27ca0) + 0x2c) = 1;
      }
      (**(code **)(**(int **)(param_1 + 0x27c9c) + 0x54))();
      (**(code **)(**(int **)(param_1 + 0x27ca0) + 0x54))();
      uVar1 = 1;
    }
    else {
      uVar1 = 0;
    }
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

