// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593479 | Name: FUN_00593479


void FUN_00593479(char *param_1,int param_2)

{
  size_t sVar1;
  
  if (param_2 != 0) {
    sVar1 = _strlen(param_1);
    FUN_00591fd0(param_1 + param_2,param_1,sVar1 + 1);
  }
  return;
}

