// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585c78 | Name: FUN_00585c78


undefined4 FUN_00585c78(void)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  int unaff_EDI;
  
  iVar2 = *(int *)(unaff_EDI + 0x194);
  piVar1 = (int *)(*(int *)(unaff_EDI + 400) + 0x58);
  *piVar1 = *piVar1 + *(int *)(iVar2 + 0x14) / 8;
  *(undefined4 *)(iVar2 + 0x14) = 0;
  uVar3 = (**(code **)(*(int *)(unaff_EDI + 400) + 8))();
  if ((char)uVar3 != '\0') {
    iVar5 = 0;
    if (0 < *(int *)(unaff_EDI + 0x120)) {
      puVar4 = (undefined4 *)(iVar2 + 0x24);
      do {
        *puVar4 = 0;
        iVar5 = iVar5 + 1;
        puVar4 = puVar4 + 1;
      } while (iVar5 < *(int *)(unaff_EDI + 0x120));
    }
    *(undefined4 *)(iVar2 + 0x20) = 0;
    uVar3 = *(undefined4 *)(unaff_EDI + 0xfc);
    *(undefined4 *)(iVar2 + 0x34) = uVar3;
    *(undefined1 *)(iVar2 + 0x18) = 0;
    uVar3 = CONCAT31((int3)((uint)uVar3 >> 8),1);
  }
  return uVar3;
}

