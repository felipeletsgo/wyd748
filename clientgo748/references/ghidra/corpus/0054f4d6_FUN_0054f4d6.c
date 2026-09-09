// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f4d6 | Name: FUN_0054f4d6


void FUN_0054f4d6(void)

{
  undefined4 local_18;
  int local_14;
  int local_10;
  undefined *local_c;
  int local_8;
  
  local_8 = 0xdc00;
  local_c = &DAT_00a34178;
  for (local_10 = 0; local_10 < 0xdc00; local_10 = local_10 + 1) {
    (&DAT_00a34178)[local_10] = (&DAT_00a34178)[local_10] ^ 0x5a;
  }
  local_14 = FUN_0058f716(s_UI_strdef_bin_005c400c,&DAT_005c4008);
  if (local_14 != 0) {
    FUN_005904b6(&DAT_00a34178,1,0xdc00,local_14);
    local_18 = FUN_0054f2d9(&DAT_00a34178,local_8);
    FUN_005904b6(&local_18,4,1,local_14);
    FUN_0058f62c(local_14);
  }
  for (local_10 = 0; local_10 < local_8; local_10 = local_10 + 1) {
    local_c[local_10] = local_c[local_10] ^ 0x5a;
  }
  return;
}

