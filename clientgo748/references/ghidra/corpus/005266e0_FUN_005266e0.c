// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005266e0 | Name: FUN_005266e0


void __fastcall FUN_005266e0(int param_1)

{
  undefined4 uVar1;
  int unaff_EBP;
  float10 fVar2;
  
  *(uint *)(unaff_EBP + -4) = *(ushort *)(param_1 + 0x1c) & 0xfff;
  if ((((*(int *)(unaff_EBP + -4) < 0x938) || (0x955 < *(int *)(unaff_EBP + -4))) &&
      ((*(int *)(unaff_EBP + -4) < 0xf8c || (0xf9e < *(int *)(unaff_EBP + -4))))) &&
     ((*(int *)(unaff_EBP + -4) < 0xb90 || (2999 < *(int *)(unaff_EBP + -4))))) {
    *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x79c) = 0;
  }
  else {
    *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x79c) = 1;
    *(int *)(unaff_EBP + -0x20) = *(int *)(unaff_EBP + -4) + -0x7fd;
    if ((*(int *)(unaff_EBP + -4) < 0x953) || (0x954 < *(int *)(unaff_EBP + -4))) {
      if (*(int *)(unaff_EBP + -4) == 0xf95) {
        *(undefined4 *)(unaff_EBP + -0x20) = 0x159;
      }
      else if ((*(int *)(unaff_EBP + -4) < 0xf8c) || (0xf8e < *(int *)(unaff_EBP + -4))) {
        if ((*(int *)(unaff_EBP + -4) < 0xf8f) || (0xf91 < *(int *)(unaff_EBP + -4))) {
          if ((*(int *)(unaff_EBP + -4) < 0xf92) || (0xf94 < *(int *)(unaff_EBP + -4))) {
            if (*(int *)(unaff_EBP + -4) == 0xf96) {
              *(undefined4 *)(unaff_EBP + -0x20) = 0x14e;
            }
            else if (*(int *)(unaff_EBP + -4) == 0xf97) {
              *(undefined4 *)(unaff_EBP + -0x20) = 0x14f;
            }
            else if (*(int *)(unaff_EBP + -4) == 0xf98) {
              *(undefined4 *)(unaff_EBP + -0x20) = 0x13e;
            }
          }
          else {
            *(int *)(unaff_EBP + -0x20) = *(int *)(unaff_EBP + -4) + -0xe3c;
          }
        }
        else {
          *(int *)(unaff_EBP + -0x20) = *(int *)(unaff_EBP + -4) + -0xe39;
        }
      }
      else {
        *(uint *)(unaff_EBP + -0x20) = (*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1c) & 0xfff) - 0xe36
        ;
      }
    }
    else {
      *(undefined4 *)(unaff_EBP + -0x20) = 0x150;
    }
    _memset((void *)(unaff_EBP + -0x1c),0,8);
    *(undefined2 *)(unaff_EBP + -0x1c) = *(undefined2 *)(unaff_EBP + -0x20);
    *(short *)(*(int *)(unaff_EBP + -0x28) + 0x1a0) =
         (short)*(undefined4 *)(unaff_EBP + -0x20) + -0x13b;
    *(int *)(unaff_EBP + -0x14) = (int)(uint)*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1c) >> 0xc;
    uVar1 = FUN_0054cd07(unaff_EBP + -0x1c,0x12);
    *(undefined4 *)(unaff_EBP + -0x24) = uVar1;
    uVar1 = FUN_004fab52(*(undefined4 *)(unaff_EBP + -0x24));
    *(undefined4 *)(*(int *)(unaff_EBP + -0x28) + 0x7a0) = uVar1;
    *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2) =
         *(undefined2 *)(&DAT_00d44990 + *(int *)(unaff_EBP + -0x20) * 0x8c);
    *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a6) =
         *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2);
    *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a4) =
         *(undefined2 *)(&DAT_00d44992 + *(int *)(unaff_EBP + -0x20) * 0x8c);
    *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a8) =
         *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a4);
    if ((*(int *)(unaff_EBP + -0x20) < 0x141) || (0x145 < *(int *)(unaff_EBP + -0x20))) {
      if ((*(int *)(unaff_EBP + -0x20) < 0x146) || (0x14a < *(int *)(unaff_EBP + -0x20))) {
        if ((*(int *)(unaff_EBP + -0x20) == 0x14e) || (*(int *)(unaff_EBP + -0x20) == 0x14f)) {
          *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
               *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2);
          *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = 0;
          *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) = 0;
          *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) = 0;
        }
        else if ((*(int *)(unaff_EBP + -0x20) < 0x150) || (0x152 < *(int *)(unaff_EBP + -0x20))) {
          if ((*(int *)(unaff_EBP + -0x20) < 0x153) || (0x155 < *(int *)(unaff_EBP + -0x20))) {
            if ((*(int *)(unaff_EBP + -0x20) < 0x156) || (0x159 < *(int *)(unaff_EBP + -0x20))) {
              *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) = 0;
              *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = 0;
              *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) =
                   *(undefined1 *)(unaff_EBP + -0x14);
              *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) =
                   *(undefined1 *)(unaff_EBP + -0x14);
            }
            else {
              if (*(int *)(unaff_EBP + -0x20) == 0x158) {
                *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) = 10;
              }
              else if (*(int *)(unaff_EBP + -0x20) == 0x159) {
                *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) = 0xb;
              }
              else if (*(int *)(unaff_EBP + -0x20) == 0x157) {
                *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
                     *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2);
              }
              else {
                *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
                     *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2);
              }
              *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) = 0;
              *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) = 0;
              *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = 0;
              if (*(int *)(unaff_EBP + -0x20) == 0x159) {
                *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = 7;
              }
            }
          }
          else {
            *(short *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
                 (short)*(undefined4 *)(unaff_EBP + -0x20) + -0x153 +
                 *(short *)(*(int *)(unaff_EBP + -0x28) + 0x1a2);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) =
                 *(undefined1 *)(unaff_EBP + -0x14);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) =
                 *(undefined1 *)(unaff_EBP + -0x14);
            *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) =
                 *(undefined1 *)(unaff_EBP + -0x14);
          }
        }
        else {
          *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
               *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2);
          *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = *(undefined1 *)(unaff_EBP + -0x14);
          *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) = *(undefined1 *)(unaff_EBP + -0x14);
          *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) = *(undefined1 *)(unaff_EBP + -0x14);
        }
      }
      else {
        *(short *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
             (short)*(undefined4 *)(unaff_EBP + -0x20) + -0x145;
        *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) = 0;
        *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) = 0;
        *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = *(undefined1 *)(unaff_EBP + -0x14);
      }
    }
    else {
      *(short *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
           (short)*(undefined4 *)(unaff_EBP + -0x20) + -0x140;
      *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c2) = 0;
      *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c3) = 0;
      *(undefined1 *)(*(int *)(unaff_EBP + -0x28) + 0x1c4) = *(undefined1 *)(unaff_EBP + -0x14);
    }
    if ((0x952 < (*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1c) & 0xfff)) &&
       ((*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1c) & 0xfff) < 0x955)) {
      *(ushort *)(*(int *)(unaff_EBP + -0x28) + 0x1aa) =
           (*(ushort *)(*(int *)(unaff_EBP + 8) + 0x1c) & 0xfff) - 0x94b;
    }
    fVar2 = (float10)FUN_004faaf4(*(undefined4 *)(*(int *)(unaff_EBP + -0x28) + 0x7a0),
                                  *(undefined2 *)(*(int *)(unaff_EBP + -0x28) + 0x1a2));
    *(float *)(*(int *)(unaff_EBP + -0x28) + 0x7a4) = (float)fVar2;
  }
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

