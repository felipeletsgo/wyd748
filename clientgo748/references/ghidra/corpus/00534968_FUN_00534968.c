// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00534968 | Name: FUN_00534968


undefined4 __fastcall FUN_00534968(int param_1)

{
  int local_30;
  int local_2c;
  undefined1 local_28 [32];
  int local_8;
  
  for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
    for (local_2c = 0; local_2c < 3; local_2c = local_2c + 1) {
      _memset(local_28,0,0x20);
      FUN_0058f078(local_28,s_UI_m_02d_02d_wyt_005bded8,*(int *)(param_1 + 0x20a20) + -1 + local_2c,
                   *(int *)(param_1 + 0x20a24) + -1 + local_8);
      local_30 = FUN_004b8844(local_28);
      if (local_30 < 0) {
        if (*(int *)(param_1 + 0x20a24) + -1 + local_8 < 0x1a) {
          local_30 = 3;
        }
        else {
          local_30 = 0xd;
        }
      }
      FUN_004ba3ca(4,local_30,local_2c << 7,(2 - local_8) * 0x80,0,0,0x80,0x80);
    }
  }
  return 1;
}

