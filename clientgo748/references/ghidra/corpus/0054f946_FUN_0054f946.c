// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f946 | Name: FUN_0054f946


undefined4 FUN_0054f946(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_18;
  int local_14;
  undefined *local_10;
  int local_c;
  
  local_c = 0x2700;
  local_10 = &DAT_0092fef8;
  local_18 = 0;
  iVar1 = FUN_0058f716(s_SkillData_bin_005c4118,&DAT_005c4114);
  if (iVar1 == 0) {
    MessageBoxA((HWND)0x0,s_Can_t_read_SkillData_bin_005c4130,s_ERROR_005c4128,0);
    uVar2 = 0;
  }
  else {
    FUN_00590d68(&DAT_0092fef8,local_c,1,iVar1);
    FUN_00590d68(&local_18,4,1,iVar1);
    FUN_0058f62c(iVar1);
    FUN_0054f3d9(&DAT_0092fef8,local_c);
    for (local_14 = 0; local_14 < local_c; local_14 = local_14 + 1) {
      local_10[local_14] = local_10[local_14] ^ 0x5a;
    }
    uVar2 = 1;
  }
  return uVar2;
}

