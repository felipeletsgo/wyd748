// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054567a | Name: FUN_0054567a


undefined4 __thiscall FUN_0054567a(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x28); local_8 = local_8 + 1) {
    (**(code **)(**(int **)(param_1 + 0x2c + local_8 * 4) + 0x20))(param_2);
  }
  return 1;
}

