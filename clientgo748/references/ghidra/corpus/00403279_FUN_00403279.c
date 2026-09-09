// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403279 | Name: FUN_00403279


void __thiscall FUN_00403279(int param_1,undefined4 param_2)

{
  if (*(int *)(param_1 + 0x1f0) != 0) {
    (**(code **)(**(int **)(param_1 + 0x1f0) + 0x80))(param_2,0);
  }
  FUN_0058f078(param_1 + 0xaa,&DAT_005b10a8,param_2);
  FUN_00421b8f(param_1 + 0xaa,0xffffffff,0);
  return;
}

