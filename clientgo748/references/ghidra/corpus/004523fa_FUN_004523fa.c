// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004523fa | Name: FUN_004523fa


undefined4 __thiscall FUN_004523fa(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((((param_2 == 'k') || (param_2 == 'K')) || (param_2 == '`')) || (param_2 == '~')) {
    FUN_0044ecae(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

