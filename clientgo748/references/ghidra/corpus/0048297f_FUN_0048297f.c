// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0048297f | Name: FUN_0048297f


undefined4 FUN_0048297f(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  
  uVar1 = *(undefined4 *)(param_1 + 0xc);
  if (DAT_005ccf98 != 0) {
    iVar2 = FUN_00429a6d(uVar1);
    if (iVar2 != 0) {
      uVar3 = extraout_var;
      FUN_00429a6d(uVar1);
      iVar2 = FUN_0042afd0(uVar3);
      if (iVar2 == 0) {
        uVar4 = 0;
        uVar3 = 0;
        FUN_00429a6d(uVar1);
        FUN_0042ad2b(uVar3,uVar4);
      }
    }
  }
  return 1;
}

