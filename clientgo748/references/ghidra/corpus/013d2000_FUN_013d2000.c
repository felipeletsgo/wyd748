// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d2000 | Name: FUN_013d2000


void FUN_013d2000(void)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  int unaff_EBP;
  short *psVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  
  iVar3 = 0x3b;
  psVar4 = &DAT_013d2400;
  do {
    if ((ushort)((*(ushort *)(*(int *)(unaff_EBP + 8) + 0xcc) & 0xfff) + 0x1000) == *psVar4) {
      iVar3 = *(int *)(unaff_EBP + -0x80);
      *(undefined1 *)(iVar3 + 0x79d) = *(undefined1 *)(iVar3 + 0x79c);
      *(undefined1 *)(iVar3 + 0x79c) = 1;
      *(undefined4 *)(unaff_EBP + -0x14) = 1;
      *(undefined4 *)(unaff_EBP + -0x28) = 0x156;
      *(undefined4 *)(unaff_EBP + -0x1c) = 0;
      *(undefined2 *)(iVar3 + 0x1a0) = 0x1b;
      *(uint *)(iVar3 + 0x7a0) = (uint)(ushort)psVar4[1];
      *(undefined4 *)(iVar3 + 0x7a4) = *(undefined4 *)(psVar4 + 2);
      *(undefined4 *)(iVar3 + 0x1a2) = *(undefined4 *)(psVar4 + 4);
      *(undefined4 *)(iVar3 + 0x1a6) = *(undefined4 *)(psVar4 + 6);
      *(undefined4 *)(iVar3 + 0x1aa) = *(undefined4 *)(psVar4 + 8);
      *(char *)(iVar3 + 0x1c2) = (char)psVar4[10];
      *(undefined1 *)(iVar3 + 0x1c3) = *(undefined1 *)((int)psVar4 + 0x15);
      *(char *)(iVar3 + 0x1c4) = (char)psVar4[0xb];
      if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x79d) == '\0') &&
         (*(int *)(*(int *)(unaff_EBP + -0x80) + 0x7a0) == 0x1f)) {
        *(undefined4 *)(unaff_EBP + -0x34) = 0x114;
        if (DAT_005ccf98 != 0) {
          iVar3 = FUN_00429a6d(*(undefined4 *)(unaff_EBP + -0x34));
          if (iVar3 != 0) {
            FUN_00429a6d(*(undefined4 *)(unaff_EBP + -0x34));
            iVar3 = FUN_0042afd0();
            if (iVar3 == 0) {
              uVar6 = 0;
              uVar5 = 0;
              FUN_00429a6d(*(undefined4 *)(unaff_EBP + -0x34));
              FUN_0042ad2b(uVar5,uVar6);
            }
          }
        }
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79d) =
             *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79c);
      }
      *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x28) + -0x13b;
      if (*(int *)(unaff_EBP + -0x18) < 0) {
        *(undefined4 *)(unaff_EBP + -0x18) = 0;
      }
      else if (0x11 < *(int *)(unaff_EBP + -0x18)) {
        *(undefined4 *)(unaff_EBP + -0x18) = 0x11;
      }
      (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x80) + 0x438) + 0x8c))
                (*(undefined4 *)(&DAT_005bb41c + *(int *)(unaff_EBP + -0x18) * 4));
      (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x80) + 0x438) + 0x88))
                (*(undefined4 *)(unaff_EBP + -0x14));
      *(int *)(unaff_EBP + -0x30) = DAT_0067cf38;
      if ((DAT_005b892c == 2) && (*(int *)(unaff_EBP + -0x30) != 0)) {
        (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x30) + 0x27ad4) + 0x8c))
                  (*(undefined4 *)(&DAT_005bb41c + *(int *)(unaff_EBP + -0x18) * 4));
        (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x30) + 0x27ad4) + 0x88))
                  (*(undefined4 *)(unaff_EBP + -0x14));
        FUN_0058f078(unaff_EBP + -0x54,&DAT_005bb7ec,
                     *(undefined4 *)(&DAT_005bb41c + *(int *)(unaff_EBP + -0x18) * 4));
        (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x30) + 0x27ac4) + 0x80))(unaff_EBP + -0x54,0)
        ;
        FUN_0058f078(unaff_EBP + -0x54,&DAT_005bb7f0,*(undefined4 *)(unaff_EBP + -0x14));
        (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x30) + 0x27ac0) + 0x80))(unaff_EBP + -0x54,0)
        ;
      }
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x1fa) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x5c) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x1fb) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 100) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x1fc) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x6c) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x1fd) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x74) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x1fe) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x7c) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x1ff) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x84) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x201) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x8c) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x200) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0x94) * 0x8c];
      *(undefined *)(*(int *)(unaff_EBP + -0x80) + 0x7ad) =
           (&DAT_00d449da)[*(short *)(*(int *)(unaff_EBP + 8) + 0xd4) * 0x8c];
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1fa) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f2))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x5c);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fa) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fa) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f2))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x5c);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1fa) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1fb) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 499))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 100);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fb) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fb) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 499))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 100);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1fb) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1fc) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 500))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x6c);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fc) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fc) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 500))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x6c);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1fc) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1fd) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f5))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x74);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fd) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fd) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f5))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x74);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1fd) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1fe) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f6))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x7c);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fe) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x1fe) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f6))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x7c);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1fe) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1ff) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f7))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x84);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x1ff) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x1ff) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f7))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x84);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1ff) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x201) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f9))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x8c);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x201) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x201) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f9))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x8c);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x201) = uVar2;
      }
      if ((*(byte *)(*(int *)(unaff_EBP + -0x80) + 0x200) < 5) &&
         (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f8))) {
        cVar1 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x94);
        *(char *)(*(int *)(unaff_EBP + -0x80) + 0x200) = cVar1 + '\x04';
      }
      else if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x200) == '\x04') &&
              (9 < *(byte *)(*(int *)(unaff_EBP + -0x80) + 0x1f8))) {
        uVar2 = FUN_004173cf(*(int *)(unaff_EBP + 8) + 0x94);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x200) = uVar2;
      }
      *(undefined4 *)(unaff_EBP + -4) = 0;
      if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
        *(int *)(unaff_EBP + -4) = DAT_0067cf38;
      }
      if ((*(int *)(unaff_EBP + -0x80) == *(int *)(DAT_0067cf38 + 0x4c)) &&
         (*(int *)(unaff_EBP + -4) != 0)) {
        uVar5 = FUN_0054d4b6(*(int *)(unaff_EBP + 8) + 0xcc,0x50);
        *(undefined4 *)(unaff_EBP + -0x7c) = uVar5;
        if ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xf8c) ||
           (0xf9e < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))) {
          if (*(int *)(unaff_EBP + -0x7c) < 1) {
            *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x26e7c) = 1;
          }
          else {
            *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x26e7c) = 0;
          }
        }
        else {
          *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x26e7c) = 0;
        }
      }
      return;
    }
    psVar4 = psVar4 + 0xc;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  FUN_00525395();
  return;
}

