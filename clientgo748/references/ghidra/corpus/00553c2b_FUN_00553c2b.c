// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553c2b | Name: FUN_00553c2b


void FUN_00553c2b(undefined4 param_1)

{
  undefined1 local_420 [128];
  undefined1 local_3a0 [128];
  int local_320;
  size_t local_31c;
  int local_318;
  int local_314;
  int local_310;
  int local_30c;
  undefined1 local_308 [256];
  char *local_208;
  char local_204 [510];
  undefined1 local_6;
  undefined1 local_5;
  
  _memset(&DAT_00a34178,0x1b8,0x80);
  local_30c = FUN_0058f716(param_1,&DAT_005c419c);
  if (local_30c != 0) {
    while( true ) {
      local_310 = -1;
      local_314 = FUN_00590a06(local_204,0x200,local_30c);
      if (local_314 == 0) break;
      FUN_005909d2(local_204,s__d__s_005c41a0,&local_310,local_308);
      local_6 = 0;
      local_5 = 0;
      if ((0 < local_310) && (local_310 < 0x1b9)) {
        local_320 = 0;
        for (local_318 = 0; local_318 < 0x100; local_318 = local_318 + 1) {
          if (local_204[local_318] == '\t') {
            local_320 = local_318;
            break;
          }
        }
        if (local_320 == 0) {
          FUN_0058f078(local_3a0,s_Can_t_parse_String__s_005c41a8,local_204);
          FUN_00423b20(local_3a0,s__system_005c41c0,0);
        }
        else {
          local_208 = local_204 + local_320 + 1;
          for (local_318 = 0; local_318 < 0x80; local_318 = local_318 + 1) {
            if (((local_208[local_318] == '\t') || (local_208[local_318] == '\n')) ||
               (local_208[local_318] == '\0')) {
              local_208[local_318] = '\0';
              break;
            }
          }
          local_31c = _strlen(local_208);
          if (((int)local_31c < 1) || (0x5d < (int)local_31c)) {
            FUN_0058f078(local_420,s_Empty_or_Long_String__s_005c41c8,local_204);
            FUN_00423b20(local_420,s__system_005c41e0,0);
            local_208[0x5f] = '\0';
            local_208[0x5e] = '\0';
          }
          FUN_0058ee20(&DAT_00a34178 + local_310 * 0x80,local_208);
        }
      }
    }
    FUN_0058f62c(local_30c);
  }
  return;
}

