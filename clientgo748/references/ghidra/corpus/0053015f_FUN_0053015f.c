// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053015f | Name: FUN_0053015f


undefined4 __fastcall FUN_0053015f(int param_1)

{
  void *pvVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a24ce;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x60) == 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x358);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_20 = 0;
    }
    else {
      local_20 = FUN_004bd420(param_1 + 0x160,param_1 + 0x180,*(undefined4 *)(param_1 + 0x5c),0,0,0,
                              0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x60) = local_20;
    if (*(int *)(param_1 + 0x60) != 0) {
      FUN_004be7b1();
      if (*(int *)(param_1 + 0x194) == 1) {
        if (*(short *)(param_1 + 0x162) == 3) {
          *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = 0x3f800000;
        }
        else {
          *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = 0x3f333333;
          *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = 0x3f333333;
          *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = 0x3f333333;
        }
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 10;
      }
      else if (*(int *)(param_1 + 0x194) == 2) {
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 4;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = 0x3e4ccccd;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = 0x3e4ccccd;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = 0x3e4ccccd;
      }
      else if (*(int *)(param_1 + 0x194) == 3) {
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 8;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = 0x3f000000;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = 0x3f000000;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = 0x3f000000;
      }
      else {
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 0xf;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = 0x3f000000;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = 0x3f000000;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = 0x3f000000;
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

