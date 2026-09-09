// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f6e5 | Name: FUN_0058f6e5


undefined4 FUN_0058f6e5(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00594ae2();
  if (iVar1 == 0) {
    return 0;
  }
  uVar2 = FUN_00594972(param_1,param_2,param_3,iVar1);
  FUN_00591c05(iVar1);
  return uVar2;
}

