// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00526140 | Name: FUN_00526140


void __thiscall FUN_00526140(int param_1,ushort *param_2)

{
  undefined2 uVar1;
  undefined1 local_10;
  
  if (*(int *)(param_1 + 0x214) != 0) {
    return;
  }
  *(ushort *)(param_1 + 0x250) = *param_2 & 0xfff;
  *(ushort *)(param_1 + 0x252) = param_2[1] & 0xfff;
  *(undefined *)(param_1 + 0x7a8) = (&DAT_00d449da)[(*param_2 & 0xfff) * 0x8c];
  *(undefined2 *)(param_1 + 0x1d2) = *(undefined2 *)(&DAT_00d44990 + (*param_2 & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1d4) = *(undefined2 *)(&DAT_00d44992 + (*param_2 & 0xfff) * 0x8c);
  if (((param_2[1] & 0xfff) < 0xdac) || (0xdae < (param_2[1] & 0xfff))) {
    *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(&DAT_00d44990 + (param_2[1] & 0xfff) * 0x8c);
    *(undefined2 *)(param_1 + 0x1d8) = *(undefined2 *)(&DAT_00d44992 + (param_2[1] & 0xfff) * 0x8c);
  }
  *(undefined2 *)(param_1 + 0x1da) = *(undefined2 *)(&DAT_00d44990 + (param_2[2] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1dc) = *(undefined2 *)(&DAT_00d44992 + (param_2[2] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1de) = *(undefined2 *)(&DAT_00d44990 + (param_2[3] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e0) = *(undefined2 *)(&DAT_00d44992 + (param_2[3] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e2) = *(undefined2 *)(&DAT_00d44990 + (param_2[4] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e4) = *(undefined2 *)(&DAT_00d44992 + (param_2[4] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e6) = *(undefined2 *)(&DAT_00d44990 + (param_2[5] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1e8) = *(undefined2 *)(&DAT_00d44992 + (param_2[5] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1ee) = *(undefined2 *)(&DAT_00d44990 + (param_2[6] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1f0) = *(undefined2 *)(&DAT_00d44992 + (param_2[6] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1ea) = *(undefined2 *)(&DAT_00d44990 + (param_2[7] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x1ec) = *(undefined2 *)(&DAT_00d44992 + (param_2[7] & 0xfff) * 0x8c);
  *(undefined2 *)(param_1 + 0x7aa) = *(undefined2 *)(&DAT_00d44992 + (param_2[0xf] & 0xfff) * 0x8c);
  uVar1 = FUN_00526cf4(*(undefined2 *)(param_1 + 0x7aa));
  *(undefined2 *)(param_1 + 0x7aa) = uVar1;
  if ((param_2[0xd] < 0x1037) || (0x1068 < param_2[0xd])) {
    *(ushort *)(param_1 + 0x7ae) = param_2[0xd] & 0xfff;
  }
  else {
    *(ushort *)(param_1 + 0x7ae) = param_2[0xd];
  }
  *(char *)(param_1 + 0x1f2) = (char)((int)(uint)*param_2 >> 0xc);
  *(char *)(param_1 + 499) = (char)((int)(uint)param_2[1] >> 0xc);
  *(char *)(param_1 + 500) = (char)((int)(uint)param_2[2] >> 0xc);
  *(char *)(param_1 + 0x1f5) = (char)((int)(uint)param_2[3] >> 0xc);
  *(char *)(param_1 + 0x1f6) = (char)((int)(uint)param_2[4] >> 0xc);
  *(char *)(param_1 + 0x1f7) = (char)((int)(uint)param_2[5] >> 0xc);
  *(char *)(param_1 + 0x1f9) = (char)((int)(uint)param_2[6] >> 0xc);
  *(char *)(param_1 + 0x1f8) = (char)((int)(uint)param_2[7] >> 0xc);
  *(ushort *)(param_1 + 0x7e0) = param_2[0xf] & 0xfff;
  if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
    *(undefined1 *)(DAT_013b71e8 + 0x74b) = *(undefined1 *)(param_1 + 0x1f2);
  }
  *(undefined2 *)(param_1 + 0x7b0) = 0;
  if (*(short *)(param_1 + 0x7ae) == 0x301) {
    *(short *)(param_1 + 0x7b0) = (short)((int)(uint)param_2[0xd] >> 0xc) + 1;
  }
  FUN_004ff365();
  if ((param_2[0xf] & 0xfff) == 0) {
    *(undefined1 *)(param_1 + 0x79b) = 0;
  }
  else {
    *(undefined1 *)(param_1 + 0x79b) = 1;
    FUN_00524d01((int)*(short *)(&DAT_00d44992 + (param_2[0xf] & 0xfff) * 0x8c));
  }
  *(undefined1 *)(param_1 + 0x7a9) = 1;
  if ((param_2[0xd] & 0xfff) == 0x312) {
    if (((uint)((int)(uint)param_2[0xd] >> 0xc) < 2) && ((param_2[0xd] & 0xfff) != 0x312)) {
      *(undefined1 *)(param_1 + 0x7a9) = 2;
    }
    else {
      local_10 = (undefined1)((int)(uint)param_2[0xd] >> 0xc);
      *(undefined1 *)(param_1 + 0x7a9) = local_10;
    }
  }
  if ((param_2[0xb] & 0xfff) == 0x312) {
    *(char *)(param_1 + 0x7a9) =
         *(char *)(param_1 + 0x7a9) + (char)(((int)(uint)param_2[0xb] >> 0xc) << 4);
  }
  FUN_013d2200();
  return;
}

