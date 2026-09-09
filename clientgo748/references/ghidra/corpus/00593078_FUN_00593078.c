// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593078 | Name: FUN_00593078


void FUN_00593078(char *param_1)

{
  char cVar1;
  char cVar2;
  int iVar3;
  uint uVar4;
  
  iVar3 = FUN_0059a5fa((int)*param_1);
  if (iVar3 != 0x65) {
    do {
      param_1 = param_1 + 1;
      if (DAT_005cc31c < 2) {
        uVar4 = (byte)PTR_DAT_005cc110[*param_1 * 2] & 4;
      }
      else {
        uVar4 = FUN_005980ea((int)*param_1,4);
      }
    } while (uVar4 != 0);
  }
  cVar2 = *param_1;
  *param_1 = DAT_005cc320;
  do {
    param_1 = param_1 + 1;
    cVar1 = *param_1;
    *param_1 = cVar2;
    cVar2 = cVar1;
  } while (*param_1 != '\0');
  return;
}

