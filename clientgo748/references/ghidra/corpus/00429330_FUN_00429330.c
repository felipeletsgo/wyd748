// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429330 | Name: FUN_00429330


undefined4 __thiscall FUN_00429330(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xc) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 0xc) + 0x1c))(*(undefined4 *)(param_1 + 0xc),param_2);
  }
  return uVar1;
}

