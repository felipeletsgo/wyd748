// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00554513 | Name: FUN_00554513


undefined4 FUN_00554513(void)

{
  undefined4 uVar1;
  int local_134;
  undefined1 local_130 [4];
  undefined1 local_12c [4];
  undefined1 local_128 [4];
  undefined1 local_124 [4];
  undefined1 local_120 [4];
  undefined1 local_11c [4];
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  char local_108 [256];
  int local_8;
  
  local_8 = FUN_0058f716(s_S__Settings_Lotto_txt_005c430c,&DAT_005c4308);
  if (local_8 == 0) {
    uVar1 = 0;
  }
  else {
    _memset(&DAT_00d44690,0,700);
    while( true ) {
      local_10c = -1;
      _memset(&local_134,0,0x1c);
      local_118 = 0;
      local_110 = FUN_00590a06(local_108,0x100,local_8);
      if (local_110 == 0) break;
      for (local_114 = 0; local_114 < 0x100; local_114 = local_114 + 1) {
        if (local_108[local_114] == ',') {
          local_108[local_114] = ' ';
        }
        if (local_108[local_114] == '\0') break;
      }
      FUN_005909d2(local_108,s__d__d__d__d__d__d__d__d__d_005c4324,&local_10c,&local_134,local_130,
                   local_12c,local_128,local_124,local_120,local_11c,&local_118);
      local_10c = local_10c + -0x50;
      if ((-1 < local_10c) && (local_10c < 100)) {
        for (local_114 = 0; local_114 < 7; local_114 = local_114 + 1) {
          if ((*(int *)(local_130 + local_114 * 4 + -4) < 1) ||
             (0x2d < *(int *)(local_130 + local_114 * 4 + -4))) {
            *(undefined4 *)(local_130 + local_114 * 4 + -4) = 0;
          }
          (&DAT_00d44690)[local_114 + local_10c * 7] = local_130[local_114 * 4 + -4];
        }
        if ((local_118 < 1) || (989999 < local_118)) {
          local_118 = 100000;
        }
        *(int *)(local_10c * 4 + 0xd44500) = local_118;
      }
    }
    FUN_0058f62c(local_8);
    uVar1 = 1;
  }
  return uVar1;
}

