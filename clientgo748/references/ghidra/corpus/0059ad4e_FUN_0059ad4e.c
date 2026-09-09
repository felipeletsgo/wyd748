// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059ad4e | Name: FUN_0059ad4e


bool FUN_0059ad4e(FARPROC param_1)

{
  BOOL BVar1;
  
  BVar1 = IsBadCodePtr(param_1);
  return BVar1 == 0;
}

