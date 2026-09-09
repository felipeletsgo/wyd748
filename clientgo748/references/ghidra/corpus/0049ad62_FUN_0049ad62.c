// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ad62 | Name: FUN_0049ad62


int __fastcall FUN_0049ad62(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = __ftol();
  iVar2 = __ftol();
  if ((((iVar1 < 0) || (iVar2 < 0)) || (0xff < iVar1)) || (0xff < iVar2)) {
    iVar1 = -10000;
  }
  else {
    iVar1 = (int)*(char *)(param_1 + 0x10094 + iVar2 * 0x100 + iVar1);
  }
  return iVar1;
}

