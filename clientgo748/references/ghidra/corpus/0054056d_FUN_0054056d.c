// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054056d | Name: FUN_0054056d


undefined4 __fastcall FUN_0054056d(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(*param_1 + 0x70))();
  if ((iVar1 == 0) && (param_1[9] != 3)) {
    uVar2 = 0;
  }
  else {
    (**(code **)(*DAT_0092e654 + 8))();
    if ((param_1[9] == 0x1bb) &&
       ((iVar1 = __ftol(), iVar1 == 0x9ec && (iVar1 = __ftol(), iVar1 == 0x826)))) {
      param_1[0xc] = 0;
    }
    if (((param_1[9] == 0x1c6) && (iVar1 = __ftol(), iVar1 == 0x9ee)) &&
       (iVar1 = __ftol(), iVar1 == 0x82a)) {
      param_1[0xc] = 0;
    }
    if (((param_1[9] == 0x1c6) && (iVar1 = __ftol(), iVar1 == 0x9ec)) &&
       (iVar1 = __ftol(), iVar1 == 0x822)) {
      param_1[0xc] = 0;
    }
    if (((param_1[9] == 0x1c1) && (iVar1 = __ftol(), iVar1 == 0x9ec)) &&
       (iVar1 = __ftol(), iVar1 == 0x82e)) {
      param_1[0xc] = 0;
    }
    uVar2 = 1;
  }
  return uVar2;
}

