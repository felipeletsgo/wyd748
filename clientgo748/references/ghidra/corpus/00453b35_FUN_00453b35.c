// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00453b35 | Name: FUN_00453b35


undefined4 __thiscall FUN_00453b35(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'p') || (param_2 == 'P')) {
    FUN_0044da6f(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

