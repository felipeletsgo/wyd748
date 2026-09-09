// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053993d | Name: FUN_0053993d


int __fastcall FUN_0053993d(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  
  uVar1 = __ftol();
  iVar2 = __ftol();
  uVar3 = __ftol();
  iVar4 = __ftol(uVar3,uVar1);
  if ((((iVar2 < 0) || (iVar4 < 0)) || (0x7f < iVar2)) || (0x7f < iVar4)) {
    iVar2 = -10000;
  }
  else {
    iVar2 = (int)*(char *)(param_1 + 0xc0cc + iVar2 + iVar4 * 0x80);
  }
  return iVar2;
}

