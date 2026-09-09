// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004829f2 | Name: FUN_004829f2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004829f2(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  void *pvVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  float fVar7;
  undefined4 uVar8;
  int local_27c;
  int local_278;
  int local_274;
  int local_270;
  int local_268;
  int local_254;
  int local_250;
  int *local_1f8;
  int local_17c;
  uint local_16c;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_150;
  undefined4 local_14c;
  undefined4 local_140;
  undefined4 local_13c;
  undefined4 local_138;
  int local_128;
  undefined4 local_11c;
  ushort local_114 [4];
  uint local_10c;
  int local_108;
  float local_104;
  int local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  uint local_f4;
  undefined4 local_f0;
  uint local_ec;
  int local_e8;
  int local_e4;
  byte local_e0;
  undefined2 *local_dc;
  undefined4 local_d8;
  int local_d4;
  ushort local_d0 [4];
  undefined4 local_c8;
  uint local_c4;
  undefined1 local_c0 [8];
  uint local_b8;
  float local_b4;
  int local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  undefined1 auStack_98 [128];
  int *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fab0;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x34) != 0) {
    local_9c = param_2;
    ExceptionList = &local_10;
    local_18 = (int *)(**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 0x10));
    local_14 = (int *)0x0;
    if ((uint)*(ushort *)(local_9c + 0x10) == *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
      for (local_a4 = 0; local_a4 < 0x10; local_a4 = local_a4 + 1) {
        FUN_0058f220(auStack_98 + local_a4 * 8,*(int *)(param_1 + 0x4c) + 0x6a4 + local_a4 * 8,8);
      }
      _DAT_005b9a48 = *(int *)(param_1 + 0x4c) + 0x28;
      _DAT_005b9a54 = *(int *)(param_1 + 0x4c) + 0x28;
      if (*(int *)(param_1 + 0x28794) == 0) {
        *(int *)(param_1 + 0x28794) = (int)*(short *)(local_9c + 0xc);
        *(int *)(param_1 + 0x28798) = (int)*(short *)(local_9c + 0xe);
      }
    }
    iVar2 = __ftol();
    if (((iVar2 >> 7 < 0x11) || (iVar2 = __ftol(), 0x13 < iVar2 >> 7)) ||
       (iVar2 = __ftol(), iVar2 >> 7 < 0x1e)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      if (((uint)*(ushort *)(local_9c + 0x10) == *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) ||
         (999 < *(ushort *)(local_9c + 0x10))) {
        if ((uint)*(ushort *)(local_9c + 0x10) != *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
          *(undefined2 *)(local_9c + 0x62) = 0;
        }
      }
      else {
        iVar2 = __ftol();
        if ((iVar2 >> 7 == 0x12) && (iVar2 = __ftol(), iVar2 >> 7 == 0x1e)) {
          bVar1 = true;
        }
        else {
          bVar1 = false;
        }
        if (bVar1) {
          if (*(short *)(*(int *)(param_1 + 0x4c) + 0x680) != *(short *)(local_9c + 0x62)) {
            FUN_0058f078(local_9c + 0x12,&DAT_005b883c);
            *(undefined2 *)(local_9c + 0x62) = 0;
          }
        }
        else {
          FUN_0058f078(local_9c + 0x12,&DAT_005b8844);
          *(undefined2 *)(local_9c + 0x62) = 0;
        }
        *(undefined1 *)(local_9c + 0x1f) = 0;
        *(undefined1 *)(local_9c + 0x20) = 0;
        FUN_0058f078(local_9c + 0x92,&DAT_005d04c4);
      }
    }
    iVar2 = __ftol();
    if ((iVar2 >> 7 == 0x11) && (iVar2 = __ftol(), iVar2 >> 7 == 0x1c)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (bVar1) {
      if (((uint)*(ushort *)(local_9c + 0x10) == *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) ||
         (999 < *(ushort *)(local_9c + 0x10))) {
        if ((uint)*(ushort *)(local_9c + 0x10) != *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
          *(undefined2 *)(local_9c + 0x62) = 0;
        }
      }
      else {
        FUN_0058f078(local_9c + 0x12,&DAT_005b884c);
        *(undefined2 *)(local_9c + 0x62) = 0;
        *(undefined1 *)(local_9c + 0x1f) = 0;
        *(undefined1 *)(local_9c + 0x20) = 0;
        FUN_0058f078(local_9c + 0x92,&DAT_005d04c8);
      }
    }
    if (local_18 == (int *)0x0) {
      pvVar3 = operator_new(0x894);
      local_8 = 0;
      if (pvVar3 == (void *)0x0) {
        local_1f8 = (int *)0x0;
      }
      else {
        local_1f8 = (int *)FUN_004f7ea6(param_1);
      }
      local_8 = 0xffffffff;
      local_14 = local_1f8;
      if (local_1f8 == (int *)0x0) {
        ExceptionList = local_10;
        return 1;
      }
      local_1f8[8] = (uint)*(ushort *)(local_9c + 0x10);
      *(undefined2 *)(local_1f8 + 0x1a0) = *(undefined2 *)(local_9c + 0x62);
      if (*(ushort *)(local_9c + 0x10) < 1000) {
        local_e0 = *(byte *)(local_9c + 0x1f);
        local_dc = (undefined2 *)(local_9c + 0x20);
        *(ushort *)((int)local_1f8 + 0x192) = (ushort)local_e0;
        *(undefined2 *)(local_1f8 + 0x65) = *local_dc;
        *(undefined1 *)((int)local_1f8 + 0x196) = *(undefined1 *)(local_9c + 0x1e);
        *(undefined1 *)(local_9c + 0x1e) = 0;
        *(undefined1 *)(local_9c + 0x21) = 0;
        *(undefined1 *)(local_9c + 0xab) = 0;
        FUN_0058f078(local_1f8 + 0x5a,&DAT_005b8854,local_9c + 0x12);
        FUN_0058f078(local_14 + 0x5e,&DAT_005b8858,local_9c + 0x92);
        *(short *)((int)local_14 + 0x7ba) = (short)*(char *)(local_9c + 0xac);
      }
      else {
        *(undefined2 *)((int)local_1f8 + 0x192) = 0;
        *(undefined2 *)(local_1f8 + 0x65) = 0;
        *(undefined1 *)((int)local_1f8 + 0x196) = 0x4b;
        *(undefined1 *)(local_9c + 0x21) = 0;
        *(undefined1 *)(local_9c + 0xab) = 0;
        FUN_0058f078(local_1f8 + 0x5a,&DAT_005b885c,local_9c + 0x12);
        FUN_0058f078(local_14 + 0x5e,&DAT_005b8860,local_9c + 0x92);
        *(undefined2 *)((int)local_14 + 0x7ba) = 0;
      }
      FUN_0058f220(local_14 + 0x1a1,local_9c + 0x42,0x20);
      FUN_00526140(local_9c + 0x22);
      FUN_00527032(local_9c + 0x82);
      local_14[0x88] = 0x40000000;
      FUN_0058f220(local_14 + 0x111,local_9c + 100,0x1c);
      local_b4 = (float)(int)*(short *)((int)local_14 + 0x45a);
      FUN_005277a7(local_b4);
      if (*(ushort *)(local_9c + 0x22) < 0x28) {
        local_14[0xe] = (int)((float)local_14[0xe] * _DAT_005a40f4);
      }
      FUN_004faf13(*(ushort *)(local_9c + 0x22) & 0xfff);
      _memset(local_c0,0,8);
      FUN_0058f220(local_c0,local_9c + 0x22,8);
      _memset(local_d0,0,8);
      local_d0[0] = *(ushort *)(local_9c + 0x2e) & 0xfff;
      local_b0 = FUN_0054cd07(local_d0,0x15);
      if (local_b0 == 0x29) {
        *(undefined2 *)((int)local_14 + 0x1ea) = *(undefined2 *)((int)local_14 + 0x1ee);
        *(short *)(local_14 + 0x7b) = (short)local_14[0x7c];
        *(undefined1 *)(local_14 + 0x7e) = *(undefined1 *)((int)local_14 + 0x1f9);
        *(undefined1 *)(local_14 + 0x80) = *(undefined1 *)((int)local_14 + 0x201);
      }
      (**(code **)(*local_14 + 0x38))();
      if (local_14 == *(int **)(param_1 + 0x4c)) {
        for (local_e4 = 0; local_e4 < 0x10; local_e4 = local_e4 + 1) {
          FUN_0058f220(*(int *)(param_1 + 0x4c) + 0x6a4 + local_e4 * 8,auStack_98 + local_e4 * 8,8);
        }
      }
      FUN_0052433d();
      FUN_0051bb41(*(ushort *)(local_9c + 0x2e) & 0xfff,*(ushort *)(local_9c + 0x30) & 0xfff);
      *(ushort *)((int)local_14 + 0x682) = *(ushort *)(local_9c + 0x3a) & 0xfff;
      local_b8 = CONCAT22(local_b8._2_2_,*(short *)(local_9c + 0x62));
      if (*(short *)(local_9c + 0x62) != 0) {
        local_e8 = FUN_00552e21((int)*(short *)((int)local_14 + 0x682));
        local_14[0x1fc] = local_b8 & 0xfff;
        local_14[0x1fa] = local_e8;
        local_14[0x1fb] = (int)(local_b8 & 0xffff) >> 0xc;
        *(undefined2 *)(local_14 + 0x220) = *(undefined2 *)((int)local_14 + 0x682);
        if ((*(short *)(param_2 + 4) != 0x363) && (iVar2 = FUN_0040c0f0(), iVar2 == 0)) {
          FUN_0047dee5(local_14 + 0x1f9);
        }
      }
      local_d8 = 0;
      local_d4 = (int)(uint)*(byte *)((int)local_14 + 0x44a) >> 4;
      if (local_d4 == 6) {
        local_d8 = 0x40490fdb;
      }
      if (local_d4 == 9) {
        local_d8 = 0x4016cbe4;
      }
      if (local_d4 == 8) {
        local_d8 = 0x3fc90fdb;
      }
      if (local_d4 == 7) {
        local_d8 = 0x3f490fdb;
      }
      if (local_d4 == 4) {
        local_d8 = 0;
      }
      if (local_d4 == 1) {
        local_d8 = 0x40afede0;
      }
      if (local_d4 == 2) {
        local_d8 = 0x4096cbe4;
      }
      if (local_d4 == 3) {
        local_d8 = 0x407b53d2;
      }
      (**(code **)(*local_14 + 0x40))(0,local_d8,0);
      FUN_00430f20((float)(int)*(short *)(local_9c + 0xc) + _DAT_005a34a0,
                   (float)(int)*(short *)(local_9c + 0xe) + _DAT_005a34a0);
      iVar2 = FUN_0049c70f(local_ac,local_a8);
      (**(code **)(*local_14 + 0x3c))(local_ac,(float)iVar2 * _DAT_005a4214,local_a8);
      if (((uint)local_14[8] < 1000) && ((*(byte *)((int)local_14 + 0x44a) & 1) == 1)) {
        *(undefined1 *)((int)local_14 + 0x795) = 1;
      }
      else {
        *(undefined1 *)((int)local_14 + 0x795) = 0;
      }
      if ((999 < (uint)local_14[8]) &&
         ((iVar2 = FUN_00516105(), iVar2 != 0 || ((*(byte *)((int)local_14 + 0x44a) & 0xf) == 0xf)))
         ) {
        (**(code **)(*(int *)local_14[0x106] + 0x84))(0xffaaffaa);
      }
      local_c4 = *(ushort *)(local_9c + 0x3c) & 0xfff;
      local_c8 = 0;
      if (((local_c4 == 0x312) || (local_c4 == 0x790)) || (local_c4 == 0x791)) {
        local_f0 = 0;
        local_f4 = (int)(uint)*(ushort *)(local_9c + 0x3c) >> 0xc;
        if (local_f4 < 2) {
          local_f4 = 2;
        }
        if (local_c4 == 0x790) {
          local_f4 = local_f4 * 10;
        }
        else if (local_c4 == 0x791) {
          local_f4 = local_f4 * 1000;
        }
        local_ec = local_f4 * (int)*(short *)(local_9c + 0x70);
        if (2000000000 < local_ec) {
          local_ec = 2000000000;
        }
        local_14[0x86] = local_ec;
        local_ec = local_f4 * (int)*(short *)(local_9c + 0x6c);
        if (2000000000 < local_ec) {
          local_ec = 2000000000;
        }
        local_14[0x87] = local_ec;
      }
      if ((local_14[0x93] == 0x38) && (*(short *)((int)local_14 + 0x1d2) == 0)) {
        *(int *)(param_1 + 0x276bc) = local_14[8];
      }
      if ((((uint)local_14[8] < 1000) || (iVar2 = __ftol(), iVar2 >> 7 < 0x1c)) ||
         ((iVar2 = __ftol(), 0x1e < iVar2 >> 7 ||
          ((iVar2 = __ftol(), iVar2 >> 7 < 0x1b || (iVar2 = __ftol(), 0x1c < iVar2 >> 7)))))) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) {
        FUN_004f9946(0xaaff0000);
      }
      FUN_0054ac09(local_14);
    }
    else if (local_18[0x95] == 0) {
      local_18[0x95] = -1;
      local_18[0x17e] = 0;
    }
    else {
      local_108 = local_18[0x17b];
      local_18[0x95] = -1;
      local_18[0x17e] = 0;
      local_14 = local_18;
      FUN_004f9334();
      local_14[8] = (uint)*(ushort *)(local_9c + 0x10);
      *(undefined2 *)(local_14 + 0x1a0) = *(undefined2 *)(local_9c + 0x62);
      if (*(ushort *)(local_9c + 0x10) < 1000) {
        *(ushort *)((int)local_14 + 0x192) = (ushort)*(byte *)(local_9c + 0x1f);
        *(undefined2 *)(local_14 + 0x65) = *(undefined2 *)(local_9c + 0x20);
        *(undefined1 *)((int)local_14 + 0x196) = *(undefined1 *)(local_9c + 0x1e);
        *(undefined1 *)(local_9c + 0x1e) = 0;
        *(undefined1 *)(local_9c + 0x21) = 0;
        *(undefined1 *)(local_9c + 0x20) = 0;
        FUN_0058f078(local_14 + 0x5a,&DAT_005b8864,local_9c + 0x12);
      }
      else {
        *(undefined2 *)((int)local_14 + 0x192) = 0;
        *(undefined2 *)(local_14 + 0x65) = 0;
        *(undefined1 *)((int)local_14 + 0x196) = 0x4b;
        *(undefined1 *)(local_9c + 0x21) = 0;
        *(undefined1 *)(local_9c + 0x20) = 0;
        FUN_0058f078(local_14 + 0x5a,&DAT_005b8868,local_9c + 0x12);
      }
      *(undefined1 *)(local_9c + 0xab) = 0;
      FUN_0058f078(local_14 + 0x5e,&DAT_005b886c,local_9c + 0x92);
      FUN_0058f220(local_14 + 0x1a1,local_9c + 0x42,0x20);
      FUN_00526140(local_9c + 0x22);
      FUN_00527032(local_9c + 0x82);
      local_14[0x88] = 0x40000000;
      FUN_0058f220(local_14 + 0x111,local_9c + 100,0x1c);
      local_104 = (float)(int)*(short *)((int)local_14 + 0x45a);
      FUN_005277a7(local_104);
      FUN_004faf13(*(ushort *)(local_9c + 0x22) & 0xfff);
      _memset(local_114,0,8);
      local_114[0] = *(ushort *)(local_9c + 0x2e) & 0xfff;
      local_100 = FUN_0054cd07(local_114,0x15);
      if (local_100 == 0x29) {
        *(undefined2 *)((int)local_14 + 0x1ea) = *(undefined2 *)((int)local_14 + 0x1ee);
        *(short *)(local_14 + 0x7b) = (short)local_14[0x7c];
        *(undefined1 *)(local_14 + 0x7e) = *(undefined1 *)((int)local_14 + 0x1f9);
        *(undefined1 *)(local_14 + 0x80) = *(undefined1 *)((int)local_14 + 0x201);
      }
      (**(code **)(*local_14 + 0x38))();
      if (local_14 == *(int **)(param_1 + 0x4c)) {
        for (local_128 = 0; local_128 < 0x10; local_128 = local_128 + 1) {
          FUN_0058f220(local_14 + local_128 * 2 + 0x1a9,auStack_98 + local_128 * 8,8);
        }
      }
      FUN_0052433d();
      FUN_0051bb41(*(ushort *)(local_9c + 0x2e) & 0xfff,*(ushort *)(local_9c + 0x30) & 0xfff);
      local_11c = 0;
      iVar2 = (int)(uint)*(byte *)((int)local_14 + 0x44a) >> 4;
      if (iVar2 == 6) {
        local_11c = 0x40490fdb;
      }
      if (iVar2 == 9) {
        local_11c = 0x4016cbe4;
      }
      if (iVar2 == 8) {
        local_11c = 0x3fc90fdb;
      }
      if (iVar2 == 7) {
        local_11c = 0x3f490fdb;
      }
      if (iVar2 == 4) {
        local_11c = 0;
      }
      if (iVar2 == 1) {
        local_11c = 0x40afede0;
      }
      if (iVar2 == 2) {
        local_11c = 0x4096cbe4;
      }
      if (iVar2 == 3) {
        local_11c = 0x407b53d2;
      }
      (**(code **)(*local_14 + 0x40))(0,local_11c,0);
      FUN_00430f20((float)(int)*(short *)(local_9c + 0xc) + _DAT_005a34a0,
                   (float)(int)*(short *)(local_9c + 0xe) + _DAT_005a34a0);
      iVar2 = FUN_0049c70f(local_fc,local_f8);
      (**(code **)(*local_14 + 0x3c))(local_fc,(float)iVar2 * _DAT_005a4214,local_f8);
      *(ushort *)((int)local_14 + 0x682) = *(ushort *)(local_9c + 0x3a) & 0xfff;
      local_10c = CONCAT22(local_10c._2_2_,*(short *)(local_9c + 0x62));
      if (*(short *)(local_9c + 0x62) != 0) {
        iVar2 = FUN_00552e21((int)*(short *)((int)local_14 + 0x682));
        local_14[0x1fc] = local_10c & 0xfff;
        local_14[0x1fa] = iVar2;
        local_14[0x1fb] = (int)(local_10c & 0xffff) >> 0xc;
        *(undefined2 *)(local_14 + 0x220) = *(undefined2 *)((int)local_14 + 0x682);
        if ((*(short *)(param_2 + 4) != 0x363) && (iVar2 = FUN_0040c0f0(), iVar2 == 0)) {
          FUN_0047dee5(local_14 + 0x1f9);
        }
      }
      if (((uint)local_14[8] < 1000) && ((*(byte *)((int)local_14 + 0x44a) & 1) == 1)) {
        *(undefined1 *)((int)local_14 + 0x795) = 1;
      }
      else {
        *(undefined1 *)((int)local_14 + 0x795) = 0;
      }
      if (((999 < (uint)local_14[8]) && ((*(byte *)((int)local_14 + 0x44a) & 0xf) != 0)) &&
         ((*(byte *)((int)local_14 + 0x44a) & 0xf) < 0x10)) {
        (**(code **)(*(int *)local_14[0x106] + 0x84))(0xffaaffaa);
      }
      if ((999 < (uint)local_14[8]) && ((short)local_14[0x94] == 0x36)) {
        (**(code **)(*(int *)local_14[0x106] + 0x84))(0xffaaffaa);
      }
      if (local_14 == *(int **)(param_1 + 0x4c)) {
        FUN_004431e4(0);
      }
      local_18[0x17b] = local_108;
    }
    if (local_14 != (int *)0x0) {
      *(undefined4 *)(local_14[0x106] + 0xd10) = 0x55aa0000;
      if ((999 < (uint)local_14[8]) && (*(short *)((int)local_14 + 0x446) == 0)) {
        *(undefined4 *)(local_14[0x106] + 0xd10) = 0x5500aa00;
        *(undefined1 *)(local_14[0x106] + 0xcdc) = 1;
        *(undefined1 *)((int)local_14 + 0x249) = 1;
      }
      if (*(short *)(param_2 + 4) == 0x363) {
        *(undefined1 *)(param_2 + 0xc3) = 0;
        *(undefined1 *)(param_2 + 0xc2) = 0;
        FUN_0058f078(local_14 + 0x1c9,param_2 + 0xac);
        (**(code **)(*(int *)local_14[0x109] + 0x80))(local_14 + 0x1c9,0);
      }
      else {
        _memset(local_14 + 0x1c9,0,0x18);
        (**(code **)(*(int *)local_14[0x109] + 0x80))(&DAT_005d04cc,0);
        if (((*(int *)(param_1 + 0x27b38) != 0) && (iVar2 = FUN_0040c0f0(), iVar2 == 1)) &&
           (local_14[8] == (uint)*(ushort *)(param_1 + 0x275ae))) {
          FUN_0044ae38(0,0);
        }
      }
    }
    if ((local_14 != (int *)0x0) && (*(char *)((int)local_14 + 0x795) == '\0')) {
      if (((*(char *)((int)local_14 + 0x795) == '\0') &&
          (((*(ushort *)(local_9c + 0x80) & 0x7fff) == 2 ||
           ((*(ushort *)(local_9c + 0x80) & 0x7fff) == 3)))) && (local_14 != (int *)0x0)) {
        fVar7 = (float)(int)*(short *)(local_9c + 0xe) + _DAT_005a34a0;
        iVar2 = FUN_0049c70f(local_14[10],local_14[0xb]);
        FUN_00430f50((float)(int)*(short *)(local_9c + 0xc) + _DAT_005a34a0,
                     (float)iVar2 * _DAT_005a4214 + _DAT_005a439c,fVar7);
        if ((local_14[0x17] == 0) || (local_14[0x17] == 1)) {
          pvVar3 = operator_new(0x94);
          local_8 = 1;
          if (pvVar3 == (void *)0x0) {
            local_250 = 0;
          }
          else {
            local_250 = FUN_004d8ef0(local_140,local_13c,local_138,0,0);
          }
          local_8 = 0xffffffff;
          if ((local_250 != 0) && (*(int *)(param_1 + 0x2c) != 0)) {
            FUN_0054ac09(local_250);
          }
        }
        else if ((((*(ushort *)(local_9c + 0x80) & 0x7fff) != 3) && (local_14[0x17] != 0x23)) &&
                (local_14[0x17] != 0x24)) {
          pvVar3 = operator_new(0x94);
          local_8 = 2;
          if (pvVar3 == (void *)0x0) {
            local_254 = 0;
          }
          else {
            local_254 = FUN_004d8ef0(local_140,local_13c,local_138,1,0);
          }
          local_8 = 0xffffffff;
          if ((local_254 != 0) && (*(int *)(param_1 + 0x2c) != 0)) {
            FUN_0054ac09(local_254);
          }
          if (((DAT_005ccf98 != 0) && (*(int **)(param_1 + 0x4c) == local_14)) &&
             (iVar2 = FUN_00429a6d(0x97), iVar2 != 0)) {
            uVar8 = 0;
            uVar6 = 0;
            FUN_00429a6d(0x97);
            FUN_0042ad2b(uVar6,uVar8);
          }
        }
        if ((*(ushort *)(local_9c + 0x80) & 0x7fff) == 3) {
          if ((local_14[0x93] == 0x3e) && (*(short *)((int)local_14 + 0x1d2) == 2)) {
            FUN_00430f20((float)(int)*(short *)(local_9c + 0xc) + _DAT_005a34a0,
                         (float)(int)*(short *)(local_9c + 0xe) + _DAT_005a34a0);
            uVar6 = local_14c;
            iVar2 = FUN_0049c70f(local_150,local_14c);
            (**(code **)(*local_14 + 0x3c))
                      (local_150,(float)iVar2 * _DAT_005a4214 - _DAT_005a365c,uVar6);
            iVar2 = FUN_0049c70f(local_150,local_14c);
            FUN_00430f50(local_150,(float)iVar2 * _DAT_005a4214 + _DAT_005a4158,local_14c);
            pvVar3 = operator_new(0x94);
            local_8 = 3;
            if (pvVar3 == (void *)0x0) {
              local_268 = 0;
            }
            else {
              local_268 = FUN_004e6590(local_160,local_15c,local_158,4,0x3dcccccd);
            }
            local_8 = 0xffffffff;
            if (local_268 != 0) {
              FUN_0054ac09(local_268);
            }
          }
          else {
            local_164 = 1;
            if (999 < (uint)local_14[8]) {
              local_164 = 3;
            }
            pvVar3 = operator_new(0xac);
            local_8 = 4;
            if (pvVar3 == (void *)0x0) {
              local_270 = 0;
            }
            else {
              local_270 = FUN_004ee730(local_140,local_13c,local_138,local_164);
            }
            local_8 = 0xffffffff;
            if ((local_270 != 0) && (*(int *)(param_1 + 0x2c) != 0)) {
              FUN_0054ac09(local_270);
            }
          }
        }
        FUN_0051dba4(0xe,0);
        if ((local_14[0x17] == 0x23) || (local_14[0x17] == 0x24)) {
          if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x12f), iVar2 != 0)) {
            uVar8 = 0;
            uVar6 = 0;
            FUN_00429a6d(0x12f);
            FUN_0042ad2b(uVar6,uVar8);
          }
          if (DAT_0092e660 == 0) {
            for (local_16c = 0xfffffffd; (int)local_16c < 3; local_16c = local_16c + 1) {
              pvVar3 = operator_new(0x180);
              local_8 = 5;
              if (pvVar3 == (void *)0x0) {
                local_274 = 0;
              }
              else {
                local_274 = FUN_004c8f70(0xc1,4000,0x3f800000,0x3f800000,0x3f800000,0x3a83126f,1,
                                         0x50);
              }
              local_8 = 0xffffffff;
              if (local_274 != 0) {
                uVar5 = local_16c & 0x80000001;
                if ((int)uVar5 < 0) {
                  uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
                }
                *(uint *)(local_274 + 0x100) = uVar5;
                uVar6 = FUN_00430f50((float)(int)local_16c * _DAT_005a34a0 + (float)local_14[10],
                                     local_14[0xc],
                                     (float)(int)local_16c * _DAT_005a34a0 + (float)local_14[0xb]);
                FUN_004310a0(uVar6);
                FUN_0054ac09(local_274);
              }
              pvVar3 = operator_new(0x180);
              local_8 = 6;
              if (pvVar3 == (void *)0x0) {
                local_278 = 0;
              }
              else {
                local_278 = FUN_004c8f70(0xc1,4000,0x3f800000,0x3f800000,0x3f800000,0x3a83126f,1,
                                         0x50);
              }
              local_8 = 0xffffffff;
              if (local_278 != 0) {
                uVar5 = local_16c & 0x80000001;
                if ((int)uVar5 < 0) {
                  uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
                }
                *(uint *)(local_278 + 0x100) = uVar5;
                uVar6 = FUN_00430f50((float)local_14[10] - (float)(int)local_16c * _DAT_005a34a0,
                                     local_14[0xc],
                                     (float)local_14[0xb] - (float)(int)local_16c * _DAT_005a34a0);
                FUN_004310a0(uVar6);
                FUN_0054ac09(local_278);
              }
            }
          }
        }
        else {
          pvVar3 = operator_new(0x90);
          local_8 = 7;
          if (pvVar3 == (void *)0x0) {
            local_27c = 0;
          }
          else {
            local_27c = FUN_004d19a0(local_140,local_13c,local_138,0);
          }
          local_8 = 0xffffffff;
          if (local_27c != 0) {
            FUN_0054ac09(local_27c);
          }
        }
      }
      if ((local_14[0x17] == 0) || (local_14[0x17] == 1)) {
        if ((*(ushort *)(local_9c + 0x80) & 0xf0) == 0x10) {
          FUN_0051dba4(0x1a,1);
        }
        else if ((*(ushort *)(local_9c + 0x80) & 0xf0) == 0x20) {
          FUN_0051dba4(0x18,1);
        }
      }
    }
    if (local_14 == *(int **)(param_1 + 0x4c)) {
      uVar6 = __ftol();
      *(undefined4 *)(param_1 + 0x275b0) = uVar6;
      uVar6 = __ftol();
      *(undefined4 *)(param_1 + 0x275b4) = uVar6;
    }
    local_a0 = *(int *)(param_1 + 0x2872c);
    if (*(int *)(param_1 + 0x2872c) != 0) {
      for (local_17c = 0; local_17c < *(int *)(local_a0 + 0x210); local_17c = local_17c + 1) {
        iVar2 = *(int *)(local_a0 + 0x214 + local_17c * 4);
        if (*(uint *)(iVar2 + 0xe50) == (uint)*(ushort *)(local_9c + 0x10)) {
          iVar4 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined4 *)(iVar2 + 0xe50));
          if ((iVar4 != 0) && (*(int *)(iVar2 + 0xe5c) != 1)) {
            *(undefined1 *)(iVar4 + 0x248) = 1;
            FUN_004f9946(0xaaffff00);
          }
          if (*(int *)(iVar2 + 0xe5c) == 4) {
            *(undefined4 *)(iVar2 + 0xe5c) = 2;
            *(undefined4 *)(iVar2 + 0x754) = 0xffaaaaff;
            FUN_00421b8f(iVar2 + 0x76a,*(undefined4 *)(iVar2 + 0x754),0);
            ExceptionList = local_10;
            return 1;
          }
          if (*(int *)(iVar2 + 0xe5c) != 3) {
            ExceptionList = local_10;
            return 1;
          }
          *(undefined4 *)(iVar2 + 0xe5c) = 0;
          *(undefined4 *)(iVar2 + 0x754) = 0xffffffff;
          FUN_00421b8f(iVar2 + 0x76a,*(undefined4 *)(iVar2 + 0x754),0);
          ExceptionList = local_10;
          return 1;
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

