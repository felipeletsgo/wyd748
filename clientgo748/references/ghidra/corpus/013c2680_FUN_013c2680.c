// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c2680 | Name: FUN_013c2680


void FUN_013c2680(void)

{
  int iVar1;
  int unaff_EBP;
  short *psVar2;
  
  iVar1 = 0x87;
  psVar2 = &DAT_013c2200;
  do {
    if ((ushort)((*(ushort *)(*(int *)(unaff_EBP + -0x368) + 0x7ae) & 0xfff) + 0x1000) == *psVar2) {
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1da) =
           *(undefined2 *)(&DAT_00d44990 + *(short *)(*(int *)(unaff_EBP + -0x368) + 0x7ae) * 0x8c);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1de) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1da);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e2) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1de);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e6) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e2);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1dc) =
           *(undefined2 *)(&DAT_00d44992 + *(short *)(*(int *)(unaff_EBP + -0x368) + 0x7ae) * 0x8c);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e0) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1dc);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e4) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e0);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e8) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x368) + 0x1e4);
      FUN_013c2000();
      return;
    }
    psVar2 = psVar2 + 4;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  FUN_004fb22e();
  return;
}

