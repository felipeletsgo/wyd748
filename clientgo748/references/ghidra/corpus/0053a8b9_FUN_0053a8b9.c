// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053a8b9 | Name: FUN_0053a8b9


void __thiscall FUN_0053a8b9(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  
  iVar1 = __ftol();
  iVar2 = __ftol();
  iVar1 = iVar1 / 2;
  iVar2 = iVar2 / 2;
  if ((((-1 < iVar1) && (iVar1 < 0x40)) && (-1 < iVar2)) && (iVar2 < 0x40)) {
    *(undefined4 *)(param_1 + 0xd4 + (iVar1 + iVar2 * 0x40) * 0xc) = param_4;
  }
  return;
}

