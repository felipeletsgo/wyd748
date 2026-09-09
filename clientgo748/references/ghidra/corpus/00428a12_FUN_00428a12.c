// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428a12 | Name: FUN_00428a12


void FUN_00428a12(char *param_1)

{
  int iVar1;
  size_t _Size;
  size_t sVar2;
  int local_114;
  undefined4 local_110;
  undefined1 local_10c [256];
  void *local_c;
  size_t local_8;
  
  local_8 = _strlen(param_1);
  FUN_0058f078(local_10c,param_1);
  FUN_0058f078(local_10c + (local_8 - 3),&PTR_DAT_005b4788);
  iVar1 = FUN_0058fb07(local_10c,0x8000,0);
  if (iVar1 != -1) {
    _Size = FUN_0058f83f(iVar1);
    local_c = _malloc(_Size);
    FUN_0058f8c9(iVar1,local_c,_Size);
    FUN_0058f75f(iVar1);
    local_110 = &DAT_005b478c;
    sVar2 = _strlen(&DAT_005b478c);
    for (local_114 = 0; local_114 < (int)_Size; local_114 = local_114 + 1) {
      *(char *)((int)local_c + local_114) =
           *(char *)((int)local_c + local_114) - local_110[local_114 % (int)sVar2];
    }
    iVar1 = FUN_0058f716(param_1,&DAT_005b4838);
    if (iVar1 != 0) {
      FUN_005904b6(local_c,_Size,1,iVar1);
      FUN_0058f62c(iVar1);
    }
    FUN_0058ffd8(local_c);
  }
  return;
}

