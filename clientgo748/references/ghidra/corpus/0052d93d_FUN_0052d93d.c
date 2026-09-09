// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052d93d | Name: FUN_0052d93d


undefined4 __thiscall FUN_0052d93d(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int local_3c;
  undefined1 local_38 [4];
  undefined2 local_34;
  undefined2 local_32;
  char local_2c [20];
  int local_18;
  uint local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = param_2;
  if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
    if (*(int *)(DAT_013b71e8 + 0x9c8) != *(int *)(param_2 + 0x14)) {
      if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x1f), iVar1 != 0)) {
        uVar3 = 0;
        uVar2 = 0;
        FUN_00429a6d(0x1f);
        FUN_0042ad2b(uVar2,uVar3);
      }
      local_10 = 0;
      for (local_c = 0; local_c < 0x14; local_c = local_c + 1) {
        local_18 = (int)*(char *)(DAT_013b71e8 + local_c + 0xef6) +
                   (uint)*(byte *)(DAT_013b71e8 + 0x700) * -0x18;
        if ('_' < *(char *)(DAT_013b71e8 + local_c + 0xef6)) {
          local_18 = *(char *)(DAT_013b71e8 + local_c + 0xef6) + -0x48;
        }
        local_14 = 1 << ((byte)local_18 & 0x1f);
        if ((((*(uint *)(local_8 + 0x14) & local_14) != local_14) &&
            (-1 < *(char *)(DAT_013b71e8 + local_c + 0xef6))) &&
           (*(char *)(DAT_013b71e8 + local_c + 0xef6) < '\x18')) {
          *(undefined1 *)(DAT_013b71e8 + local_c + 0xef6) = 0xff;
          local_10 = 1;
        }
      }
      if (local_10 == 1) {
        _memset(local_38,0,0x20);
        local_32 = *(undefined2 *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x20);
        local_34 = 0x378;
        FUN_0058f220(local_2c,DAT_013b71e8 + 0xef6,0x14);
        for (local_3c = 0; local_3c < 0x14; local_3c = local_3c + 1) {
          if ((-1 < local_2c[local_3c]) && (local_2c[local_3c] < '`')) {
            local_2c[local_3c] = local_2c[local_3c] + *(char *)(DAT_013b71e8 + 0x700) * -0x18;
          }
        }
        FUN_0055f2dd(local_38,0x20);
      }
    }
    *(undefined4 *)(DAT_013b71e8 + 0x9c8) = *(undefined4 *)(local_8 + 0x14);
    *(undefined4 *)(DAT_013b71e8 + 0x708) = *(undefined4 *)(local_8 + 0x10);
    *(undefined2 *)(DAT_013b71e8 + 0x9cc) = *(undefined2 *)(local_8 + 0x18);
    *(undefined2 *)(DAT_013b71e8 + 0x9ce) = *(undefined2 *)(local_8 + 0x1a);
    *(undefined2 *)(DAT_013b71e8 + 0x9d0) = *(undefined2 *)(local_8 + 0x1c);
    *(undefined4 *)(DAT_013b71e8 + 0x704) = *(undefined4 *)(local_8 + 0x20);
    *(undefined4 *)(DAT_013b71e8 + 0xc50) = *(undefined4 *)(local_8 + 0xc);
    *(undefined4 *)(DAT_013b71e8 + 0x2cc + *(char *)(DAT_013b71e8 + 0xef4) * 4) =
         *(undefined4 *)(local_8 + 0x20);
    iVar1 = FUN_00494dcf();
    if (iVar1 == 30000) {
      FUN_004431e4(0);
    }
  }
  return 1;
}

