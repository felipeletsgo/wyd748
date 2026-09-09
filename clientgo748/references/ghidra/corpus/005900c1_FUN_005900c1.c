// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005900c1 | Name: FUN_005900c1


void FUN_005900c1(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00593d53();
  *(undefined4 *)(iVar1 + 0x14) = param_1;
  return;
}

