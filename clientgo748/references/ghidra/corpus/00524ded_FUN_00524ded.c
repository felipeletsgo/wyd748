// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00524ded | Name: FUN_00524ded


void __thiscall FUN_00524ded(int param_1,int param_2)

{
  undefined1 uVar1;
  char cVar2;
  undefined2 uVar3;
  short sVar4;
  int iVar5;
  undefined1 local_10;
  
  if (*(int *)(param_1 + 0x214) != 0) {
    return;
  }
  *(undefined2 *)(param_1 + 0x250) = *(undefined2 *)(param_2 + 0x5c);
  *(undefined2 *)(param_1 + 0x252) = *(undefined2 *)(param_2 + 100);
  *(short *)(param_1 + 0x7ba) = (short)*(char *)(param_2 + 99);
  *(undefined *)(param_1 + 0x7a8) = (&DAT_00d449da)[*(short *)(param_2 + 0x5c) * 0x8c];
  *(undefined2 *)(param_1 + 0x1d2) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x5c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1d4) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x5c) * 0x8c);
  if ((*(short *)(param_2 + 100) < 0xdac) || (0xdae < *(short *)(param_2 + 100))) {
    *(undefined2 *)(param_1 + 0x1d6) =
         *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 100) * 0x8c);
    *(undefined2 *)(param_1 + 0x1d8) =
         *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 100) * 0x8c);
  }
  *(undefined2 *)(param_1 + 0x1da) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x6c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1dc) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x6c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1de) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x74) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e0) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x74) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e2) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x7c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e4) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x7c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e6) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x84) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e8) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x84) * 0x8c);
  *(undefined2 *)(param_1 + 0x1ee) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x8c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1f0) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x8c) * 0x8c);
  *(undefined2 *)(param_1 + 0x1ea) =
       *(undefined2 *)(&DAT_00d44990 + *(short *)(param_2 + 0x94) * 0x8c);
  *(undefined2 *)(param_1 + 0x1ec) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0x94) * 0x8c);
  *(undefined2 *)(param_1 + 0x7aa) =
       *(undefined2 *)(&DAT_00d44992 + *(short *)(param_2 + 0xd4) * 0x8c);
  uVar3 = FUN_00526cf4(*(undefined2 *)(param_1 + 0x7aa));
  *(undefined2 *)(param_1 + 0x7aa) = uVar3;
  *(undefined2 *)(param_1 + 0x7ae) = *(undefined2 *)(param_2 + 0xc4);
  uVar1 = FUN_0054e06c(param_2 + 0x5c);
  *(undefined1 *)(param_1 + 0x1f2) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 100);
  *(undefined1 *)(param_1 + 499) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 0x6c);
  *(undefined1 *)(param_1 + 500) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 0x74);
  *(undefined1 *)(param_1 + 0x1f5) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 0x7c);
  *(undefined1 *)(param_1 + 0x1f6) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 0x84);
  *(undefined1 *)(param_1 + 0x1f7) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 0x8c);
  *(undefined1 *)(param_1 + 0x1f9) = uVar1;
  uVar1 = FUN_0054e06c(param_2 + 0x94);
  *(undefined1 *)(param_1 + 0x1f8) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x5c);
  *(undefined1 *)(param_1 + 0x202) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 100);
  *(undefined1 *)(param_1 + 0x203) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x6c);
  *(undefined1 *)(param_1 + 0x204) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x74);
  *(undefined1 *)(param_1 + 0x205) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x7c);
  *(undefined1 *)(param_1 + 0x206) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x84);
  *(undefined1 *)(param_1 + 0x207) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x8c);
  *(undefined1 *)(param_1 + 0x209) = uVar1;
  uVar1 = FUN_0054df49(param_2 + 0x94);
  *(undefined1 *)(param_1 + 0x208) = uVar1;
  *(undefined2 *)(param_1 + 0x7e0) = *(undefined2 *)(param_2 + 0xd4);
  *(undefined2 *)(param_1 + 0x7b0) = 0;
  if (*(short *)(param_1 + 0x7ae) == 0x301) {
    sVar4 = FUN_0054e06c(param_2 + 0xc4);
    *(short *)(param_1 + 0x7b0) = sVar4 + 1;
  }
  FUN_004ff365();
  if (*(short *)(param_2 + 0xd4) < 1) {
    *(undefined1 *)(param_1 + 0x79b) = 0;
  }
  else {
    FUN_00524d01((int)*(short *)(&DAT_00d44992 + *(short *)(param_2 + 0xd4) * 0x8c));
  }
  *(undefined1 *)(param_1 + 0x7a9) = 1;
  if (*(short *)(param_2 + 0xc4) == 0x312) {
    iVar5 = FUN_0054e06c(param_2 + 0xc4);
    if ((iVar5 < 2) && (*(short *)(param_2 + 0xc4) != 0x312)) {
      *(undefined1 *)(param_1 + 0x7a9) = 2;
    }
    else {
      local_10 = (undefined1)iVar5;
      *(undefined1 *)(param_1 + 0x7a9) = local_10;
    }
  }
  if (*(short *)(param_2 + 0xb4) == 0x312) {
    cVar2 = FUN_0054e06c(param_2 + 0xb4);
    *(char *)(param_1 + 0x7a9) = *(char *)(param_1 + 0x7a9) + cVar2 * '\x10';
  }
  FUN_013d2000();
  return;
}

