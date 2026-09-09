// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00484d44 | Name: FUN_00484d44


undefined4 __thiscall FUN_00484d44(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  size_t sVar3;
  uint uVar4;
  int local_418;
  uint local_414;
  int local_410;
  char local_40c [256];
  char *local_30c;
  int local_308;
  int local_304;
  size_t local_300;
  int local_2fc;
  undefined1 local_2f8 [4];
  undefined2 local_2f4;
  undefined2 local_2f2;
  char local_2ec [16];
  undefined1 local_2dc [12];
  undefined4 local_2d0;
  undefined4 local_2cc;
  undefined1 local_2c8 [4];
  undefined1 local_2c4 [4];
  undefined1 local_2c0 [4];
  undefined1 local_2bc [4];
  char local_2b8 [52];
  undefined1 local_284 [640];
  
  if (((uint)*(ushort *)(param_2 + 6) == DAT_013b71e8[0x317]) && (*DAT_013b71e4 == 0)) {
    local_2fc = param_2;
    FUN_00403df2(&DAT_00a344f8,0);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,0);
    DAT_005d04e0 = 0;
    local_304 = 0;
    FUN_005909d2(local_2fc + 0x1c,&PTR_DAT_005b8870,&local_304);
    DAT_013b71e8[0x6bd4] = local_304;
    FUN_00494d83(DAT_013b71e8[0x6bd4]);
    local_308 = DAT_013b71e8[0x6bd3];
    FUN_0058f078(DAT_013b71e0 + 0x60,&DAT_005b8874,
                 &DAT_009325f8 + local_304 * 0x40 + local_308 * 0x2c0);
    iVar1 = FUN_00423f37(DAT_013b71e0 + 0x60,0x2059,0,0x464);
    if (iVar1 == 0) {
      FUN_00403df2(&DAT_00a34578,4000);
      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
      if (*(int *)(param_1 + 0x24) != 0x7532) {
        (**(code **)(*DAT_013b71e8 + 0x54))(7);
      }
    }
    else {
      _memset(local_2f8,0,0x74);
      local_2f2 = 0;
      local_2f4 = 0x20d;
      local_2d0 = 0x2ec;
      local_2cc = 1;
      local_300 = 0;
      _memset(local_284,0,0x280);
      GetAdaptersInfo(local_284,&local_300);
      if (local_300 != 0) {
        pvVar2 = _malloc(local_300);
        GetAdaptersInfo(pvVar2,&local_300);
        local_30c = (char *)((int)pvVar2 + 8);
        local_418 = 0;
        local_414 = 0;
        sVar3 = _strlen(local_30c);
        for (local_410 = 0; local_410 < (int)sVar3; local_410 = local_410 + 1) {
          iVar1 = local_418;
          if (((local_30c[local_410] != '{') && (local_30c[local_410] != '}')) &&
             (local_30c[local_410] != '-')) {
            local_40c[local_418] = local_30c[local_410];
            local_414 = local_414 + 1;
            uVar4 = local_414 & 0x80000007;
            if ((int)uVar4 < 0) {
              uVar4 = (uVar4 - 1 | 0xfffffff8) + 1;
            }
            iVar1 = local_418 + 1;
            if (uVar4 == 0) {
              local_40c[local_418 + 1] = ' ';
              iVar1 = local_418 + 2;
            }
          }
          local_418 = iVar1;
        }
        local_40c[local_418] = '\0';
        FUN_005909d2(local_40c,s__x__x__x__x_005b8878,local_2c8,local_2c4,local_2c0,local_2bc);
        FUN_0058ffd8(pvVar2);
      }
      _strncpy(local_2ec,(char *)(local_2fc + 0xc),0x10);
      _strncpy(local_2b8,(char *)(local_2fc + 0x1c),0x34);
      FUN_0058f078(local_2dc,&DAT_005d04d0);
      FUN_0055f2dd(local_2f8,0x74);
    }
  }
  else if (((uint)*(ushort *)(param_2 + 6) == DAT_013b71e8[0x317]) && (*DAT_013b71e4 != 0)) {
    FUN_0058f220(param_1 + 0x28bbc,param_2,0x50);
    *(undefined4 *)(param_1 + 0x26e18) = 1;
  }
  return 1;
}

