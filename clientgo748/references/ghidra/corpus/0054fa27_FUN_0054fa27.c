// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054fa27 | Name: FUN_0054fa27


undefined4 FUN_0054fa27(void)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  short local_130;
  undefined4 local_12e [63];
  undefined4 *local_30;
  short local_2c;
  int local_28;
  int local_20;
  undefined4 local_1c;
  int local_18;
  undefined *local_14;
  int local_10;
  undefined4 local_c;
  int local_8;
  
  local_10 = 910000;
  local_14 = &DAT_00d44950;
  local_1c = 0;
  local_20 = FUN_0058f716(s___ItemList_bin_005c4150,&DAT_005c414c);
  if (local_20 == 0) {
    MessageBoxA((HWND)0x0,s_Can_t_read_ItemList_bin_005c4168,s_ERROR_005c4160,0);
    uVar1 = 0;
  }
  else {
    FUN_00590d68(&DAT_00d44950,910000,1,local_20);
    FUN_00590d68(&local_1c,4,1,local_20);
    FUN_0058f62c(local_20);
    local_c = FUN_0054f3d9(&DAT_00d44950,local_10);
    for (local_18 = 0; local_18 < local_10; local_18 = local_18 + 1) {
      local_14[local_18] = local_14[local_18] ^ 0x5a;
    }
    local_8 = FUN_0058fb07(s___ExtraItem_bin_005c4180,0x8000,0);
    if (local_8 != -1) {
      while (uVar2 = FUN_0058f8c9(local_8,&local_130,0x8e), 0x8d < uVar2) {
        local_2c = local_130;
        if ((0 < local_130) && (local_130 < 0x1964)) {
          local_30 = local_12e;
          puVar4 = local_30;
          puVar5 = (undefined4 *)(&DAT_00d44950 + local_130 * 0x8c);
          for (iVar3 = 0x23; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar5 = *puVar4;
            puVar4 = puVar4 + 1;
            puVar5 = puVar5 + 1;
          }
        }
      }
      FUN_0058f75f(local_8);
      for (local_28 = 0; local_28 < local_10; local_28 = local_28 + 1) {
        local_14[local_28] = local_14[local_28] ^ 0x5a;
      }
      FUN_0054f3d9(&DAT_00d44950,local_10);
      for (local_28 = 0; local_28 < local_10; local_28 = local_28 + 1) {
        local_14[local_28] = local_14[local_28] ^ 0x5a;
      }
    }
    uVar1 = 1;
  }
  return uVar1;
}

