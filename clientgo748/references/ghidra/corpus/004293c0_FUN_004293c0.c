// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004293c0 | Name: FUN_004293c0


undefined4 __fastcall FUN_004293c0(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 4) + 0x24))(*(undefined4 *)(param_1 + 4));
  }
  return uVar1;
}

