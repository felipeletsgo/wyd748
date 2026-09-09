// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00524b16 | Name: FUN_00524b16


undefined4 __fastcall FUN_00524b16(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    iVar2 = __ftol();
    iVar3 = __ftol();
    iVar4 = __ftol();
    iVar5 = __ftol();
    if ((iVar2 == iVar4) && (iVar3 == iVar5)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

