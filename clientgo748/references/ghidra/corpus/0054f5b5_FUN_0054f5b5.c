// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f5b5 | Name: FUN_0054f5b5


undefined4 FUN_0054f5b5(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_18;
  int local_14;
  undefined *local_10;
  int local_c;
  
  _memset(&DAT_00a34178,0x1b8,0x80);
  local_c = 0xdc00;
  local_10 = &DAT_00a34178;
  local_18 = 0;
  iVar1 = FUN_0058f716(s_UI_strdef_bin_005c4020,&DAT_005c401c);
  if (iVar1 != 0) {
    FUN_00590d68(&DAT_00a34178,local_c,1,iVar1);
    FUN_00590d68(&local_18,4,1,iVar1);
    FUN_0058f62c(iVar1);
  }
  iVar1 = FUN_0054f2d9(&DAT_00a34178,local_c);
  if (iVar1 == local_18) {
    for (local_14 = 0; local_14 < local_c; local_14 = local_14 + 1) {
      local_10[local_14] = local_10[local_14] ^ 0x5a;
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

