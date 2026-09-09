// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054fdb9 | Name: FUN_0054fdb9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0054fdb9(int param_1,int param_2,int *param_3,int param_4,int param_5,int param_6,
                 int param_7,int *param_8)

{
  byte bVar1;
  char cVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  bool bVar5;
  undefined1 uVar6;
  short sVar7;
  short sVar8;
  undefined2 uVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  undefined4 *puVar14;
  undefined4 *puVar15;
  undefined8 uVar16;
  int local_188;
  int local_184;
  int local_180;
  int local_17c;
  int local_164;
  int local_160;
  int local_15c;
  int local_158;
  undefined2 local_154;
  short local_150;
  int local_148;
  int local_140;
  int local_13c;
  short local_138;
  int local_130;
  int local_124;
  int local_114;
  int local_10c;
  int local_108;
  int local_104;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_ec;
  uint local_e8;
  undefined2 local_e0;
  short local_d8;
  int local_d4;
  short local_d0;
  short local_cc;
  short local_c4;
  int local_b4;
  int local_b0;
  int local_98;
  int local_94;
  short local_84;
  short local_78;
  short local_74;
  short local_70;
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_58;
  int local_50;
  int local_48;
  int local_44;
  int local_40;
  int local_38;
  int local_34;
  int local_2c;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  *(undefined1 *)(param_1 + 0x15) = 0;
  puVar14 = (undefined4 *)(param_1 + 0x24);
  puVar15 = (undefined4 *)(param_1 + 0x40);
  for (iVar13 = 7; iVar13 != 0; iVar13 = iVar13 + -1) {
    *puVar15 = *puVar14;
    puVar14 = puVar14 + 1;
    puVar15 = puVar15 + 1;
  }
  uVar16 = FUN_00559ef5(param_5);
  iVar13 = (int)((ulonglong)uVar16 >> 0x20);
  if ((int)uVar16 == 1) {
    local_6c = 0;
    while (local_6c < 4) {
      iVar13 = FUN_005588b2(CONCAT31((int3)((uint)iVar13 >> 8),*(undefined1 *)(param_4 + 1)),
                            local_6c);
      if (iVar13 != 0) {
        uVar3 = *(undefined4 *)(&DAT_005c38c8 + local_6c * 0xc);
        uVar4 = *(undefined4 *)(&DAT_005c38cc + local_6c * 0xc);
        local_74 = (short)*(undefined4 *)(&DAT_005c38c4 + local_6c * 0xc);
        *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) + local_74;
        local_70 = (short)uVar4;
        *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + local_70;
        local_78 = (short)uVar3;
        *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + local_78;
      }
      iVar13 = local_6c + 1;
      local_6c = iVar13;
    }
  }
  sVar7 = FUN_0054eb20(param_1,3);
  sVar8 = FUN_0054eb20(param_1,0x35);
  *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x44);
  sVar8 = FUN_0054eb20(param_1,2);
  *(short *)(param_1 + 0x44) = sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x48);
  sVar8 = FUN_0054eb20(param_1,4);
  *(short *)(param_1 + 0x48) = sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x4a);
  sVar8 = FUN_0054eb20(param_1,5);
  *(short *)(param_1 + 0x4a) = sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x50);
  sVar8 = FUN_0054eb20(param_1,7);
  *(short *)(param_1 + 0x50) = sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x52);
  sVar8 = FUN_0054eb20(param_1,8);
  *(short *)(param_1 + 0x52) = sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x54);
  sVar8 = FUN_0054eb20(param_1,9);
  *(short *)(param_1 + 0x54) = sVar7 + sVar8;
  sVar7 = *(short *)(param_1 + 0x56);
  sVar8 = FUN_0054eb20(param_1,10);
  *(short *)(param_1 + 0x56) = sVar7 + sVar8;
  bVar1 = *(byte *)(param_1 + 0x58);
  local_34 = FUN_0054eb20(param_1,0xb);
  local_34 = (uint)bVar1 + local_34;
  if (0xff < local_34) {
    local_34 = 0xff;
  }
  bVar1 = *(byte *)(param_1 + 0x59);
  iVar13 = FUN_0054eb20(param_1,0xc);
  local_2c = FUN_0054eb20(param_1,0x4a);
  local_2c = (uint)bVar1 + iVar13 + local_2c;
  if (0xff < local_2c) {
    local_2c = 0xff;
  }
  bVar1 = *(byte *)(param_1 + 0x5a);
  iVar13 = FUN_0054eb20(param_1,0xd);
  local_44 = FUN_0054eb20(param_1,0x4a);
  local_44 = (uint)bVar1 + iVar13 + local_44;
  if (0xff < local_44) {
    local_44 = 0xff;
  }
  bVar1 = *(byte *)(param_1 + 0x5b);
  iVar13 = FUN_0054eb20(param_1,0xe);
  local_40 = FUN_0054eb20(param_1,0x4a);
  local_40 = (uint)bVar1 + iVar13 + local_40;
  if (0xff < local_40) {
    local_40 = 0xff;
  }
  *(undefined1 *)(param_1 + 0x58) = (undefined1)local_34;
  *(undefined1 *)(param_1 + 0x59) = (undefined1)local_2c;
  *(undefined1 *)(param_1 + 0x5a) = (undefined1)local_44;
  *(undefined1 *)(param_1 + 0x5b) = (undefined1)local_40;
  uVar6 = FUN_0054eb20(param_1,0x2c);
  *(undefined1 *)(param_1 + 0x2e7) = uVar6;
  local_38 = FUN_0054eb20(param_1,0x3c);
  iVar13 = FUN_0054eb20(param_1,0x44);
  local_38 = local_38 + iVar13;
  bVar1 = *(byte *)(param_1 + 0x47);
  local_20 = FUN_0054eb20(param_1,0x1d);
  local_20 = (bVar1 & 0xf) + local_20;
  bVar1 = *(byte *)(param_1 + 0x2b);
  local_58 = FUN_0054eb20(param_1,0x1a);
  local_58 = ((int)(uint)bVar1 >> 4) * 10 + local_58;
  local_14 = FUN_0054eb20(param_1,0x2f);
  local_60 = FUN_0054eb20(param_1,0x30);
  local_10 = (int)*(short *)(param_1 + 0x5c);
  if ((local_10 < 1) || (((0x15 < local_10 && (local_10 < 0x1a)) || (local_10 == 0x20)))) {
    local_10 = param_5;
  }
  iVar13 = local_10 / 10;
  if (iVar13 < 4) {
    *(undefined1 *)(param_1 + 0x5e) = 0x2b;
    *(undefined1 *)(param_1 + 0x5f) = 0;
    if (((*(char *)(param_1 + 0x10) == '\a') || (*(char *)(param_1 + 0x10) == '\b')) ||
       (*(char *)(param_1 + 0x10) == '\t')) {
      *(undefined1 *)(param_1 + 0x10) = 0;
    }
  }
  iVar10 = FUN_00559ef5(param_5);
  if ((iVar10 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
    if (*(char *)(param_1 + 0x14) == '\0') {
      *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x370;
      *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x3b6;
      *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + 0x640;
      *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) + 300;
    }
    else if (*(char *)(param_1 + 0x14) == '\x01') {
      *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 800;
      *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x3b6;
      *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + 800;
      *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) + 0x6a4;
    }
    else if (*(char *)(param_1 + 0x14) == '\x02') {
      *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x370;
      *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x3b6;
      *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + 800;
      *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) + 0x514;
    }
    else if (*(char *)(param_1 + 0x14) == '\x03') {
      *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 800;
      *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x3b6;
      *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + 800;
      *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) + 700;
    }
  }
  sVar7 = *(short *)(param_1 + 0xd4);
  if ((*(char *)(param_1 + 0x10) != '\x04') &&
     ((((((sVar7 == 0x21f || (sVar7 == 0x221)) || (sVar7 == 0x6e6)) ||
        ((sVar7 == 0x6e7 || (sVar7 == 0x6e8)))) || (sVar7 == 0xc77)) ||
      ((sVar7 == 0xc7a || (sVar7 == 0xc7d)))))) {
    *(undefined1 *)(param_1 + 0x10) = 7;
  }
  if ((*(char *)(param_1 + 0x10) != '\x04') &&
     ((((sVar7 == 0x220 || (sVar7 == 0x222)) || (sVar7 == 0x6e9)) ||
      (((sVar7 == 0x6ea || (sVar7 == 0x6eb)) ||
       ((sVar7 == 0xc78 || ((sVar7 == 0xc7b || (sVar7 == 0xc7e)))))))))) {
    *(undefined1 *)(param_1 + 0x10) = 8;
  }
  if ((*(char *)(param_1 + 0x10) != '\x04') && (sVar7 == 0x6b8)) {
    *(undefined1 *)(param_1 + 0x10) = 9;
  }
  if ((*(char *)(param_1 + 0x10) != '\x04') && ((sVar7 == 0x2de || (sVar7 == 0x2e0)))) {
    *(undefined1 *)(param_1 + 0x10) = 7;
  }
  if ((*(char *)(param_1 + 0x10) != '\x04') && ((sVar7 == 0x2df || (sVar7 == 0x2e1)))) {
    *(undefined1 *)(param_1 + 0x10) = 8;
  }
  if ((((*(char *)(param_1 + 0x14) == '\x03') && ((*(uint *)(param_1 + 0x2dc) & 0x400) != 0)) &&
      (iVar10 = (int)*(short *)(param_1 + 0x8c), 0 < iVar10)) &&
     ((iVar10 < 0x1964 &&
      ((*(short *)(&DAT_00d449d4 + iVar10 * 0x8c) == 0x2a ||
       (*(short *)(&DAT_00d449d4 + iVar10 * 0x8c) == 0x2b)))))) {
    sVar7 = (short)(local_44 / 5);
    local_84 = sVar7 + 10;
    if ((*(uint *)(param_1 + 0x2dc) & 0x8000) != 0) {
      local_84 = sVar7 + 0x78;
    }
    *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + local_84;
  }
  iVar10 = FUN_0054eb20(param_1,0x2d);
  *(short *)(param_1 + 0x48) = (short)(((int)*(short *)(param_1 + 0x48) * (iVar10 + 100)) / 100);
  iVar10 = FUN_0054eb20(param_1,0x2e);
  *(short *)(param_1 + 0x4a) = (short)(((int)*(short *)(param_1 + 0x4a) * (iVar10 + 100)) / 100);
  for (local_c = 1; local_c < 8; local_c = local_c + 1) {
    iVar10 = (int)*(short *)(param_1 + 0x5c + local_c * 8);
    if (((0 < iVar10) && (iVar10 < 0x1964)) &&
       (iVar11 = FUN_0054e06c(param_1 + 0x5c + local_c * 8), 8 < iVar11)) {
      sVar7 = *(short *)(&DAT_00d449d6 + iVar10 * 0x8c);
      if (sVar7 == 2) {
        local_94 = *(char *)(param_1 + 0x2f0) + 0x1e;
        if (100 < local_94) {
          local_94 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f0) = (undefined1)local_94;
        local_94 = *(char *)(param_1 + 0x2f1) + 0x1e;
        if (100 < local_94) {
          local_94 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f1) = (undefined1)local_94;
        local_94 = *(char *)(param_1 + 0x2f2) + 0x1e;
        if (100 < local_94) {
          local_94 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f2) = (undefined1)local_94;
        local_94 = *(char *)(param_1 + 0x2f3) + 0x1e;
        if (100 < local_94) {
          local_94 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f3) = (undefined1)local_94;
      }
      else if (sVar7 == 4) {
        *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x19;
      }
      else if (sVar7 == 8) {
        *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x19;
      }
      else if (sVar7 == 0x10) {
        *(byte *)(param_1 + 0x15) = *(byte *)(param_1 + 0x15) | 0x40;
      }
      else if (sVar7 == 0x80) {
        *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + 0x19;
        local_98 = *(char *)(param_1 + 0x2f0) + 10;
        if (100 < local_98) {
          local_98 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f0) = (undefined1)local_98;
        local_98 = *(char *)(param_1 + 0x2f1) + 10;
        if (100 < local_98) {
          local_98 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f1) = (undefined1)local_98;
        local_98 = *(char *)(param_1 + 0x2f2) + 10;
        if (100 < local_98) {
          local_98 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f2) = (undefined1)local_98;
        local_98 = *(char *)(param_1 + 0x2f3) + 10;
        if (100 < local_98) {
          local_98 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f3) = (undefined1)local_98;
      }
      else if ((sVar7 == 0x40) || (sVar7 == 0xc0)) {
        if (*(short *)(&DAT_00d449d4 + iVar10 * 0x8c) == 0x2f) {
          local_38 = local_38 + 0x10;
        }
        else if (*(short *)(&DAT_00d449d4 + iVar10 * 0x8c) == 0x2c) {
          local_38 = local_38 + 0x10;
        }
        else {
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x28;
        }
      }
    }
  }
  local_50 = 0;
  local_68 = 0;
  local_64 = 100;
  local_8 = 100;
  local_1c = 100;
  local_18 = 100;
  for (local_48 = 0; local_48 < 0x10; local_48 = local_48 + 1) {
    if ((param_2 != 0) && (cVar2 = *(char *)(param_2 + local_48 * 8), cVar2 != '\0')) {
      local_d0 = *(short *)(param_2 + 2 + local_48 * 8);
      local_b4 = (int)local_d0;
      bVar1 = *(byte *)(param_2 + 1 + local_48 * 8);
      uVar12 = (uint)bVar1;
      if (cVar2 == '\x01') {
        local_20 = local_20 + -1;
        local_58 = local_58 + -0x1e;
      }
      else if (cVar2 == '\x02') {
        local_20 = local_20 + local_b4;
        *(byte *)(param_1 + 0x15) = *(byte *)(param_1 + 0x15) | 0x20;
      }
      else if (cVar2 == '\x03') {
        if (*(short *)(param_1 + 0x5c) < 0x32) {
          local_b4 = local_b4 + -0x19;
        }
        if (uVar12 == 0xff) {
          local_b4 = local_b4 + 10;
        }
        local_b0 = *(char *)(param_1 + 0x2f0) - local_b4;
        if (local_b0 < -0x32) {
          local_b0 = -0x32;
        }
        if (100 < local_b0) {
          local_b0 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f0) = (undefined1)local_b0;
        local_b0 = *(char *)(param_1 + 0x2f1) - local_b4;
        if (local_b0 < -0x32) {
          local_b0 = -0x32;
        }
        if (100 < local_b0) {
          local_b0 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f1) = (undefined1)local_b0;
        local_b0 = *(char *)(param_1 + 0x2f2) - local_b4;
        if (local_b0 < -0x32) {
          local_b0 = -0x32;
        }
        if (100 < local_b0) {
          local_b0 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f2) = (undefined1)local_b0;
        local_b0 = *(char *)(param_1 + 0x2f3) - local_b4;
        if (local_b0 < -0x32) {
          local_b0 = -0x32;
        }
        if (100 < local_b0) {
          local_b0 = 100;
        }
        *(undefined1 *)(param_1 + 0x2f3) = (undefined1)local_b0;
      }
      else if (cVar2 == '\x04') {
        if (local_b4 == 0) {
          local_20 = local_20 + 2;
          local_64 = local_64 + 3;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x14;
          local_38 = local_38 + 10;
        }
        else if (local_b4 == 1) {
          local_64 = local_64 + 5;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x28;
          local_38 = local_38 + 0xc;
        }
        else if (local_b4 == 2) {
          local_64 = local_64 + 6;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x3c;
          local_38 = local_38 + 0x10;
        }
        else if (local_b4 == 3) {
          local_64 = local_64 + 0xf;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x50;
          local_38 = local_38 + 0x14;
        }
      }
      else if (cVar2 == '\x05') {
        uVar9 = __ftol();
        *(undefined2 *)(param_1 + 0x54) = uVar9;
      }
      else if (cVar2 == '\x06') {
        uVar9 = __ftol();
        *(undefined2 *)(param_1 + 0x54) = uVar9;
      }
      else if (cVar2 == '\a') {
        local_58 = local_58 - (uVar12 / 10 + 10);
        if (0x32 < *(short *)(param_1 + 0x5c)) {
          local_c4 = *(short *)(param_1 + 0x52) - ((short)(uVar12 / 10) + 0x14);
          *(short *)(param_1 + 0x52) = local_c4;
        }
      }
      else if (cVar2 == '\b') {
        local_58 = local_58 + uVar12 / 10 + 10;
      }
      else if (cVar2 == '\t') {
        local_cc = bVar1 / 3 + 0xf;
        iVar10 = local_64 + 5;
        if ((*(char *)(param_1 + 0x14) == '\x01') && ((*(uint *)(param_1 + 0x2dc) & 0x100000) != 0))
        {
          local_cc = local_cc * 5;
          iVar10 = local_64 + 0x14;
          if ((*(uint *)(param_1 + 0x2dc) & 0x800000) != 0) {
            iVar10 = local_64 + 0x1b;
          }
        }
        local_64 = iVar10;
        *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + local_cc;
      }
      else if (cVar2 == '\n') {
        local_d0 = bVar1 / 5 + local_d0;
        local_d4 = (int)*(short *)(param_1 + 0x40);
        iVar10 = FUN_00559ef5(param_5);
        if ((iVar10 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
          local_d4 = 400;
        }
        if (uVar12 == 0xff) {
          local_d0 = local_d0 + (short)(local_d4 / 2);
        }
        *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) - local_d0;
      }
      else if (cVar2 == '\v') {
        local_d8 = bVar1 / 3 + local_d0;
        if ((*(char *)(param_1 + 0x14) == '\x01') && ((*(uint *)(param_1 + 0x2dc) & 0x800000) != 0))
        {
          local_d8 = local_d8 + 100;
        }
        *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + local_d8;
      }
      else if (cVar2 == '\f') {
        uVar9 = __ftol();
        *(undefined2 *)(param_1 + 0x42) = uVar9;
      }
      else if (cVar2 == '\r') {
        local_64 = local_64 + local_b4 + uVar12 / 10;
        if ((*(uint *)(param_1 + 0x2dc) & 0x8000) != 0) {
          local_64 = local_64 + 5;
        }
        local_e0 = (undefined2)((*(short *)(param_1 + 0x48) * 9) / 10);
        *(undefined2 *)(param_1 + 0x48) = local_e0;
      }
      else if (cVar2 == '\x0e') {
        local_ec = (int)*(short *)(param_1 + 0x40);
        iVar10 = FUN_00559ef5(param_5);
        if ((iVar10 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
          local_ec = local_ec + 400;
        }
        local_e8 = 0;
        if ((*(uint *)(param_1 + 0x2dc) & 0x8000) != 0) {
          local_e8 = uVar12;
        }
        *(ushort *)(param_1 + 0x56) =
             *(short *)(param_1 + 0x56) + local_d0 + (ushort)bVar1 + (short)((local_ec << 2) / 3) +
             (short)local_e8;
      }
      else if (cVar2 == '\x0f') {
        local_f4 = local_b4 + uVar12 / 10;
        if ((*(char *)(param_1 + 0x14) == '\x01') && ((*(uint *)(param_1 + 0x2dc) & 0x800000) != 0))
        {
          local_f4 = (local_f4 * 0x78) / 100;
        }
        local_f8 = (uint)*(byte *)(param_1 + 0x58) + local_f4;
        if (0xff < local_f8) {
          local_f8 = 0xff;
        }
        *(undefined1 *)(param_1 + 0x58) = (undefined1)local_f8;
        local_f8 = (uint)*(byte *)(param_1 + 0x59) + local_f4;
        if (0xff < local_f8) {
          local_f8 = 0xff;
        }
        *(undefined1 *)(param_1 + 0x59) = (undefined1)local_f8;
        local_f8 = (uint)*(byte *)(param_1 + 0x5a) + local_f4;
        if (0xff < local_f8) {
          local_f8 = 0xff;
        }
        *(undefined1 *)(param_1 + 0x5a) = (undefined1)local_f8;
        local_f8 = (uint)*(byte *)(param_1 + 0x5b) + local_f4;
        if (0xff < local_f8) {
          local_f8 = 0xff;
        }
        *(undefined1 *)(param_1 + 0x5b) = (undefined1)local_f8;
      }
      else if (cVar2 == '\x10') {
        local_b4 = local_b4 + -1;
        if (((-1 < local_b4) && (local_b4 < 5)) && (*(char *)(param_1 + 0x14) == '\x02')) {
          if (local_b4 == 4) {
            *(undefined2 *)(param_1 + 0x5c) = 0x20;
          }
          else {
            *(short *)(param_1 + 0x5c) = (short)local_b4 + 0x16;
          }
          local_10c = 0;
          local_108 = 0;
          local_fc = 0;
          local_124 = 0;
          local_104 = 0;
          if ((*(short *)(param_1 + 0x5c) == 0x16) && ((*(uint *)(param_1 + 0x2dc) & 0x20000) != 0))
          {
            local_10c = 10;
            local_104 = 0xf;
          }
          else if ((*(short *)(param_1 + 0x5c) == 0x17) &&
                  ((*(uint *)(param_1 + 0x2dc) & 0x80000) != 0)) {
            local_108 = 0x14;
            local_104 = 0x1e;
            local_124 = 0x14;
          }
          else if ((*(short *)(param_1 + 0x5c) == 0x18) &&
                  ((*(uint *)(param_1 + 0x2dc) & 0x200000) != 0)) {
            local_10c = 10;
            local_fc = 5;
            local_108 = 5;
            local_104 = 0xf;
            local_124 = 0x14;
          }
          else if (*(short *)(param_1 + 0x5c) == 0x19) {
            local_104 = 0x1e;
          }
          else if (*(short *)(param_1 + 0x5c) == 0x20) {
            local_104 = 0x1b;
            local_124 = 10;
          }
          if ((*(uint *)(param_1 + 0x2dc) & 0x800000) != 0) {
            local_10c = local_10c + 5;
            local_fc = local_fc + 2;
          }
          iVar10 = FUN_00559ef5(param_5);
          if ((iVar10 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
            local_130 = 9;
          }
          else {
            local_130 = ((local_40 + *(short *)(param_1 + 0x40) * 2) / 3 -
                        *(int *)(&DAT_005c36f0 + local_b4 * 0x44)) / 0xc;
          }
          if (local_130 < 1) {
            local_130 = 0;
          }
          if (9 < local_130) {
            local_130 = 9;
          }
          *(undefined1 *)(param_1 + 0x5e) = 0x2b;
          *(undefined1 *)(param_1 + 0x5f) = (undefined1)local_130;
          iVar10 = *(int *)(&DAT_005c36b0 + local_b4 * 0x44);
          iVar11 = *(int *)(&DAT_005c36b4 + local_b4 * 0x44);
          local_138 = *(short *)(param_1 + 0x44);
          if (*(short *)(param_1 + 0x5c) == 0x16) {
            local_138 = local_138 + 10;
          }
          *(short *)(param_1 + 0x44) = local_138;
          local_64 = local_64 + -100 +
                     iVar10 + local_10c +
                     (int)(((iVar11 + local_10c) - (iVar10 + local_10c)) * uVar12) / 200;
          local_138 = (short)(((int)*(short *)(param_1 + 0x42) *
                              (*(int *)(&DAT_005c36b8 + local_b4 * 0x44) + local_fc +
                              (int)(((*(int *)(&DAT_005c36bc + local_b4 * 0x44) + local_fc) -
                                    (*(int *)(&DAT_005c36b8 + local_b4 * 0x44) + local_fc)) * uVar12
                                   ) / 200)) / 100);
          if (*(short *)(param_1 + 0x5c) == 0x16) {
            local_138 = local_138 + 5;
          }
          *(short *)(param_1 + 0x42) = local_138;
          local_138 = (short)(((int)*(short *)(param_1 + 0x48) *
                              (*(int *)(&DAT_005c36c0 + local_b4 * 0x44) + local_108 +
                              (int)(((*(int *)(&DAT_005c36c4 + local_b4 * 0x44) + local_108) -
                                    (*(int *)(&DAT_005c36c0 + local_b4 * 0x44) + local_108)) *
                                   uVar12) / 200)) / 100);
          *(short *)(param_1 + 0x48) = local_138;
          local_114 = *(char *)(param_1 + 0x2f0) + local_104;
          if (local_114 < -0x32) {
            local_114 = -0x32;
          }
          if (100 < local_114) {
            local_114 = 100;
          }
          *(undefined1 *)(param_1 + 0x2f0) = (undefined1)local_114;
          local_114 = *(char *)(param_1 + 0x2f1) + local_104;
          if (local_114 < -0x32) {
            local_114 = -0x32;
          }
          if (100 < local_114) {
            local_114 = 100;
          }
          *(undefined1 *)(param_1 + 0x2f1) = (undefined1)local_114;
          local_114 = *(char *)(param_1 + 0x2f2) + local_104;
          if (local_114 < -0x32) {
            local_114 = -0x32;
          }
          if (100 < local_114) {
            local_114 = 100;
          }
          *(undefined1 *)(param_1 + 0x2f2) = (undefined1)local_114;
          local_114 = *(char *)(param_1 + 0x2f3) + local_104;
          if (local_114 < -0x32) {
            local_114 = -0x32;
          }
          if (100 < local_114) {
            local_114 = 100;
          }
          *(undefined1 *)(param_1 + 0x2f3) = (undefined1)local_114;
          local_50 = *(int *)(&DAT_005c36d0 + local_b4 * 0x44) + local_124;
          local_68 = *(int *)(&DAT_005c36c8 + local_b4 * 0x44);
        }
      }
      else if (cVar2 == '\x12') {
        local_13c = (int)*(short *)(param_1 + 0x40);
        iVar10 = FUN_00559ef5(param_5);
        if ((iVar10 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
          local_13c = 400;
        }
        local_140 = ((int)((int)*(short *)(param_1 + 0x4a) +
                          ((int)*(short *)(param_1 + 0x4a) >> 0x1f & 3U)) >> 2) + uVar12 + local_13c
        ;
        if (*(short *)(param_1 + 0x4a) < local_140) {
          local_140 = *(short *)(param_1 + 0x4a) + -1;
        }
        *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + (short)local_140;
        *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) - (short)local_140;
      }
      else if (cVar2 == '\x13') {
        *(byte *)(param_1 + 0x15) = *(byte *)(param_1 + 0x15) | 0x80;
      }
      else if (cVar2 == '\x15') {
        *(ushort *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) - (bVar1 / 3 + 10);
        local_148 = local_b4 + uVar12 / 10;
        if ((*(uint *)(param_1 + 0x2dc) & 0x80) != 0) {
          local_148 = local_148 + 5;
        }
        local_64 = local_64 + local_148;
      }
      else if (cVar2 == '\x19') {
        *(short *)(param_1 + 0x42) =
             *(short *)(param_1 + 0x42) + (short)((int)uVar12 >> 2) + local_d0;
      }
      else if (cVar2 == '\x1a') {
        *(byte *)(param_1 + 0x15) = *(byte *)(param_1 + 0x15) | 8;
      }
      else if (cVar2 == '\x1b') {
        *(byte *)(param_1 + 0x15) = *(byte *)(param_1 + 0x15) | 1;
      }
      else if (cVar2 == '\x1c') {
        *(byte *)(param_1 + 0x15) = *(byte *)(param_1 + 0x15) | 0x10;
      }
      else if (cVar2 == '\x1d') {
        iVar10 = FUN_00559ef5(param_5);
        if (iVar10 == 1) {
          if (*(char *)(param_1 + 0x14) == '\0') {
            if ((*(uint *)(param_1 + 0x2dc) & 0x8000) == 0) {
              *(short *)(param_1 + 0x52) =
                   *(short *)(param_1 + 0x52) + (short)((*(short *)(param_1 + 0x52) * 0x5a) / 100);
              *(short *)(param_1 + 0x56) =
                   *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x28) / 100);
            }
            else {
              *(short *)(param_1 + 0x50) =
                   *(short *)(param_1 + 0x50) + (short)((*(short *)(param_1 + 0x50) * 0x5a) / 100);
              *(short *)(param_1 + 0x56) =
                   *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x3c) / 100);
            }
          }
          else if (*(char *)(param_1 + 0x14) == '\x01') {
            if ((*(uint *)(param_1 + 0x2dc) & 0x80) == 0) {
              if ((*(uint *)(param_1 + 0x2dc) & 0x800000) == 0) {
                if ((*(uint *)(param_1 + 0x2dc) & 0x8000) != 0) {
                  *(short *)(param_1 + 0x52) =
                       *(short *)(param_1 + 0x52) + *(short *)(param_1 + 0x52);
                  *(short *)(param_1 + 0x56) =
                       *(short *)(param_1 + 0x56) +
                       (short)((*(short *)(param_1 + 0x56) * 0x28) / 100);
                }
              }
              else {
                *(short *)(param_1 + 0x50) = *(short *)(param_1 + 0x50) + *(short *)(param_1 + 0x50)
                ;
                *(short *)(param_1 + 0x56) =
                     *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x14) / 100)
                ;
              }
            }
            else {
              *(short *)(param_1 + 0x56) =
                   *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x3c) / 100);
            }
          }
          else if (*(char *)(param_1 + 0x14) == '\x02') {
            if ((*(uint *)(param_1 + 0x2dc) & 0x800000) == 0) {
              *(short *)(param_1 + 0x52) = *(short *)(param_1 + 0x52) + *(short *)(param_1 + 0x52);
              *(short *)(param_1 + 0x56) =
                   *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x28) / 100);
            }
            else {
              *(short *)(param_1 + 0x50) =
                   *(short *)(param_1 + 0x50) + (short)((*(short *)(param_1 + 0x50) * 0x78) / 100);
              *(short *)(param_1 + 0x56) =
                   *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x14) / 100);
            }
          }
          else if (*(char *)(param_1 + 0x14) == '\x03') {
            *(short *)(param_1 + 0x50) = *(short *)(param_1 + 0x50) + *(short *)(param_1 + 0x50);
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x28) / 100);
          }
        }
        else if ((*(uint *)(param_1 + 0x2dc) & 0x40000000) == 0) {
          *(undefined2 *)(param_1 + 0x50) = 2000;
          *(undefined2 *)(param_1 + 0x52) = 2000;
          *(undefined2 *)(param_1 + 0x48) = 10000;
        }
        else if (*(char *)(param_1 + 0x14) == '\0') {
          if ((*(uint *)(param_1 + 0x2dc) & 0x8000) == 0) {
            *(short *)(param_1 + 0x52) =
                 *(short *)(param_1 + 0x52) + (short)((*(short *)(param_1 + 0x52) * 0x3c) / 100);
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x14) / 100);
          }
          else {
            *(short *)(param_1 + 0x50) =
                 *(short *)(param_1 + 0x50) + (short)((*(short *)(param_1 + 0x50) * 0x3c) / 100);
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x1e) / 100);
          }
        }
        else if (*(char *)(param_1 + 0x14) == '\x01') {
          if ((*(uint *)(param_1 + 0x2dc) & 0x80) == 0) {
            if ((*(uint *)(param_1 + 0x2dc) & 0x8000) != 0) {
              *(short *)(param_1 + 0x52) =
                   *(short *)(param_1 + 0x52) + *(short *)(param_1 + 0x52) / 2;
              *(short *)(param_1 + 0x56) =
                   *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x28) / 100);
            }
          }
          else {
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x1e) / 100);
          }
          if ((*(uint *)(param_1 + 0x2dc) & 0x800000) != 0) {
            *(short *)(param_1 + 0x50) = *(short *)(param_1 + 0x50) + *(short *)(param_1 + 0x50) / 2
            ;
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 10) / 100);
          }
        }
        else if (*(char *)(param_1 + 0x14) == '\x02') {
          if ((*(uint *)(param_1 + 0x2dc) & 0x800000) == 0) {
            *(short *)(param_1 + 0x52) =
                 *(short *)(param_1 + 0x52) + (short)((*(short *)(param_1 + 0x52) * 0x3c) / 100);
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x14) / 100);
          }
          else {
            *(short *)(param_1 + 0x50) =
                 *(short *)(param_1 + 0x50) + (short)((*(short *)(param_1 + 0x50) * 0x3c) / 100);
            *(short *)(param_1 + 0x56) =
                 *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x14) / 100);
          }
        }
        else if (*(char *)(param_1 + 0x14) == '\x03') {
          *(short *)(param_1 + 0x50) = *(short *)(param_1 + 0x50) + *(short *)(param_1 + 0x50) / 2;
          *(short *)(param_1 + 0x56) =
               *(short *)(param_1 + 0x56) + (short)((*(short *)(param_1 + 0x56) * 0x14) / 100);
        }
      }
      else if (cVar2 == '\x1f') {
        local_150 = bVar1 / 10 + local_d0;
        if ((*(uint *)(param_1 + 0x2dc) & 0x8000) != 0) {
          local_150 = local_150 + 100;
        }
        *(short *)(param_1 + 0x42) = *(short *)(param_1 + 0x42) + local_150;
      }
      else if (cVar2 == '!') {
        local_154 = 0;
        if (local_b4 == 0) {
          local_154 = 0xca;
        }
        if (local_b4 == 1) {
          local_154 = 0xd1;
        }
        if (local_b4 == 2) {
          local_154 = 0xd4;
        }
        if (local_b4 == 3) {
          local_154 = 0xe6;
        }
        if (local_b4 == 4) {
          local_154 = 0xe5;
        }
        if (local_b4 == 5) {
          local_154 = 0xd8;
        }
        if (local_b4 == 6) {
          local_154 = 0xe2;
        }
        if (local_b4 == 7) {
          local_154 = 0x12a;
        }
        *(undefined2 *)(param_1 + 0x5c) = local_154;
      }
      else if (cVar2 == '\"') {
        if (local_b4 == 6) {
          local_64 = local_64 + 6;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x28;
          local_38 = local_38 + 8;
        }
        else if ((local_b4 == 4) || (local_b4 == 5)) {
          local_64 = local_64 + 6;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x28;
          local_38 = local_38 + 8;
        }
        else if (local_b4 == 2) {
          local_64 = local_64 + 3;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 0x14;
          local_38 = local_38 + 4;
        }
        else if (local_b4 == 1) {
          local_64 = local_64 + 1;
          *(short *)(param_1 + 0x44) = *(short *)(param_1 + 0x44) + 10;
          local_38 = local_38 + 2;
        }
        else if (local_b4 == -10) {
          local_64 = local_64 + -10;
          if (local_64 < 0) {
            local_64 = 0;
          }
          local_38 = local_38 + -4;
          if (local_38 < 0) {
            local_38 = 0;
          }
          for (local_158 = 0; local_158 < 4; local_158 = local_158 + 1) {
            if (*(byte *)(param_1 + local_158 + 0x58) < 0x97) {
              uVar6 = *(undefined1 *)(param_1 + local_158 + 0x58);
            }
            else {
              uVar6 = 0x96;
            }
            *(undefined1 *)(param_1 + local_158 + 0x58) = uVar6;
          }
        }
        else if (local_b4 == -0xf) {
          local_64 = local_64 + -0xf;
          if (local_64 < 0) {
            local_64 = 0;
          }
          local_38 = local_38 + -6;
          if (local_38 < 0) {
            local_38 = 0;
          }
          for (local_15c = 0; local_15c < 4; local_15c = local_15c + 1) {
            if (*(byte *)(param_1 + local_15c + 0x58) < 0x65) {
              uVar6 = *(undefined1 *)(param_1 + local_15c + 0x58);
            }
            else {
              uVar6 = 100;
            }
            *(undefined1 *)(param_1 + local_15c + 0x58) = uVar6;
          }
        }
        else if (local_b4 == -0x14) {
          local_64 = local_64 + -0x14;
          if (local_64 < 0) {
            local_64 = 0;
          }
          local_38 = local_38 + -10;
          if (local_38 < 0) {
            local_38 = 0;
          }
          for (local_160 = 0; local_160 < 4; local_160 = local_160 + 1) {
            *(undefined1 *)(param_1 + local_160 + 0x58) = 0;
          }
        }
        local_8 = local_8 + local_b4;
      }
      else if (cVar2 == '#') {
        if (*(int *)(param_2 + 4 + local_48 * 8) < 1000000) {
          local_1c = local_1c + 10;
          local_18 = local_18 + 10;
        }
        else {
          local_1c = local_1c + 0x14;
          local_18 = local_18 + 0x14;
        }
      }
      else if (cVar2 == '$') {
        *(byte *)(param_1 + 0x2e6) = (*(byte *)(param_1 + 0x5a) / 0x32) * '\n' + '\x1e';
        local_8 = local_8 + 7 + *(byte *)(param_1 + 0x5a) / 0x32;
      }
      else if (cVar2 == '%') {
        *param_8 = (*(byte *)(param_1 + 0x59) / 10) * 3 + 0x18;
      }
      else if (cVar2 == '&') {
        local_164 = (int)*(short *)(param_1 + 0x40);
        iVar10 = FUN_00559ef5(param_5);
        if (iVar10 == 1) {
          if ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0) {
            local_164 = local_164 + 400;
          }
          local_164 = (uint)*(byte *)(param_1 + 0x5a) + local_164;
          sVar7 = (short)((int)((int)*(short *)(param_1 + 0x4a) +
                               ((int)*(short *)(param_1 + 0x4a) >> 0x1f & 3U)) >> 2) +
                  (short)((int)(local_164 + (local_164 >> 0x1f & 3U)) >> 2);
          *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + 0xfa + sVar7;
          *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) - sVar7;
        }
        else {
          local_164 = (uint)*(byte *)(param_1 + 0x5a) + local_164;
          sVar7 = (short)((int)((int)*(short *)(param_1 + 0x4a) +
                               ((int)*(short *)(param_1 + 0x4a) >> 0x1f & 7U)) >> 3) +
                  (short)((int)(local_164 + (local_164 >> 0x1f & 3U)) >> 2);
          *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + 0xfa + sVar7;
          *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) - sVar7;
        }
      }
    }
  }
  if (iVar13 < 4) {
    if (local_10 % 10 < 6) {
      *(ushort *)(param_1 + 0x44) =
           *(short *)(param_1 + 0x44) + *(short *)(param_1 + 0x50) / 3 +
           (ushort)*(byte *)(param_1 + 0x58) + *(short *)(param_1 + 0x40);
    }
    else {
      *(ushort *)(param_1 + 0x44) =
           *(short *)(param_1 + 0x44) + *(short *)(param_1 + 0x50) / 3 +
           (ushort)*(byte *)(param_1 + 0x58) + *(short *)(param_1 + 0x40) * 2;
    }
  }
  if (local_64 != 100) {
    *(short *)(param_1 + 0x44) = (short)((*(short *)(param_1 + 0x44) * local_64) / 100);
  }
  if (local_8 != 100) {
    *(short *)(param_1 + 0x42) = (short)((*(short *)(param_1 + 0x42) * local_8) / 100);
  }
  if (local_1c != 100) {
    *(short *)(param_1 + 0x48) = (short)((*(short *)(param_1 + 0x48) * local_1c) / 100);
  }
  if (local_18 != 100) {
    *(short *)(param_1 + 0x4a) = (short)((*(short *)(param_1 + 0x4a) * local_18) / 100);
  }
  if (local_14 < 0) {
    local_14 = 0;
  }
  if (0xff < local_14) {
    local_14 = 0xff;
  }
  *(undefined1 *)(param_1 + 0x2ee) = (undefined1)local_14;
  if (local_60 < 0) {
    local_60 = 0;
  }
  if (0xff < local_60) {
    local_60 = 0xff;
  }
  *(undefined1 *)(param_1 + 0x2ef) = (undefined1)local_60;
  bVar1 = *(byte *)(param_1 + 0x11);
  if (((4 < iVar13) && (99 < *(byte *)(param_1 + 0x11))) && (*(byte *)(param_1 + 0x11) < 200)) {
    bVar1 = 8;
  }
  *(byte *)(param_1 + 0x46) = *(byte *)(param_1 + 0x46) & 0xf0 | bVar1;
  local_58 = local_58 + local_50 + (int)*(short *)(param_1 + 0x54) / 5;
  local_20 = local_20 + local_68;
  if (local_20 < 1) {
    local_20 = 0;
  }
  if (6 < local_20) {
    local_20 = 6;
  }
  if (iVar13 < 5) {
    iVar10 = (int)*(short *)(param_1 + 0xcc);
    if (((param_6 >> 7 < 0x1b) || (0x1e < param_6 >> 7)) ||
       ((param_7 >> 7 < 0x15 || (0x18 < param_7 >> 7)))) {
      bVar5 = false;
    }
    else {
      bVar5 = true;
    }
    if (bVar5) {
      local_20 = local_20 + -2;
    }
    if (0 < *(short *)(param_1 + 0xce)) {
      if ((iVar10 < 0x938) || (0x956 < iVar10)) {
        if ((0xf8b < iVar10) &&
           ((iVar10 < 4000 && (local_20 < *(int *)(&DAT_005c0ab8 + (iVar10 + -0xf8c) * 0x18))))) {
          local_20 = *(int *)(&DAT_005c0ab8 + (iVar10 + -0xf8c) * 0x18);
        }
      }
      else if (local_20 < *(int *)(&DAT_005c07e8 + (iVar10 + -0x938) * 0x18)) {
        local_20 = *(int *)(&DAT_005c07e8 + (iVar10 + -0x938) * 0x18);
      }
    }
    if (local_20 < 1) {
      local_20 = 0;
    }
    if (7 < local_20) {
      local_20 = 7;
    }
  }
  if (local_58 < 0) {
    local_58 = 0;
  }
  if (0x96 < local_58) {
    local_58 = 0x96;
  }
  *(char *)(param_1 + 0x47) = (char)(local_58 / 10 << 4) + (char)local_20;
  if (iVar13 < 4) {
    local_180 = 100;
    local_184 = 100;
    local_188 = 100;
    local_17c = 100;
    if (DAT_00e37d18 != 0) {
      local_180 = DAT_00e37d18 + 100;
    }
    if (DAT_00e37d1c != 0) {
      local_184 = DAT_00e37d1c + 100;
    }
    if (DAT_00e37d20 != 0) {
      local_188 = DAT_00e37d20 + 100;
    }
    if (DAT_00e37d24 != 0) {
      local_17c = DAT_00e37d24 + 100;
    }
    *(short *)(param_1 + 0x44) = (short)((*(short *)(param_1 + 0x44) * local_184) / 100);
    *(short *)(param_1 + 0x42) = (short)((*(short *)(param_1 + 0x42) * local_17c) / 100);
    local_38 = (local_38 * local_188) / 100;
    *(short *)(param_1 + 0x48) = (short)((*(short *)(param_1 + 0x48) * local_180) / 100);
    iVar13 = FUN_00559ef5(param_5);
    if (((iVar13 != 1) || ((*(uint *)(param_1 + 0x2dc) & 0x40000000) == 0)) &&
       ((int)(((uint)(*(int *)(&DAT_005c12b4 + *(short *)(param_1 + 0x24) * 4) -
                     *(int *)(&DAT_005c12b0 + *(short *)(param_1 + 0x24) * 4)) / 10 << 2) / 5) <=
        *param_3)) {
      *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) / 2;
    }
  }
  if (999 < local_38) {
    local_38 = 1000;
  }
  *(char *)(param_1 + 0x2ed) = (char)((int)(local_38 + (local_38 >> 0x1f & 3U)) >> 2);
  if (*(short *)(param_1 + 0x48) < *(short *)(param_1 + 0x4c)) {
    *(undefined2 *)(param_1 + 0x4c) = *(undefined2 *)(param_1 + 0x48);
  }
  if (*(short *)(param_1 + 0x4a) < *(short *)(param_1 + 0x4e)) {
    *(undefined2 *)(param_1 + 0x4e) = *(undefined2 *)(param_1 + 0x4a);
  }
  return;
}

