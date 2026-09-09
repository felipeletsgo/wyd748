// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452571 | Name: FUN_00452571


undefined4 __thiscall FUN_00452571(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'r') || (param_2 == 'R')) {
    FUN_00450962(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

