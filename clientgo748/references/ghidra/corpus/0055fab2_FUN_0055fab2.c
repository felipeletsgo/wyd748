// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fab2 | Name: FUN_0055fab2


int * FUN_0055fab2(int param_1)

{
  int *piVar1;
  
  piVar1 = &DAT_005a6160;
  while( true ) {
    if (PTR_DAT_005c5084 <= piVar1) {
      return &DAT_005a6138;
    }
    if (param_1 == *piVar1) break;
    piVar1 = piVar1 + 9;
  }
  return piVar1;
}

