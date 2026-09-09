// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594ae2 | Name: FUN_00594ae2


undefined4 * FUN_00594ae2(void)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  undefined4 *puVar4;
  
  puVar4 = (undefined4 *)0x0;
  FUN_00596aad(2);
  iVar3 = 0;
  if (0 < DAT_013b9760) {
    do {
      iVar1 = *(int *)(DAT_013b8740 + iVar3 * 4);
      if (iVar1 == 0) {
        iVar3 = iVar3 * 4;
        pvVar2 = _malloc(0x38);
        *(void **)(iVar3 + DAT_013b8740) = pvVar2;
        if (*(int *)(iVar3 + DAT_013b8740) != 0) {
          InitializeCriticalSection((LPCRITICAL_SECTION)(*(int *)(iVar3 + DAT_013b8740) + 0x20));
          EnterCriticalSection((LPCRITICAL_SECTION)(*(int *)(iVar3 + DAT_013b8740) + 0x20));
          puVar4 = *(undefined4 **)(iVar3 + DAT_013b8740);
LAB_00594b86:
          if (puVar4 != (undefined4 *)0x0) {
            puVar4[4] = 0xffffffff;
            puVar4[1] = 0;
            puVar4[3] = 0;
            puVar4[2] = 0;
            *puVar4 = 0;
            puVar4[7] = 0;
          }
        }
        break;
      }
      if ((*(byte *)(iVar1 + 0xc) & 0x83) == 0) {
        FUN_00591be2(iVar3,iVar1);
        iVar1 = *(int *)(DAT_013b8740 + iVar3 * 4);
        if ((*(byte *)(iVar1 + 0xc) & 0x83) == 0) {
          puVar4 = *(undefined4 **)(DAT_013b8740 + iVar3 * 4);
          goto LAB_00594b86;
        }
        FUN_00591c34(iVar3,iVar1);
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < DAT_013b9760);
  }
  FUN_00596b0e(2);
  return puVar4;
}

