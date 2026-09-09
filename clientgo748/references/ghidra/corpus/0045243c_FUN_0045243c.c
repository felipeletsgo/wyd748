// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045243c | Name: FUN_0045243c


undefined4 __thiscall FUN_0045243c(undefined4 param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'n') || (param_2 == 'N')) {
    FUN_0044ed63(param_1);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

