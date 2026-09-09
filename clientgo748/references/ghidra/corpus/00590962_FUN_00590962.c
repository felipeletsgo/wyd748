// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590962 | Name: FUN_00590962


undefined4 FUN_00590962(char *param_1)

{
  char *pcVar1;
  size_t sVar2;
  char *pcVar3;
  char cVar4;
  
  sVar2 = _strlen(param_1);
  if (((4 < sVar2) && ((*param_1 == '\\' || (*param_1 == '/')))) &&
     ((param_1[1] == '\\' || (param_1[1] == '/')))) {
    pcVar3 = param_1 + 3;
    cVar4 = param_1[3];
    while (((cVar4 != '\0' && (cVar4 != '\\')) && (cVar4 != '/'))) {
      pcVar1 = pcVar3 + 1;
      pcVar3 = pcVar3 + 1;
      cVar4 = *pcVar1;
    }
    if ((*pcVar3 != '\0') && (pcVar3 = pcVar3 + 1, *pcVar3 != '\0')) {
      for (; (cVar4 = *pcVar3, cVar4 != '\0' && ((cVar4 != '\\' && (cVar4 != '/'))));
          pcVar3 = pcVar3 + 1) {
      }
      if ((*pcVar3 == '\0') || (pcVar3[1] == '\0')) {
        return 1;
      }
    }
  }
  return 0;
}

