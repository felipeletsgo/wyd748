// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00533dd7 | Name: FUN_00533dd7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00533dd7(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_94 [12];
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  int local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  uint local_10;
  uint local_c;
  uint local_8;
  
  local_14 = FUN_0058f716(param_2,&DAT_005bde78);
  if (local_14 == 0) {
    FUN_00431159(s_>>_Fail_to_Load_TileMap____s_005bdeb8,param_2);
    uVar4 = 0;
  }
  else {
    FUN_00590d68(&local_10,1,1,local_14);
    FUN_00590d68(param_1 + 0x24,1,local_10 & 0xff,local_14);
    *(undefined1 *)(param_1 + 0x24 + (local_10 & 0xff)) = 0;
    FUN_00590d68(&local_8,1,1,local_14);
    FUN_00590d68(&local_c,1,1,local_14);
    FUN_0053277e(local_8 & 0xff,local_c & 0xff);
    FUN_00532916(local_8 & 0xff,local_c & 0xff);
    FUN_00590d68(param_1 + 0xcc,0xc,0x1000,local_14);
    FUN_0058f62c(local_14);
    iVar3 = FUN_00494dcf();
    if (iVar3 == 30000) {
      local_24 = 0;
      local_30 = 0xc000;
      local_2c = param_1 + 0xcc;
      for (local_28 = 0; local_28 < 0xc000; local_28 = local_28 + 1) {
        local_24 = local_24 + *(char *)(local_2c + local_28);
      }
      if (*(int *)(&DAT_005bbe18 + (local_8 & 0xff) * 4 + (local_c & 0xff) * 0x80) !=
          local_24 + (local_c & 0xff) * (local_8 & 0xff)) {
        FUN_00431159(s_CheckSum_Error____d__d_m_nCheckS_005bde7c,local_8 & 0xff,local_c & 0xff,
                     *(undefined4 *)(&DAT_005bbe18 + (local_8 & 0xff) * 4 + (local_c & 0xff) * 0x80)
                     ,local_24 + (local_c & 0xff) * (local_8 & 0xff));
        if (*(int *)(DAT_0067cf38 + 0x54) == 0) {
          FUN_0049ed58(5,0,0,0,0);
        }
        *(undefined4 *)(DAT_0067cf38 + 0x54) = 1;
        return 0;
      }
    }
    for (local_20 = 1; local_20 < 0x3f; local_20 = local_20 + 1) {
      for (local_34 = 1; local_34 < 0x3f; local_34 = local_34 + 1) {
        uVar4 = FUN_00534ebe(local_94,local_34,local_20);
        FUN_004310a0(uVar4);
      }
    }
    for (local_18 = 0; local_18 < 0x40; local_18 = local_18 + 1) {
      FUN_004310a0(param_1 + 0x140cc + (local_18 * 0x40 + 1) * 0xc);
      FUN_004310a0(param_1 + 0x140cc + (local_18 * 0x40 + 0x3e) * 0xc);
    }
    for (local_18 = 0; local_18 < 0x40; local_18 = local_18 + 1) {
      FUN_004310a0(param_1 + 0x140cc + (local_18 + 0x40) * 0xc);
      FUN_004310a0(param_1 + 0x140cc + (local_18 + 0xf80) * 0xc);
    }
    _memset((void *)(param_1 + 0x200cc),0,0x44);
    *(undefined4 *)(param_1 + 0x200cc) = 0x3f800000;
    *(undefined4 *)(param_1 + 0x200d0) = 0x3f800000;
    *(undefined4 *)(param_1 + 0x200d4) = 0x3f800000;
    *(undefined4 *)(param_1 + 0x200ec) = *(undefined4 *)(param_1 + 0x200cc);
    *(undefined4 *)(param_1 + 0x200f0) = *(undefined4 *)(param_1 + 0x200d0);
    *(undefined4 *)(param_1 + 0x200f4) = *(undefined4 *)(param_1 + 0x200d4);
    *(undefined4 *)(param_1 + 0x200f8) = *(undefined4 *)(param_1 + 0x200d8);
    *(undefined4 *)(param_1 + 0x2010c) = 0;
    *(undefined4 *)(param_1 + 0x200fc) = 0x3e99999a;
    *(undefined4 *)(param_1 + 0x20100) = 0x3e99999a;
    *(undefined4 *)(param_1 + 0x20104) = 0x3e99999a;
    if (*(int *)(DAT_005ccf9c + 0x2a390) == 1) {
      *(undefined4 *)(param_1 + 0x200fc) = 0x3e4ccccd;
      *(undefined4 *)(param_1 + 0x20100) = 0x3e4ccccd;
      *(undefined4 *)(param_1 + 0x20104) = 0x3e4ccccd;
    }
    for (local_20 = 0; local_20 < 0x40; local_20 = local_20 + 1) {
      *(undefined1 *)(param_1 + 0xcc + local_20 * 0x300) =
           *(undefined1 *)(param_1 + 0xcc + (local_20 * 0x40 + 1) * 0xc);
    }
    for (local_1c = 0; local_1c < 0x40; local_1c = local_1c + 1) {
      *(undefined1 *)(param_1 + 0xcc + local_1c * 0xc) =
           *(undefined1 *)(param_1 + 0xcc + (local_1c + 0x40) * 0xc);
    }
    for (local_20 = 0; local_20 < 0x40; local_20 = local_20 + 1) {
      for (local_1c = 0; local_1c < 0x40; local_1c = local_1c + 1) {
        local_48 = (float)(int)*(char *)(param_1 + 0xcc + (local_1c + local_20 * 0x40) * 0xc);
        if (local_20 < 0x3e) {
          cVar1 = *(char *)(param_1 + 0xcc + (local_1c + (local_20 + 1) * 0x40) * 0xc);
        }
        else {
          cVar1 = *(char *)(param_1 + 0xcc + (local_1c + local_20 * 0x40) * 0xc);
        }
        local_3c = (float)(int)cVar1;
        if (local_1c < 0x3e) {
          local_38 = (float)(int)*(char *)(param_1 + 0xcc + (local_1c + 1 + local_20 * 0x40) * 0xc);
          if (local_20 < 0x3e) {
            cVar1 = *(char *)(param_1 + 0xcc + (local_1c + 1 + (local_20 + 1) * 0x40) * 0xc);
          }
          else {
            cVar1 = *(char *)(param_1 + 0xcc + (local_1c + 1 + local_20 * 0x40) * 0xc);
          }
          local_40 = (float)(int)cVar1;
        }
        else {
          local_38 = (float)(int)*(char *)(param_1 + 0xcc + (local_1c + local_20 * 0x40) * 0xc);
          if (local_20 < 0x3e) {
            local_40 = (float)(int)*(char *)(param_1 + 0xcc +
                                            (local_1c + (local_20 + 1) * 0x40) * 0xc);
          }
          else {
            local_40 = (float)(int)*(char *)(param_1 + 0xcc + (local_1c + local_20 * 0x40) * 0xc);
          }
        }
        local_44 = (local_48 + local_38 + local_3c + local_40) / _DAT_005a3834;
        uVar2 = __ftol();
        *(undefined1 *)(param_1 + 0xc0cc + local_20 * 0x100 + local_1c * 2) = uVar2;
        uVar2 = __ftol();
        *(undefined1 *)(param_1 + local_20 * 0x100 + 0xc0cd + local_1c * 2) = uVar2;
        uVar2 = __ftol();
        *(undefined1 *)(param_1 + 0xc0cc + (local_20 * 2 + 1) * 0x80 + local_1c * 2) = uVar2;
        uVar2 = __ftol();
        *(undefined1 *)(param_1 + (local_20 * 2 + 1) * 0x80 + 0xc0cd + local_1c * 2) = uVar2;
      }
    }
    if (*(char *)(param_1 + 0x20a32) == '\0') {
      for (local_4c = 0; local_4c < 0x80; local_4c = local_4c + 1) {
        for (local_50 = 0; local_50 < 0xf; local_50 = local_50 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_4c + local_50 * 0x80) = 0x7f;
        }
      }
    }
    if (*(char *)(param_1 + 0x20a33) == '\0') {
      for (local_54 = 0; local_54 < 0x80; local_54 = local_54 + 1) {
        for (local_58 = 0x72; local_58 < 0x80; local_58 = local_58 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_54 + local_58 * 0x80) = 0x7f;
        }
      }
    }
    if (*(char *)(param_1 + 0x20a30) == '\0') {
      for (local_5c = 0; local_5c < 0x80; local_5c = local_5c + 1) {
        for (local_60 = 0; local_60 < 0xf; local_60 = local_60 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_60 + local_5c * 0x80) = 0x7f;
        }
      }
    }
    if (*(char *)(param_1 + 0x20a31) == '\0') {
      for (local_64 = 0; local_64 < 0x80; local_64 = local_64 + 1) {
        for (local_68 = 0x72; local_68 < 0x80; local_68 = local_68 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_68 + local_64 * 0x80) = 0x7f;
        }
      }
    }
    if ((*(char *)(param_1 + 0x20a30) == '\x01') && (*(char *)(param_1 + 0x20a33) == '\x01')) {
      for (local_6c = 0; local_6c < 0x10; local_6c = local_6c + 1) {
        for (local_70 = 0x71; local_70 < 0x80; local_70 = local_70 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_6c + local_70 * 0x80) = 0x7f;
        }
      }
    }
    if ((*(char *)(param_1 + 0x20a30) == '\x01') && (*(char *)(param_1 + 0x20a32) == '\x01')) {
      for (local_74 = 0; local_74 < 0x10; local_74 = local_74 + 1) {
        for (local_78 = 0; local_78 < 0x10; local_78 = local_78 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_74 + local_78 * 0x80) = 0x7f;
        }
      }
    }
    if ((*(char *)(param_1 + 0x20a31) == '\x01') && (*(char *)(param_1 + 0x20a33) == '\x01')) {
      for (local_7c = 0x71; local_7c < 0x80; local_7c = local_7c + 1) {
        for (local_80 = 0x71; local_80 < 0x80; local_80 = local_80 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_7c + local_80 * 0x80) = 0x7f;
        }
      }
    }
    if ((*(char *)(param_1 + 0x20a31) == '\x01') && (*(char *)(param_1 + 0x20a32) == '\x01')) {
      for (local_84 = 0x71; local_84 < 0x80; local_84 = local_84 + 1) {
        for (local_88 = 0; local_88 < 0x10; local_88 = local_88 + 1) {
          *(undefined1 *)(param_1 + 0xc0cc + local_84 + local_88 * 0x80) = 0x7f;
        }
      }
    }
    uVar4 = 1;
  }
  return uVar4;
}

