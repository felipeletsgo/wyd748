// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d2200 | Name: FUN_013d2200


void FUN_013d2200(void)

{
  int iVar1;
  int unaff_EBP;
  short *psVar2;
  
  iVar1 = 0x3b;
  psVar2 = &DAT_013d2400;
  do {
    if ((ushort)((*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1c) & 0xfff) + 0x1000) == *psVar2) {
      iVar1 = *(int *)(unaff_EBP + -0x28);
      *(undefined1 *)(iVar1 + 0x79d) = *(undefined1 *)(iVar1 + 0x79c);
      *(undefined1 *)(iVar1 + 0x79c) = 1;
      *(undefined2 *)(iVar1 + 0x1a0) = 0x1b;
      *(uint *)(iVar1 + 0x7a0) = (uint)(ushort)psVar2[1];
      *(undefined4 *)(iVar1 + 0x7a4) = *(undefined4 *)(psVar2 + 2);
      *(undefined4 *)(iVar1 + 0x1a2) = *(undefined4 *)(psVar2 + 4);
      *(undefined4 *)(iVar1 + 0x1a6) = *(undefined4 *)(psVar2 + 6);
      *(undefined4 *)(iVar1 + 0x1aa) = *(undefined4 *)(psVar2 + 8);
      *(char *)(iVar1 + 0x1c2) = (char)psVar2[10];
      *(undefined1 *)(iVar1 + 0x1c3) = *(undefined1 *)((int)psVar2 + 0x15);
      *(char *)(iVar1 + 0x1c4) = (char)psVar2[0xb];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x1fa) =
           (&DAT_00d449da)[(**(ushort **)(unaff_EBP + 8) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x1fb) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 2) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x1fc) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 4) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x1fd) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 6) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x1fe) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 8) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x1ff) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 10) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x201) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 0xc) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x200) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 0xe) & 0xfff) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x28) + 0x7ad) =
           (&DAT_00d449da)[(*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1e) & 0xfff) * 0x8c];
      return;
    }
    psVar2 = psVar2 + 0xc;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  FUN_005266e0();
  return;
}

