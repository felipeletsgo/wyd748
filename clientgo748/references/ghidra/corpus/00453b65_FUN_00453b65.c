// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00453b65 | Name: FUN_00453b65


undefined4 __thiscall FUN_00453b65(int param_1,char param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  
  if (DAT_005b892c == 2) {
    if (*(int *)(param_1 + 0x27b28) == 0) {
      return 0;
    }
    if (*(int *)(param_1 + 0x27b2c) == 0) {
      return 0;
    }
  }
  if (param_2 == '\r') {
    piVar1 = *(int **)(param_1 + 0x27b2c);
    iVar2 = FUN_00493c10();
    if (iVar2 == 0) {
      (**(code **)(*piVar1 + 0x60))(1);
      if (DAT_005b892c == 2) {
        (**(code **)(**(int **)(param_1 + 0x27b28) + 0x60))(1);
      }
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(piVar1);
    }
    else {
      (**(code **)(*piVar1 + 0x60))(0);
      if (DAT_005b892c == 2) {
        (**(code **)(**(int **)(param_1 + 0x27b28) + 0x60))(0);
      }
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(0);
    }
    uVar3 = 1;
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

