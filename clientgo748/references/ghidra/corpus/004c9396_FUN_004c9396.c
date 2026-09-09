// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c9396 | Name: FUN_004c9396


void __thiscall FUN_004c9396(int param_1,undefined4 param_2,undefined4 param_3)

{
  void *pvVar1;
  undefined4 local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0831;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x158) != 0) {
    ExceptionList = &local_10;
    if (*(undefined4 **)(param_1 + 0x158) != (undefined4 *)0x0) {
      ExceptionList = &local_10;
      (**(code **)**(undefined4 **)(param_1 + 0x158))(1);
    }
    *(undefined4 *)(param_1 + 0x158) = 0;
  }
  pvVar1 = operator_new(0xc0);
  local_8 = 0;
  if (pvVar1 == (void *)0x0) {
    local_2c = 0;
  }
  else {
    local_2c = FUN_004df250(param_3,7,0x3f800000);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x158) = local_2c;
  if (*(int *)(param_1 + 0x158) != 0) {
    (**(code **)(**(int **)(param_1 + 0x158) + 0x44))(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x158) + 0x6c) = 1;
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x158));
  }
  ExceptionList = local_10;
  return;
}

