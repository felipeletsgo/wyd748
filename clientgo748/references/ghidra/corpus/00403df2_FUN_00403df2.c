// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403df2 | Name: FUN_00403df2


void __thiscall FUN_00403df2(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  *(undefined4 *)(param_1 + 0x1f8) = param_3;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 500) = uVar1;
  (**(code **)(**(int **)(param_1 + 0x1e4) + 0x80))(param_2,0);
  return;
}

