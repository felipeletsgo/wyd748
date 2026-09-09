// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553464 | Name: FUN_00553464


undefined4 FUN_00553464(undefined4 param_1,int param_2,int param_3,int param_4,int *param_5)

{
  int iVar1;
  int local_74;
  int local_70;
  int local_68;
  char local_64 [8];
  int local_5c;
  char local_58 [8];
  char local_50 [64];
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = FUN_0054cd07(param_1,0x21);
  FUN_0058f220(local_58,&DAT_005bec4c + local_10 * 8,8);
  _memset(local_50,0,0x3f);
  for (local_c = 0; local_c < 0x40; local_c = local_c + 1) {
    if (*(short *)(param_2 + local_c * 8) != 0) {
      local_10 = FUN_0054cd07(param_2 + local_c * 8,0x21);
      FUN_0058f220(local_64,&DAT_005bec4c + local_10 * 8,8);
      iVar1 = local_c % 9;
      local_5c = local_c / 9;
      for (local_68 = 0; local_68 < 4; local_68 = local_68 + 1) {
        for (local_70 = 0; local_70 < 2; local_70 = local_70 + 1) {
          if ((((local_64[local_70 + local_68 * 2] != '\0') && (-1 < local_68 + local_5c)) &&
              (-1 < local_70 + iVar1)) && ((local_68 + local_5c < 7 && (local_70 + iVar1 < 9)))) {
            local_50[local_70 + iVar1 + (local_68 + local_5c) * 9] = (char)local_c + '\x01';
          }
        }
      }
    }
  }
  local_8 = 0;
  do {
    if (3 < local_8) {
      return 1;
    }
    for (local_74 = 0; local_74 < 2; local_74 = local_74 + 1) {
      if (local_58[local_74 + local_8 * 2] != '\0') {
        if (((local_8 + param_4 < 0) || (local_74 + param_3 < 0)) ||
           ((6 < local_8 + param_4 || (8 < local_74 + param_3)))) {
          *param_5 = -1;
          return 0;
        }
        if (local_50[local_74 + param_3 + (local_8 + param_4) * 9] != '\0') {
          *param_5 = (int)local_50[local_74 + param_3 + (local_8 + param_4) * 9];
          return 0;
        }
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}

