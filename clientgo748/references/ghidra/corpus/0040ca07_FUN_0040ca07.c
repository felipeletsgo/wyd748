// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040ca07 | Name: FUN_0040ca07


undefined4 __thiscall FUN_0040ca07(int param_1,undefined1 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 300) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 300) + 0x14))(param_2,param_3);
  }
  return uVar1;
}

