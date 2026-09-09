// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1acf | Name: FUN_004b1acf


int FUN_004b1acf(char param_1)

{
  int iVar1;
  
  if ((param_1 < '\0') || ('\t' < param_1)) {
    if ((param_1 < '\n') || ('\x0f' < param_1)) {
      iVar1 = CONCAT31(param_1 >> 7,0x30);
    }
    else {
      iVar1 = param_1 + 0x37;
    }
  }
  else {
    iVar1 = param_1 + 0x30;
  }
  return iVar1;
}

