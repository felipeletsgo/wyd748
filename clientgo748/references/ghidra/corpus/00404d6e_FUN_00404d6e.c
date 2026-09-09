// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00404d6e | Name: FUN_00404d6e


void __thiscall FUN_00404d6e(undefined4 param_1,int param_2)

{
  int *piVar1;
  
  FUN_0040c090(param_2);
  if (DAT_0067cf38 != (int *)0x0) {
    if (param_2 == 1) {
      piVar1 = (int *)(**(code **)(*DAT_0067cf38 + 0x38))(param_1);
      (**(code **)(*piVar1 + 0x40))();
    }
    else {
      piVar1 = (int *)(**(code **)(*DAT_0067cf38 + 0x38))(0);
      (**(code **)(*piVar1 + 0x40))();
    }
  }
  return;
}

