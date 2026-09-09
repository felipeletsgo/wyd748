// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553eb8 | Name: FUN_00553eb8


void FUN_00553eb8(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined1 local_6ac [128];
  undefined1 local_62c [128];
  undefined1 local_5ac [128];
  int local_52c;
  size_t local_528;
  int local_524;
  int local_520;
  undefined1 local_51c [128];
  undefined1 local_49c [128];
  undefined1 local_41c [128];
  undefined1 local_39c [128];
  int local_31c;
  size_t local_318;
  int local_314;
  int local_310;
  int local_30c;
  char local_308 [256];
  char *local_208;
  char local_204 [510];
  undefined1 local_6;
  undefined1 local_5;
  
  if (param_2 == 0) {
    _memset(&DAT_00e22f2c,0,0x10000);
  }
  local_30c = FUN_0058f716(param_1,&DAT_005c41e8);
  if (local_30c != 0) {
    DAT_00e37d30 = 0;
    if (param_2 == 0) {
      while (local_310 = FUN_00590a06(local_204,0x400,local_30c), local_310 != 0) {
        FUN_005909d2(local_204,&DAT_005c41ec,local_308);
        local_6 = 0;
        local_5 = 0;
        local_31c = 0;
        for (local_314 = 0; local_314 < 0x100; local_314 = local_314 + 1) {
          if (local_204[local_314] == '\t') {
            local_31c = local_314;
            break;
          }
        }
        if (local_31c == 0) {
          FUN_0058f078(local_39c,s_Can_t_parse_String__s_005c41f0,local_204);
          FUN_00423b20(local_39c,s__system_005c4208,0);
        }
        else {
          local_208 = local_204 + local_31c + 1;
          for (local_314 = 0; local_314 < 0x80; local_314 = local_314 + 1) {
            if (((local_208[local_314] == '\t') || (local_208[local_314] == '\n')) ||
               (local_208[local_314] == '\0')) {
              local_208[local_314] = '\0';
              break;
            }
          }
          local_318 = 0;
          local_318 = _strlen(local_308);
          if (((int)local_318 < 1) || (0xf < (int)local_318)) {
            FUN_0058f078(local_41c,s_Empty_or_Long_String_1st__s_005c4210,local_204);
            FUN_00423b20(local_41c,s__system_005c422c,0);
          }
          else {
            local_318 = _strlen(local_208);
            if (((int)local_318 < 1) || (0xf < (int)local_318)) {
              FUN_0058f078(local_49c,s_EEmpty_or_Long_String_2nd__s__d__005c4234,local_204,local_318
                          );
              FUN_00423b20(local_49c,s__system_005c4258,0);
            }
            else {
              if ((DAT_00e37d30 < 0) || (0x3ff < DAT_00e37d30)) {
                FUN_0058f078(local_51c,s_Too_much_items_in_mobname_txt__s_005c4260,local_204);
                FUN_00423b20(local_51c,s__system_005c4284,0);
                break;
              }
              FUN_0058ee20(&DAT_00e22f2c + DAT_00e37d30 * 0x40,local_308);
              FUN_0058ee20(&DAT_00e22f3c + DAT_00e37d30 * 0x40,local_208);
              FUN_0058ee20(&DAT_00e22f4c + DAT_00e37d30 * 0x40,local_308);
              DAT_00e37d30 = DAT_00e37d30 + 1;
            }
          }
        }
      }
    }
    else {
      while (local_520 = FUN_00590a06(local_204,0x400,local_30c), local_520 != 0) {
        FUN_005909d2(local_204,&DAT_005c428c,local_308);
        local_6 = 0;
        local_5 = 0;
        local_52c = 0;
        for (local_524 = 0; local_524 < 0x100; local_524 = local_524 + 1) {
          if (local_204[local_524] == '\t') {
            local_52c = local_524;
            break;
          }
        }
        if (local_52c != 0) {
          local_208 = local_204 + local_52c + 1;
          for (local_524 = 0; local_524 < 0x80; local_524 = local_524 + 1) {
            if (((local_208[local_524] == '\t') || (local_208[local_524] == '\n')) ||
               (local_208[local_524] == '\0')) {
              local_208[local_524] = '\0';
              break;
            }
          }
          local_528 = 0;
          local_528 = _strlen(local_308);
          if (((int)local_528 < 1) || (0xf < (int)local_528)) {
            FUN_0058f078(local_5ac,s_EEmpty_or_Long_String_1st__s_005c4290,local_204);
            FUN_00423b20(local_5ac,s__system_005c42b0,0);
          }
          else {
            local_528 = _strlen(local_208);
            if (((int)local_528 < 1) || (0xf < (int)local_528)) {
              FUN_0058f078(local_62c,s_EEmpty_or_Long_String_2nd__s__d__005c42b8,local_204,local_528
                          );
              FUN_00423b20(local_62c,s__system_005c42dc,0);
            }
            else {
              DAT_00e37d30 = 0;
              while (((DAT_00e37d30 < 0x400 &&
                      (iVar1 = _strcmp(local_308,&DAT_00e22f2c + DAT_00e37d30 * 0x40), iVar1 != 0))
                     && (iVar1 = _strcmp(local_308,&DAT_00e22f3c + DAT_00e37d30 * 0x40), iVar1 != 0)
                     )) {
                DAT_00e37d30 = DAT_00e37d30 + 1;
              }
              if (DAT_00e37d30 == 0x400) {
                FUN_0058f078(local_6ac,s_Can_t_Find_match_mobname__s_005c42e4,local_204);
                FUN_00423b20(local_6ac,s__system_005c4300,0);
              }
              else {
                FUN_0058ee20(&DAT_00e22f4c + DAT_00e37d30 * 0x40,local_208);
              }
            }
          }
        }
      }
      local_520 = 0;
    }
    FUN_0058f62c(local_30c);
  }
  return;
}

