// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059acb2 | Name: FUN_0059acb2


undefined4 FUN_0059acb2(int *param_1)

{
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  
  piVar1 = (int *)*param_1;
  if (((*piVar1 == -0x1f928c9d) && (piVar1[4] == 3)) && (piVar1[5] == 0x19930520)) {
    uVar2 = FUN_00593e5a();
    return uVar2;
  }
  if ((DAT_013b8344 != (code *)0x0) && (iVar3 = FUN_0059ad4e(DAT_013b8344), iVar3 != 0)) {
    uVar2 = (*DAT_013b8344)(param_1);
    return uVar2;
  }
  return 0;
}

