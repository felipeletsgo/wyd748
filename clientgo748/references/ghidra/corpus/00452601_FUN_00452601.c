// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452601 | Name: FUN_00452601


undefined4 __thiscall FUN_00452601(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'q') || (param_2 == 'Q')) {
    FUN_0044effc(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

