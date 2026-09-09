// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00421b0f | Name: FUN_00421b0f


undefined4 FUN_00421b0f(LPCSTR param_1,int param_2)

{
  undefined4 uVar1;
  LPSTR pCVar2;
  int iVar3;
  
  if (param_1 == (LPCSTR)0x0) {
    uVar1 = 1;
  }
  else {
    pCVar2 = CharNextA(param_1 + param_2);
    iVar3 = (int)pCVar2 - (int)(param_1 + param_2);
    pCVar2 = CharPrevA(param_1,param_1 + iVar3 + param_2);
    if ((iVar3 == 1) && (param_1 + ((iVar3 + param_2) - (int)pCVar2) == (LPCSTR)0x2)) {
      uVar1 = 0;
    }
    else if ((iVar3 == 2) && (param_1 + ((iVar3 + param_2) - (int)pCVar2) == (LPCSTR)0x1)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  return uVar1;
}

