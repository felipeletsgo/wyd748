// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c15fc | Name: FUN_004c15fc


int * __thiscall FUN_004c15fc(int *param_1,int param_2)

{
  int *piVar1;
  
  piVar1 = param_1;
  if (((*param_1 != param_2) &&
      ((param_1[0x23] == 0 || (piVar1 = (int *)FUN_004c15fc(param_2), piVar1 == (int *)0x0)))) &&
     ((param_1[0x22] == 0 || (piVar1 = (int *)FUN_004c15fc(param_2), piVar1 == (int *)0x0)))) {
    piVar1 = (int *)0x0;
  }
  return piVar1;
}

