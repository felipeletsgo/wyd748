// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00557556 | Name: FUN_00557556


undefined4 FUN_00557556(int param_1,int param_2)

{
  undefined4 uVar1;
  LPSTR pCVar2;
  
  if (param_1 == 0) {
    uVar1 = 1;
  }
  else {
    pCVar2 = CharNextA((LPCSTR)(param_1 + param_2));
    if ((LPSTR)(param_1 + 1 + param_2) == pCVar2) {
      uVar1 = 1;
    }
    else {
      uVar1 = 0;
    }
  }
  return uVar1;
}

