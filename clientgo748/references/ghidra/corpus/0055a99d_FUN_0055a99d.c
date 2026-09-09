// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a99d | Name: FUN_0055a99d


int FUN_0055a99d(char *param_1,int param_2)

{
  undefined4 uVar1;
  uint uVar2;
  char local_24;
  undefined4 local_23;
  undefined1 local_1c;
  undefined2 local_1b;
  undefined1 local_19;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  size_t local_8;
  
  local_8 = _strlen(param_1);
  if (local_8 == 0) {
    local_18 = 0;
  }
  else {
    local_18 = 0;
    uVar2 = local_8 & 0x80000003;
    if ((int)uVar2 < 0) {
      uVar2 = (uVar2 - 1 | 0xfffffffc) + 1;
    }
    if (uVar2 == 0) {
      local_10 = param_2;
      local_23 = 0;
      local_1c = DAT_00e38528;
      local_1b = 0;
      local_19 = 0;
      for (local_c = 0; local_c < (int)(local_8 - 2); local_c = local_c + 1) {
        if (param_1[local_c] == '=') {
          return -1;
        }
      }
      for (local_c = 0; uVar1 = local_23, local_c < (int)local_8; local_c = local_c + 4) {
        local_24 = param_1[local_c];
        local_23._3_1_ = SUB41(uVar1,3);
        local_23._0_3_ =
             CONCAT12(param_1[local_c + 3],CONCAT11(param_1[local_c + 2],param_1[local_c + 1]));
        local_14 = FUN_0055a815(&local_24,&local_1c);
        if (local_14 == -1) {
          return -1;
        }
        local_18 = local_18 + local_14;
        FUN_0058f220(local_10,&local_1c,local_14);
        local_10 = local_10 + local_14;
      }
    }
    else {
      local_18 = -1;
    }
  }
  return local_18;
}

