// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056bb6e | Name: FUN_0056bb6e


undefined4 FUN_0056bb6e(void)

{
  int iVar1;
  void *pvVar2;
  void *pvVar3;
  undefined4 uVar4;
  int extraout_ECX;
  int unaff_EBP;
  
  FUN_00592900();
  if (*(int *)(extraout_ECX + 8) != *(int *)(*(int *)(unaff_EBP + 8) + 8)) {
    iVar1 = *(int *)(extraout_ECX + 0x1058);
    pvVar2 = operator_new(iVar1 << 4);
    *(void **)(unaff_EBP + -0x10) = pvVar2;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    pvVar3 = (void *)0x0;
    if (pvVar2 != (void *)0x0) {
      _vector_constructor_iterator_(pvVar2,0x10,iVar1,FUN_00430f10);
      pvVar3 = pvVar2;
    }
    *(void **)(extraout_ECX + 0x104c) = pvVar3;
    if (pvVar3 == (void *)0x0) {
      uVar4 = 0x8007000e;
      goto LAB_0056bbda;
    }
    *(undefined4 *)(extraout_ECX + 0x1048) = *(undefined4 *)(*(int *)(unaff_EBP + 8) + 8);
  }
  uVar4 = 0;
LAB_0056bbda:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar4;
}

