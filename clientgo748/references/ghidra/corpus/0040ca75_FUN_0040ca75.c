// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040ca75 | Name: FUN_0040ca75


undefined4 __thiscall FUN_0040ca75(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 300) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 300) + 0x3c))(param_2);
  }
  return uVar1;
}

