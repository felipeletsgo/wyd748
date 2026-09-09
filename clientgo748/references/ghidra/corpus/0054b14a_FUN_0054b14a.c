// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b14a | Name: FUN_0054b14a


void FUN_0054b14a(void)

{
  int iVar1;
  int local_32c [201];
  int local_8;
  
  _memset(local_32c,0,800);
  local_32c[200] = FUN_0058f716(s_ItemPrice_bin_005c3f58,&DAT_005c3f54);
  if (local_32c[200] != 0) {
    FUN_00590d68(local_32c,800,1,local_32c[200]);
    FUN_0058f62c(local_32c[200]);
  }
  for (local_8 = 0; (local_8 < 100 && (local_32c[local_8 * 2] != 0)); local_8 = local_8 + 1) {
    iVar1 = local_32c[local_8 * 2];
    if ((local_32c[local_8 * 2 + 1] <= *(int *)(&DAT_00d449d0 + iVar1 * 0x8c) * 3) &&
       ((*(int *)(&DAT_00d449d0 + iVar1 * 0x8c) * 0x1e) / 100 <= local_32c[local_8 * 2 + 1])) {
      *(int *)(&DAT_00d449d0 + iVar1 * 0x8c) = local_32c[local_8 * 2 + 1];
    }
  }
  return;
}

