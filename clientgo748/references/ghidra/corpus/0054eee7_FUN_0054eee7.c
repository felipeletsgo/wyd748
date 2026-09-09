// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054eee7 | Name: FUN_0054eee7


undefined4 FUN_0054eee7(char *param_1)

{
  char cVar1;
  size_t sVar2;
  undefined4 uVar3;
  int local_8;
  
  sVar2 = _strlen(param_1);
  if (((int)sVar2 < 4) || (0xf < (int)sVar2)) {
    uVar3 = 0;
  }
  else {
    for (local_8 = 0; local_8 < (int)sVar2; local_8 = local_8 + 1) {
      cVar1 = param_1[local_8];
      if (cVar1 < '\0') {
        local_8 = local_8 + 1;
      }
      else if ((((cVar1 < 'a') || ('z' < cVar1)) && ((cVar1 < 'A' || ('Z' < cVar1)))) &&
              (((cVar1 < '0' || ('9' < cVar1)) && (cVar1 != '-')))) {
        return 0;
      }
    }
    uVar3 = 1;
  }
  return uVar3;
}

