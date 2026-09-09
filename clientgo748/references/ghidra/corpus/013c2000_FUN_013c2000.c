// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c2000 | Name: FUN_013c2000


void FUN_013c2000(void)

{
  int iVar1;
  int unaff_EBP;
  short *psVar2;
  ushort uVar3;
  
  *(undefined2 *)(unaff_EBP + -0x14) = 1;
  iVar1 = 0x87;
  psVar2 = &DAT_013c2200;
  do {
    if ((ushort)((*(ushort *)(*(int *)(unaff_EBP + -0x368) + 0x7ae) & 0xfff) + 0x1000) == *psVar2) {
      uVar3 = psVar2[1];
      if (((char)psVar2[3] == -1) && ((*(uint *)(*(int *)(unaff_EBP + -0x368) + 0x5c) & 1) != 0)) {
        uVar3 = psVar2[2];
      }
      *(ushort *)(unaff_EBP + -0x14) = uVar3 | 0x4000;
      break;
    }
    psVar2 = psVar2 + 4;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  FUN_004fb34a();
  return;
}

