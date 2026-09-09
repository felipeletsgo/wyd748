// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a7b4 | Name: FUN_0055a7b4


byte FUN_0055a7b4(char param_1)

{
  size_t sVar1;
  byte bVar2;
  
  if (param_1 == '=') {
    bVar2 = 0xfe;
  }
  else {
    for (bVar2 = 0; sVar1 = _strlen(s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914), bVar2 < sVar1;
        bVar2 = bVar2 + 1) {
      if (s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[bVar2] == param_1) {
        return bVar2;
      }
    }
    bVar2 = 0xff;
  }
  return bVar2;
}

