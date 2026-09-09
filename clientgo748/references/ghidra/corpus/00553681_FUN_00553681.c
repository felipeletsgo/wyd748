// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553681 | Name: FUN_00553681


undefined4 FUN_00553681(undefined4 param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  int local_b4;
  int local_b0;
  int local_a8;
  char local_a4 [8];
  int local_9c;
  char local_98 [8];
  int local_90;
  int local_8c;
  int local_88;
  char local_84 [128];
  
  local_90 = FUN_0054cd07(param_1,0x21);
  FUN_0058f220(local_98,&DAT_005bec4c + local_90 * 8,8);
  _memset(local_84,0,0x7e);
  for (local_8c = 0; local_8c < 0x80; local_8c = local_8c + 1) {
    if (*(short *)(param_2 + local_8c * 8) != 0) {
      local_90 = FUN_0054cd07(param_2 + local_8c * 8,0x21);
      FUN_0058f220(local_a4,&DAT_005bec4c + local_90 * 8,8);
      iVar1 = local_8c % 9;
      local_9c = local_8c / 9;
      for (local_a8 = 0; local_a8 < 4; local_a8 = local_a8 + 1) {
        for (local_b0 = 0; local_b0 < 2; local_b0 = local_b0 + 1) {
          if ((((local_a4[local_b0 + local_a8 * 2] == '\x01') && (-1 < local_a8 + local_9c)) &&
              (-1 < local_b0 + iVar1)) && ((local_a8 + local_9c < 0xe && (local_b0 + iVar1 < 9)))) {
            local_84[local_b0 + iVar1 + (local_a8 + local_9c) * 9] = '\x01';
          }
        }
      }
    }
  }
  local_88 = 0;
  do {
    if (3 < local_88) {
      return 1;
    }
    for (local_b4 = 0; local_b4 < 2; local_b4 = local_b4 + 1) {
      if (local_98[local_b4 + local_88 * 2] == '\x01') {
        if (((local_88 + param_4 < 0) || (local_b4 + param_3 < 0)) ||
           ((0xd < local_88 + param_4 || (8 < local_b4 + param_3)))) {
          return 0;
        }
        if (local_84[local_b4 + param_3 + (local_88 + param_4) * 9] == '\x01') {
          return 0;
        }
      }
    }
    local_88 = local_88 + 1;
  } while( true );
}

