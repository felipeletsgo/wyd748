// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004525a1 | Name: FUN_004525a1


undefined4 __thiscall FUN_004525a1(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'f') || (param_2 == 'F')) {
    FUN_0044fc4b(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

