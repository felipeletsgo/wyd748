// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b8b6 | Name: FUN_0055b8b6


void FUN_0055b8b6(void)

{
  undefined1 local_324 [128];
  CHAR local_2a4 [128];
  CHAR local_224 [256];
  int local_124;
  int local_120;
  undefined1 local_11c [20];
  char local_108 [260];
  
  local_124 = FUN_00592473(s_______005c4be0,local_11c);
  if (local_124 != -1) {
    do {
      while( true ) {
        while (local_108[0] == '.') {
          local_120 = FUN_00592540(local_124,local_11c);
          if (local_120 != 0) goto LAB_0055ba8c;
          local_120 = 0;
        }
        FUN_0058f078(local_2a4,&DAT_005c4be8,local_108);
        local_120 = FUN_0055b530(local_108,local_224);
        if (local_120 != 0) break;
        local_120 = FUN_00592540(local_124,local_11c);
        if (local_120 != 0) goto LAB_0055ba8c;
        local_120 = 0;
      }
      if (local_108[0] == '_') {
        FUN_0058f078(local_324,&PTR_DAT_005c4bf0,local_224);
        FUN_0058ee20(local_224,local_324);
      }
      local_120 = CopyFileA(local_108,local_224,0);
      if (local_120 == 0) {
        local_120 = SetFileAttributesA(local_224,0x80);
        local_120 = CopyFileA(local_108,local_224,0);
        if (local_120 != 0) {
          local_120 = DeleteFileA(local_2a4);
        }
      }
      else {
        local_120 = DeleteFileA(local_2a4);
      }
      local_120 = FUN_00592540(local_124,local_11c);
    } while (local_120 == 0);
LAB_0055ba8c:
    if (local_124 != -1) {
      FUN_00592608(local_124);
    }
  }
  return;
}

