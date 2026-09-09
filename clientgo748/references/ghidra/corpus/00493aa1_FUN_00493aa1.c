// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00493aa1 | Name: FUN_00493aa1


void __thiscall FUN_00493aa1(int param_1,undefined4 param_2)

{
  int *piVar1;
  void *pvVar2;
  int local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fefb;
  local_10 = ExceptionList;
  piVar1 = *(int **)(param_1 + 0x27b2c);
  ExceptionList = &local_10;
  pvVar2 = operator_new(0xe50);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_2c = 0;
  }
  else {
    local_2c = FUN_00407203(param_2,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
  }
  local_8 = 0xffffffff;
  if (local_2c != 0) {
    FUN_00408d33(local_2c);
  }
  (**(code **)(*piVar1 + 0x90))(&DAT_005d04d4);
  (**(code **)(*piVar1 + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(0);
  if (*(int *)(DAT_013b71e8 + 0x1e428) != 1) {
    (**(code **)(*piVar1 + 0x90))(&DAT_005d04d8);
  }
  ExceptionList = local_10;
  return;
}

