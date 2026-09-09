// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004525d1 | Name: FUN_004525d1


undefined4 __thiscall FUN_004525d1(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'v') || (param_2 == 'V')) {
    FUN_004502a7(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

