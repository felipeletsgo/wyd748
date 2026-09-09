// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c2100 | Name: FUN_013c2100


void FUN_013c2100(void)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int unaff_EBP;
  
  uVar1 = *(ushort *)(*(int *)(unaff_EBP + -0xd0) + 0x356);
  if (((uVar1 & 0x4000) != 0) && (uVar4 = uVar1 & 0x3fff, uVar4 < 0x12f)) {
    if (*(uint *)(unaff_EBP + -0x3c) < 6) {
      iVar2 = *(int *)(&DAT_013c2800 + *(uint *)(unaff_EBP + -0x3c) * 8 + uVar4 * 0x30);
      if (iVar2 != 0) {
        iVar3 = *(int *)((int)(&DAT_013c2800 + *(uint *)(unaff_EBP + -0x3c) * 8 + uVar4 * 0x30) + 4)
        ;
        if (iVar3 != 0) {
          FUN_0058f078(unaff_EBP + -0x80,iVar2);
          FUN_0058f078(unaff_EBP + -0x30,iVar3);
          FUN_004c0052();
          return;
        }
        if (*(int *)(unaff_EBP + -0x3c) != 0) {
          FUN_004bead1();
          return;
        }
      }
    }
  }
  FUN_004bf8c5();
  return;
}

