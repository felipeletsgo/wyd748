// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a24b4 | Name: FUN_004a24b4


void __thiscall FUN_004a24b4(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x8c) = 1;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x2009c) = uVar1;
  FUN_0049e1e3(param_2);
  *(undefined2 *)(param_1 + 0x20098) = 1;
  return;
}

