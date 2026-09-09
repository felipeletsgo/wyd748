// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b17e2 | Name: FUN_004b17e2


char * FUN_004b17e2(char *param_1)

{
  size_t sVar1;
  int iVar2;
  int iVar3;
  undefined1 local_94 [128];
  size_t local_14;
  int local_10;
  char *local_c;
  int local_8;
  
  if (param_1 == (char *)0x0) {
    param_1 = (char *)0x0;
  }
  else {
    do {
      local_8 = 0;
      local_c = (char *)0x0;
      for (local_10 = 0; local_10 < 0x100; local_10 = local_10 + 1) {
        sVar1 = _strlen(&DAT_006703b0 + local_10 * 0x20);
        if (sVar1 == 0) {
          return param_1;
        }
        local_c = _strstr(param_1,&DAT_006703b0 + local_10 * 0x20);
        if (local_c != (char *)0x0) {
          iVar3 = (int)local_c - (int)param_1;
          local_14 = _strlen(&DAT_006703b0 + local_10 * 0x20);
          iVar2 = FUN_00421b0f(param_1,iVar3);
          if (iVar2 != 0) {
            _memset(local_94,0,0x80);
            FUN_0058f220(local_94,param_1,iVar3);
            FUN_0058ee30(local_94,&DAT_006703c0 + local_10 * 0x20);
            FUN_0058ee30(local_94,param_1 + local_14 + iVar3);
            FUN_0058f078(param_1,local_94);
            local_8 = 1;
            break;
          }
        }
      }
    } while (local_8 == 1);
  }
  return param_1;
}

