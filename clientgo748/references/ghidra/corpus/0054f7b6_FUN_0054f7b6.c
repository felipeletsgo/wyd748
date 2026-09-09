// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f7b6 | Name: FUN_0054f7b6


void FUN_0054f7b6(int param_1)

{
  int iVar1;
  int local_1c;
  int local_18;
  int local_14;
  undefined *local_10;
  int local_c;
  undefined4 local_8;
  
  local_c = 910000;
  local_10 = &DAT_00d44950;
  for (local_14 = 0; local_14 < 910000; local_14 = local_14 + 1) {
    (&DAT_00d44950)[local_14] = (&DAT_00d44950)[local_14] ^ 0x5a;
  }
  local_1c = 0;
  if (param_1 == 0) {
    local_1c = FUN_0058f716(s___ItemList_bin_005c4078,&DAT_005c4074);
    if (local_1c == 0) {
      MessageBoxA((HWND)0x0,s_Can_t_generate_ItemList_bin_file_005c4090,s_ERROR_005c4088,0);
      return;
    }
    FUN_005904b6(&DAT_00d44950,910000,1,local_1c);
  }
  local_8 = FUN_0054f3d9(&DAT_00d44950,local_c);
  local_18 = _rand();
  if (param_1 == 0) {
    FUN_005904b6(&local_18,4,1,local_1c);
    FUN_0058f62c(local_1c);
    iVar1 = FUN_0058f716(s____ItemCSum_h_005c40b8,&DAT_005c40b4,0);
  }
  else {
    iVar1 = FUN_0058f716(s____ItemCSum_h_005c40cc,&DAT_005c40c8,0);
  }
  if (iVar1 != 0) {
    if (param_1 == 0) {
      FUN_0058f6a9(iVar1,s__define_ITEM_CHECKSUM__d_005c40dc,local_8);
    }
    else {
      FUN_0058f6a9(iVar1,s__define_ITEM_CHECKSUM2__d_005c40f8,local_8);
    }
    FUN_0058f62c(iVar1);
  }
  for (local_14 = 0; local_14 < local_c; local_14 = local_14 + 1) {
    local_10[local_14] = local_10[local_14] ^ 0x5a;
  }
  return;
}

