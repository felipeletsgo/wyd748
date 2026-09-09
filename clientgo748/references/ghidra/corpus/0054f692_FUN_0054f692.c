// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f692 | Name: FUN_0054f692


void FUN_0054f692(void)

{
  int iVar1;
  int local_1c;
  undefined4 local_18;
  int local_14;
  int local_10;
  undefined *local_c;
  int local_8;
  
  local_8 = 0x2700;
  local_c = &DAT_0092fef8;
  for (local_10 = 0; local_10 < 0x2700; local_10 = local_10 + 1) {
    (&DAT_0092fef8)[local_10] = (&DAT_0092fef8)[local_10] ^ 0x5a;
  }
  local_14 = FUN_0058f716(s_SkillData_bin_005c4034,&DAT_005c4030);
  if (local_14 != 0) {
    FUN_005904b6(&DAT_0092fef8,0x60,0x68,local_14);
    local_18 = FUN_0054f3d9(&DAT_0092fef8,local_8);
    local_1c = _rand();
    FUN_005904b6(&local_1c,4,1,local_14);
    FUN_0058f62c(local_14);
    iVar1 = FUN_0058f716(s____SkillCSum_h_005c4048,&DAT_005c4044);
    if (iVar1 != 0) {
      FUN_0058f6a9(iVar1,s__define_SKILL_CHECKSUM__d_005c4058,local_18);
      FUN_0058f62c(local_14);
    }
  }
  for (local_10 = 0; local_10 < local_8; local_10 = local_10 + 1) {
    local_c[local_10] = local_c[local_10] ^ 0x5a;
  }
  return;
}

