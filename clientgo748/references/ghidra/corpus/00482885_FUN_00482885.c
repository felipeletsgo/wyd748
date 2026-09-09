// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00482885 | Name: FUN_00482885


undefined4 __thiscall FUN_00482885(int param_1,int param_2)

{
  *(undefined1 *)(param_2 + 0x1f) = 0;
  (**(code **)(**(int **)(param_1 + 0x27cf4) + 0x60))(1);
  FUN_0058f078(param_1 + 0x27254,param_2 + 0x10);
  return 1;
}

