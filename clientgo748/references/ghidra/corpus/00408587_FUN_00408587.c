// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00408587 | Name: FUN_00408587


void __thiscall FUN_00408587(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0xe; local_8 = local_8 + 1) {
    (**(code **)(**(int **)(param_1 + 100 + local_8 * 4) + 0x50))(param_2);
  }
  return;
}

