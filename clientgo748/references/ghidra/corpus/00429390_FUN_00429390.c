// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429390 | Name: FUN_00429390


undefined4 __fastcall FUN_00429390(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 4) + 0x1c))(*(undefined4 *)(param_1 + 4));
  }
  return uVar1;
}

