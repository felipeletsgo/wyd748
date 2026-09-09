// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00538536 | Name: FUN_00538536


undefined4 __fastcall FUN_00538536(int param_1)

{
  longlong lVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0xa8) = uVar2;
  if ((*(int *)(DAT_0067cf38 + 0x4c) != 0) && (*(int *)(DAT_0067cf38 + 0x40) == param_1)) {
    lVar1 = (longlong)DAT_005ba694;
    __ftol();
    iVar3 = __ftol();
    iVar4 = FUN_004b8c45(0xb);
    if (iVar4 != 0) {
      uVar2 = __ftol();
      *(undefined4 *)(*(int *)(iVar4 + 4) + 4) = uVar2;
      uVar2 = __ftol(DAT_005ba698 - iVar3 / (int)(0x200 / lVar1));
      *(undefined4 *)(*(int *)(iVar4 + 4) + 8) = uVar2;
      uVar2 = __ftol();
      *(undefined4 *)(*(int *)(iVar4 + 4) + 0xc) = uVar2;
      uVar2 = __ftol();
      *(undefined4 *)(*(int *)(iVar4 + 4) + 0x10) = uVar2;
    }
  }
  return 1;
}

