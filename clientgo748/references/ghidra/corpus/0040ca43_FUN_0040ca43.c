// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040ca43 | Name: FUN_0040ca43


undefined4 __fastcall FUN_0040ca43(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 300) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 300) + 0x38))();
  }
  return uVar1;
}

