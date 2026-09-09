// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059ddd0 | Name: FUN_0059ddd0


undefined4 FUN_0059ddd0(int param_1,int *param_2,int *param_3)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  bool bVar4;
  
  iVar1 = 4;
  bVar4 = true;
  piVar2 = param_2;
  piVar3 = &DAT_005aa898;
  do {
    if (iVar1 == 0) break;
    iVar1 = iVar1 + -1;
    bVar4 = *piVar2 == *piVar3;
    piVar2 = piVar2 + 1;
    piVar3 = piVar3 + 1;
  } while (bVar4);
  if (!bVar4) {
    iVar1 = 4;
    bVar4 = true;
    piVar2 = param_2;
    piVar3 = &DAT_005aa8e8;
    do {
      if (iVar1 == 0) break;
      iVar1 = iVar1 + -1;
      bVar4 = *piVar2 == *piVar3;
      piVar2 = piVar2 + 1;
      piVar3 = piVar3 + 1;
    } while (bVar4);
    if (!bVar4) {
      iVar1 = 4;
      bVar4 = true;
      piVar2 = param_2;
      piVar3 = &DAT_005aa8b8;
      do {
        if (iVar1 == 0) break;
        iVar1 = iVar1 + -1;
        bVar4 = *piVar2 == *piVar3;
        piVar2 = piVar2 + 1;
        piVar3 = piVar3 + 1;
      } while (bVar4);
      if (bVar4) {
        *param_3 = param_1 + 4;
        return 0;
      }
      iVar1 = 4;
      bVar4 = true;
      piVar2 = &DAT_005aa8a8;
      do {
        if (iVar1 == 0) break;
        iVar1 = iVar1 + -1;
        bVar4 = *param_2 == *piVar2;
        param_2 = param_2 + 1;
        piVar2 = piVar2 + 1;
      } while (bVar4);
      if (bVar4) {
        *param_3 = param_1 + 0x10;
        return 0;
      }
      *param_3 = 0;
      return 0x80004002;
    }
  }
  *param_3 = param_1;
  return 0;
}

