// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578d6d | Name: FUN_00578d6d


void FUN_00578d6d(int param_1)

{
  int iVar1;
  
  iVar1 = 1;
  do {
    FUN_00578c8c(param_1,iVar1);
    iVar1 = iVar1 + -1;
  } while (-1 < iVar1);
  FUN_0058a361(param_1,*(undefined4 *)(param_1 + 4),0x50);
  *(undefined4 *)(param_1 + 4) = 0;
  FUN_0058ea01(param_1);
  return;
}

