// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045398c | Name: FUN_0045398c


undefined4 __thiscall FUN_0045398c(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'm') || (param_2 == 'M')) {
    FUN_0044ca65(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

