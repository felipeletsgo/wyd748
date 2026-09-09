// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004245db | Name: FUN_004245db


undefined4 __fastcall FUN_004245db(SOCKET *param_1)

{
  SOCKET SVar1;
  int iVar2;
  undefined4 uVar3;
  
  SVar1 = param_1[4];
  iVar2 = recv(*param_1,(char *)(param_1[2] + param_1[4]),0x20000 - SVar1,0);
  if (iVar2 == -1) {
    uVar3 = 0;
  }
  else if (iVar2 == 0x20000 - SVar1) {
    uVar3 = 0xffffffff;
  }
  else {
    param_1[4] = param_1[4] + iVar2;
    uVar3 = 1;
  }
  return uVar3;
}

