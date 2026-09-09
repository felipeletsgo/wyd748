// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b92e0 | Name: FUN_004b92e0


int __thiscall FUN_004b92e0(int param_1,char *param_2)

{
  size_t sVar1;
  size_t sVar2;
  int iVar3;
  int local_10;
  
  if (param_2 != (char *)0x0) {
    sVar1 = _strlen(param_2);
    for (local_10 = 0; local_10 < 0x200; local_10 = local_10 + 1) {
      sVar2 = _strlen((char *)(param_1 + 0x22328 + local_10 * 0x108));
      FUN_0058f078(param_2 + (sVar1 - 3),param_1 + local_10 * 0x108 + 0x22325 + sVar2);
      iVar3 = _strcmp(param_2,(char *)(param_1 + 0x22328 + local_10 * 0x108));
      if (iVar3 == 0) {
        return local_10;
      }
    }
  }
  return -1;
}

