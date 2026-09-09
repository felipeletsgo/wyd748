// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00597b52 | Name: FUN_00597b52


undefined4 FUN_00597b52(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  do {
    *param_1 = *param_1 + 1;
    uVar1 = FUN_00597b21(param_2);
    iVar2 = FUN_005912ad(uVar1);
  } while (iVar2 != 0);
  return uVar1;
}

