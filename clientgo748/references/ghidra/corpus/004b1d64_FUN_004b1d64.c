// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1d64 | Name: FUN_004b1d64


void FUN_004b1d64(void)

{
  int iVar1;
  char local_118 [62];
  undefined1 local_da;
  undefined1 local_d9;
  size_t local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = FUN_0058f716(s_Itemname_bin_005b9a5c,&DAT_005b9a58);
  if (local_c != 0) {
    for (local_8 = 0; local_8 < 0x1964; local_8 = local_8 + 1) {
      local_10 = -1;
      _memset(local_118,0,0x100);
      iVar1 = FUN_00590d68(&local_10,4,1,local_c);
      if ((iVar1 == 0) || (iVar1 = FUN_00590d68(local_118,0x40,1,local_c), iVar1 == 0)) break;
      for (local_14 = 0; local_14 < 0x3e; local_14 = local_14 + 1) {
        local_118[local_14] = local_118[local_14] - (char)local_14;
      }
      if ((local_10 != -1) && (local_10 < 0x1964)) {
        local_18 = _strlen(local_118);
        if (0x3e < (int)local_18) {
          local_d9 = 0;
          local_da = 0;
        }
        FUN_0058ee20(&DAT_00d44950 + local_10 * 0x8c,local_118);
      }
    }
    FUN_0058f62c(local_c);
  }
  return;
}

