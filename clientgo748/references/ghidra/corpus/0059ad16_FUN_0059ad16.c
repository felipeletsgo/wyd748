// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059ad16 | Name: FUN_0059ad16


bool FUN_0059ad16(void *param_1,UINT_PTR param_2)

{
  BOOL BVar1;
  
  BVar1 = IsBadReadPtr(param_1,param_2);
  return BVar1 == 0;
}

