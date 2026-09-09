// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ef8f | Name: FUN_0054ef8f


undefined4 FUN_0054ef8f(char *param_1)

{
  char cVar1;
  bool bVar2;
  size_t sVar3;
  int local_8;
  
  sVar3 = _strlen(param_1);
  param_1[0xf] = '\0';
  param_1[0xe] = '\0';
  local_8 = 0;
  do {
    if ((int)sVar3 <= local_8) {
      return 1;
    }
    cVar1 = param_1[local_8];
    if (((cVar1 < 'a') || ('z' < cVar1)) && ((cVar1 < 'A' || ('Z' < cVar1)))) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    if (!bVar2) {
      if ((cVar1 < '0') || ('9' < cVar1)) {
        bVar2 = false;
      }
      else {
        bVar2 = true;
      }
      if (((((!bVar2) && (cVar1 != '-')) && (cVar1 != '[')) && ((cVar1 != ']' && (cVar1 != '{'))))
         && ((cVar1 != '}' && ((cVar1 != '(' && (cVar1 != ')')))))) {
        return 0;
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}

