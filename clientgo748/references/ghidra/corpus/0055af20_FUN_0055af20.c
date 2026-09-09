// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055af20 | Name: FUN_0055af20


bool FUN_0055af20(char *param_1)

{
  size_t sVar1;
  int iVar2;
  undefined4 *puVar3;
  char local_22c;
  undefined4 local_22b;
  char *local_2c;
  int local_28;
  size_t local_24;
  undefined4 local_20;
  uint *local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  int local_c;
  int local_8;
  
  local_1c = (uint *)0x0;
  local_18 = 0x1000;
  local_8 = 0;
  local_20 = 0x7a;
  local_10 = 0;
  local_c = 0;
  local_1c = operator_new(0x1000);
  *local_1c = local_18;
  local_14 = local_18;
  local_8 = FindFirstUrlCacheEntryA(0,local_1c,&local_14);
  local_10 = (uint)(local_8 != 0);
  do {
    if (local_10 != 1) {
      FUN_0058ea3a(local_1c);
      if (local_8 != 0) {
        FindCloseUrlCache(local_8);
      }
      return local_c != 0;
    }
    if ((local_1c != (uint *)0x0) && (local_1c[1] != 0)) {
      local_24 = 0;
      local_28 = 0;
      while (sVar1 = _strlen((char *)local_1c[1]), local_28 < (int)(sVar1 - 4)) {
        if ((((*(char *)(local_1c[1] + local_28) == '.') &&
             (*(char *)(local_1c[1] + 1 + local_28) == 'a')) &&
            (*(char *)(local_1c[1] + 2 + local_28) == 's')) &&
           ((*(char *)(local_1c[1] + 3 + local_28) == 'p' &&
            (*(char *)(local_1c[1] + 4 + local_28) == '?')))) {
          local_24 = local_28 + 4;
          break;
        }
        local_28 = local_28 + 1;
      }
      local_22c = DAT_013b7230;
      puVar3 = &local_22b;
      for (iVar2 = 0x7f; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
      *(undefined2 *)puVar3 = 0;
      *(undefined1 *)((int)puVar3 + 2) = 0;
      _strncpy(&local_22c,(char *)local_1c[1],local_24);
      local_2c = _strstr(&local_22c,param_1);
      if ((local_2c != (char *)0x0) && (iVar2 = _strcmp(param_1,local_2c), iVar2 == 0)) {
        DeleteUrlCacheEntry(local_1c[1]);
        local_c = local_c + 1;
      }
    }
    FUN_0058ea3a(local_1c);
    local_1c = (uint *)0x0;
    local_1c = operator_new(local_18);
    *local_1c = local_18;
    local_14 = local_18;
    local_10 = FindNextUrlCacheEntryA(local_8,local_1c,&local_14);
  } while( true );
}

