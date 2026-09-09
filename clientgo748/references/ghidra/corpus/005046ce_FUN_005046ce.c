// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005046ce | Name: FUN_005046ce


int __thiscall FUN_005046ce(int param_1,char *param_2,undefined4 *param_3)

{
  int iVar1;
  size_t sVar2;
  undefined4 uVar3;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    uVar3 = 0x3f800000;
    sVar2 = _strlen(param_2);
    if ((int)sVar2 < 0x29) {
      iVar1 = __ftol(sVar2 * 6,param_1,sVar2,uVar3);
      iVar1 = iVar1 + 0x14;
      *param_3 = 0x28;
    }
    else {
      iVar1 = __ftol();
    }
  }
  else {
    iVar1 = 0;
  }
  return iVar1;
}

