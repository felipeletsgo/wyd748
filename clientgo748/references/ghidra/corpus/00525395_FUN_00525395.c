// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00525395 | Name: FUN_00525395


void __fastcall FUN_00525395(undefined4 param_1,int param_2)

{
  char cVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  int iVar4;
  int unaff_EBP;
  float10 fVar5;
  undefined4 uVar6;
  
  if ((((param_2 < 0x938) || (0x955 < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))) &&
      ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xf8c ||
       (0xf9e < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))))) &&
     ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xb90 ||
      (2999 < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))))) {
    *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79c) = 0;
    *(int *)(unaff_EBP + -0x58) = DAT_0067cf38;
    if (((DAT_005b892c == 2) && (*(int *)(unaff_EBP + -0x58) != 0)) &&
       (*(int *)(*(int *)(unaff_EBP + -0x58) + 0x26e1c) == 0)) {
      FUN_0058f078(unaff_EBP + -0x78,&DAT_005bb7f4,0);
      (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x58) + 0x27ac4) + 0x80))(unaff_EBP + -0x78,0);
      FUN_0058f078(unaff_EBP + -0x78,&DAT_005bb7f8,0);
      (**(code **)(**(int **)(*(int *)(unaff_EBP + -0x58) + 0x27ac0) + 0x80))(unaff_EBP + -0x78,0);
    }
  }
  else {
    uVar3 = FUN_0054cd07(*(int *)(unaff_EBP + 8) + 0xcc,0x50);
    *(undefined4 *)(unaff_EBP + -0x14) = uVar3;
    if ((*(int *)(unaff_EBP + -0x14) < 1) &&
       ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xf8c ||
        (0xf9e < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))))) {
      *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79c) = 0;
    }
    else {
      *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79d) =
           *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79c);
      *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x79c) = 1;
      *(int *)(unaff_EBP + -0x28) = *(short *)(*(int *)(unaff_EBP + 8) + 0xcc) + -0x7fd;
      if ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0x953) ||
         (0x954 < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))) {
        if ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xf8c) ||
           (0xf8e < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))) {
          if ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xf8f) ||
             (0xf91 < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))) {
            if ((*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0xf92) ||
               (0xf94 < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc))) {
              if (*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) == 0xf95) {
                *(undefined4 *)(unaff_EBP + -0x28) = 0x159;
              }
              else if (*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) == 0xf96) {
                *(undefined4 *)(unaff_EBP + -0x28) = 0x14e;
              }
              else if (*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) == 0xf97) {
                *(undefined4 *)(unaff_EBP + -0x28) = 0x14f;
              }
              else if (*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) == 0xf98) {
                *(undefined4 *)(unaff_EBP + -0x28) = 0x13e;
              }
            }
            else {
              *(int *)(unaff_EBP + -0x28) = *(short *)(*(int *)(unaff_EBP + 8) + 0xcc) + -0xe3c;
            }
          }
          else {
            *(int *)(unaff_EBP + -0x28) = *(short *)(*(int *)(unaff_EBP + 8) + 0xcc) + -0xe39;
          }
        }
        else {
          *(int *)(unaff_EBP + -0x28) = *(short *)(*(int *)(unaff_EBP + 8) + 0xcc) + -0xe36;
        }
      }
      else {
        *(undefined4 *)(unaff_EBP + -0x28) = 0x150;
      }
      _memset((void *)(unaff_EBP + -0x24),0,8);
      *(undefined2 *)(unaff_EBP + -0x24) = *(undefined2 *)(unaff_EBP + -0x28);
      iVar4 = FUN_0054cd07(*(int *)(unaff_EBP + 8) + 0xcc,0x51);
      *(int *)(unaff_EBP + -0x1c) = iVar4 / 10;
      uVar3 = FUN_0054cd07(unaff_EBP + -0x24,0x12);
      *(undefined4 *)(unaff_EBP + -0x2c) = uVar3;
      uVar3 = FUN_004fab52(*(undefined4 *)(unaff_EBP + -0x2c));
      *(undefined4 *)(*(int *)(unaff_EBP + -0x80) + 0x7a0) = uVar3;
      *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2) =
           *(undefined2 *)(&DAT_00d44990 + *(int *)(unaff_EBP + -0x28) * 0x8c);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a6) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a4) =
           *(undefined2 *)(&DAT_00d44992 + *(int *)(unaff_EBP + -0x28) * 0x8c);
      *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a8) =
           *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a4);
      *(short *)(*(int *)(unaff_EBP + -0x80) + 0x1a0) =
           (short)*(undefined4 *)(unaff_EBP + -0x28) + -0x13b;
      if ((*(int *)(unaff_EBP + -0x28) < 0x141) || (0x145 < *(int *)(unaff_EBP + -0x28))) {
        if ((*(int *)(unaff_EBP + -0x28) < 0x146) || (0x14a < *(int *)(unaff_EBP + -0x28))) {
          if ((*(int *)(unaff_EBP + -0x28) == 0x14e) || (*(int *)(unaff_EBP + -0x28) == 0x14f)) {
            *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
                 *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) =
                 *(undefined1 *)(unaff_EBP + -0x1c);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) =
                 *(undefined1 *)(unaff_EBP + -0x1c);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) =
                 *(undefined1 *)(unaff_EBP + -0x1c);
          }
          else if ((*(int *)(unaff_EBP + -0x28) < 0x150) || (0x152 < *(int *)(unaff_EBP + -0x28))) {
            if ((*(int *)(unaff_EBP + -0x28) < 0x153) || (0x155 < *(int *)(unaff_EBP + -0x28))) {
              if ((*(int *)(unaff_EBP + -0x28) < 0x156) || (0x159 < *(int *)(unaff_EBP + -0x28))) {
                *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) = 0;
                *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) = 0;
                *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) =
                     *(undefined1 *)(unaff_EBP + -0x1c);
                *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) =
                     *(undefined1 *)(unaff_EBP + -0x1c);
              }
              else {
                if (*(int *)(unaff_EBP + -0x28) == 0x158) {
                  *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) = 10;
                }
                else if (*(int *)(unaff_EBP + -0x28) == 0x159) {
                  *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) = 0xb;
                }
                else if (*(int *)(unaff_EBP + -0x28) == 0x157) {
                  *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
                       *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2);
                }
                else {
                  *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
                       *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2);
                }
                *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) = 0;
                *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) = 0;
                *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) = 0;
                if (*(int *)(unaff_EBP + -0x28) == 0x159) {
                  *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) = 7;
                }
              }
            }
            else {
              *(short *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
                   (short)*(undefined4 *)(unaff_EBP + -0x28) + -0x153 +
                   *(short *)(*(int *)(unaff_EBP + -0x80) + 0x1a2);
              *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) =
                   *(undefined1 *)(unaff_EBP + -0x1c);
              *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) =
                   *(undefined1 *)(unaff_EBP + -0x1c);
              *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) =
                   *(undefined1 *)(unaff_EBP + -0x1c);
            }
          }
          else {
            *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
                 *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) =
                 *(undefined1 *)(unaff_EBP + -0x1c);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) =
                 *(undefined1 *)(unaff_EBP + -0x1c);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) =
                 *(undefined1 *)(unaff_EBP + -0x1c);
          }
        }
        else {
          *(short *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
               (short)*(undefined4 *)(unaff_EBP + -0x28) + -0x145;
          *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) = *(undefined1 *)(unaff_EBP + -0x1c);
          *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) = 0;
          *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) = 0;
        }
      }
      else {
        *(short *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
             (short)*(undefined4 *)(unaff_EBP + -0x28) + -0x140;
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c4) = *(undefined1 *)(unaff_EBP + -0x1c);
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c2) = 0;
        *(undefined1 *)(*(int *)(unaff_EBP + -0x80) + 0x1c3) = 0;
      }
      if ((0x952 < *(short *)(*(int *)(unaff_EBP + 8) + 0xcc)) &&
         (*(short *)(*(int *)(unaff_EBP + 8) + 0xcc) < 0x955)) {
        *(short *)(*(int *)(unaff_EBP + -0x80) + 0x1aa) =
             *(short *)(*(int *)(unaff_EBP + 8) + 0xcc) + -0x94b;
      }
      fVar5 = (float10)FUN_004faaf4(*(undefined4 *)(*(int *)(unaff_EBP + -0x80) + 0x7a0),
                                    *(undefined2 *)(*(int *)(unaff_EBP + -0x80) + 0x1a2));
      *(float *)(*(int *)(unaff_EBP + -0x80) + 0x7a4) = (float)fVar5;
      if ((*(char *)(*(int *)(unaff_EBP + -0x80) + 0x79d) == '\0') &&
         (*(int *)(*(int *)(unaff_EBP + -0x80) + 0x7a0) == 0x1f)) {
        *(undefined4 *)(unaff_EBP + -0x34) = 0x114;
        if ((DAT_005ccf98 != 0) &&
           (iVar4 = FUN_00429a6d(*(undefined4 *)(unaff_EBP + -0x34)), iVar4 != 0)) {
          FUN_00429a6d(*(undefined4 *)(unaff_EBP + -0x34));
          iVar4 = FUN_0042afd0();
          if (iVar4 == 0) {
            uVar6 = 0;
            uVar3 = 0;
            FUN_00429a6d(*(undefined4 *)(unaff_EBP + -0x34));
            FUN_0042ad2b(uVar3,uVar6);
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
    }
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
    uVar3 = FUN_0054d4b6(*(int *)(unaff_EBP + 8) + 0xcc,0x50);
    *(undefined4 *)(unaff_EBP + -0x7c) = uVar3;
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

