// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004640e5 | Name: FUN_004640e5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004640e5(int param_1)

{
  void **ppvVar1;
  byte bVar2;
  int iVar3;
  undefined4 uVar4;
  void *pvVar5;
  undefined4 uVar6;
  int local_3b0;
  int local_3ac;
  undefined1 local_394 [4];
  undefined2 local_390;
  undefined2 local_38e;
  undefined4 local_388;
  undefined4 local_384;
  undefined1 local_380 [4];
  undefined2 local_37c;
  undefined2 local_37a;
  undefined2 local_374;
  undefined2 local_372;
  short local_370;
  short local_36c;
  undefined2 local_368;
  int local_364;
  undefined1 local_360 [4];
  undefined2 local_35c;
  undefined2 local_35a;
  uint local_354;
  uint local_350;
  uint local_34c;
  int local_348;
  undefined4 local_344;
  undefined1 local_340 [4];
  undefined2 local_33c;
  undefined2 local_33a;
  undefined2 local_332;
  undefined2 local_330;
  undefined1 local_32c [4];
  undefined2 local_328;
  undefined2 local_326;
  undefined2 local_320;
  undefined2 local_31c;
  undefined2 local_31a;
  undefined2 local_318;
  undefined2 local_316;
  undefined2 local_314;
  undefined2 local_312;
  undefined1 local_310;
  undefined1 local_30f;
  int local_2cc;
  int local_2c8;
  undefined4 local_2c4;
  undefined1 local_2c0 [4];
  undefined2 local_2bc;
  undefined2 local_2ba;
  undefined4 local_2b4;
  undefined4 local_2b0;
  undefined1 local_2ac [4];
  undefined2 local_2a8;
  undefined2 local_2a6;
  undefined4 local_2a0;
  undefined4 local_29c;
  int local_298;
  undefined4 local_294;
  uint local_290;
  uint local_28c;
  int local_288;
  int *local_284;
  undefined1 local_280 [4];
  undefined2 local_27c;
  undefined2 local_27a;
  undefined4 local_274;
  undefined4 local_270;
  undefined1 local_26c [4];
  undefined2 local_268;
  undefined2 local_266;
  undefined4 local_260;
  undefined4 local_25c;
  undefined4 local_258;
  byte local_254;
  undefined3 uStack_253;
  undefined1 local_250 [4];
  undefined2 local_24c;
  undefined2 local_24a;
  uint local_244;
  undefined4 local_240;
  undefined1 local_23c [4];
  undefined2 local_238;
  undefined2 local_236;
  uint local_230;
  undefined4 local_22c;
  undefined1 local_228 [4];
  undefined2 local_224;
  undefined2 local_222;
  undefined4 local_21c;
  int local_218;
  undefined4 local_214;
  undefined1 local_210 [4];
  undefined2 local_20c;
  undefined2 local_20a;
  undefined2 local_204;
  ushort local_202;
  short local_200;
  int *local_1f8;
  int local_1f4;
  int local_1f0;
  int local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  byte local_1e0;
  undefined1 local_1dc [4];
  undefined2 local_1d8;
  undefined2 local_1d6;
  undefined2 local_1d0;
  undefined2 local_1ce;
  undefined2 local_1cc;
  undefined1 local_1c8 [4];
  undefined2 local_1c4;
  undefined2 local_1c2;
  undefined4 local_1bc;
  int *local_1b8;
  undefined1 local_1b4 [128];
  int *local_134;
  int local_130;
  undefined1 local_12c [128];
  undefined1 local_ac [4];
  undefined2 local_a8;
  undefined2 local_a6;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f6ab;
  local_10 = ExceptionList;
  if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x259) {
    ExceptionList = &local_10;
    *(undefined2 *)((int)DAT_013b71e8 + 0xcfa) = *(undefined2 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
    FUN_0058f220(local_ac,DAT_013b71e8 + 0x318,0x9c);
    local_a6 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_a8 = 899;
    FUN_0055f2dd(local_ac,0x9c);
    ppvVar1 = ExceptionList;
    if ((*(int *)(param_1 + 0x285f0) != 0) &&
       (iVar3 = FUN_0040c0f0(), ppvVar1 = ExceptionList, iVar3 == 0)) {
      local_1b8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x26a);
      local_134 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x25a);
      local_130 = (**(code **)(*DAT_013b71e8 + 0x34))
                            (*(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8));
      if (local_130 == 0) {
        FUN_00403df2(&DAT_00a353f8,2000);
        (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
        ExceptionList = local_10;
        return 1;
      }
      FUN_0058f078(local_1b4,&DAT_005b7db0,*(int *)(param_1 + 0x4c) + 0x168);
      FUN_0058f078(local_12c,&DAT_005b7db8,local_130 + 0x168);
      (**(code **)(*local_1b8 + 0x80))(local_1b4,1);
      (**(code **)(*local_134 + 0x80))(local_12c,1);
      FUN_0044b890(1);
      ppvVar1 = ExceptionList;
    }
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0xb) {
    ExceptionList = &local_10;
    uVar4 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x27140) = uVar4;
    *(undefined1 *)(param_1 + 0x271d0) = 1;
    local_1c2 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_1c4 = 0x3ae;
    local_1bc = 2;
    FUN_0055f2dd(local_1c8,0x10);
    ppvVar1 = ExceptionList;
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 4) {
    ExceptionList = &local_10;
    _memset(local_1dc,0,0x14);
    local_1d6 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_1d8 = 0x277;
    local_1d0 = 2;
    local_1ce = (undefined2)((uint)*(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) >> 0x10);
    local_1cc = (undefined2)*(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
    FUN_0055f2dd(local_1dc,0x14);
    ppvVar1 = ExceptionList;
  }
  else if ((((*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1831) ||
            (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1832)) ||
           (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1833)) ||
          (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1834)) {
    local_1e0 = 0;
    local_1e4 = 0;
    local_1e8 = 0;
    if ((*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1831) ||
       (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1834)) {
      local_1e4 = 2;
      local_1e8 = 4;
      if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1831) {
        local_1e0 = 1;
      }
      else {
        local_1e0 = 4;
      }
    }
    else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1832) {
      local_1e4 = 2;
      local_1e8 = 3;
      local_1e0 = 2;
    }
    if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1833) {
      local_1e4 = 1;
      local_1e8 = 1;
      local_1e0 = 3;
    }
    local_1ec = DAT_0067cf38;
    local_1f8 = *(int **)(DAT_0067cf38 + 0x2873c);
    ExceptionList = &local_10;
    (**(code **)(*local_1f8 + 0x98))(&local_1f4,local_1e4,local_1e8);
    if ((local_1f4 < 0) || (local_1f0 < 0)) {
      local_214 = *(undefined4 *)(param_1 + 0x27ae8);
      pvVar5 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar5 == (void *)0x0) {
        local_3ac = 0;
      }
      else {
        local_3ac = FUN_00407203(&DAT_00a341f8,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0
                                );
      }
      local_8 = 0xffffffff;
      local_218 = local_3ac;
      if (local_3ac != 0) {
        FUN_00408d33(local_3ac);
      }
      ppvVar1 = ExceptionList;
      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x21), ppvVar1 = ExceptionList, iVar3 != 0))
      {
        uVar6 = 0;
        uVar4 = 0;
        FUN_00429a6d(0x21);
        FUN_0042ad2b(uVar4,uVar6);
        ppvVar1 = ExceptionList;
      }
    }
    else {
      _memset(local_210,0,0x18);
      local_20a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
      local_204 = *(undefined2 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
      local_20c = 0x2bd;
      local_202 = (ushort)local_1e0;
      local_200 = (short)local_1f0 * 9 + (short)local_1f4;
      FUN_0055f2dd(local_210,0x18);
      ppvVar1 = ExceptionList;
      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x150), ppvVar1 = ExceptionList, iVar3 != 0))
      {
        uVar6 = 0;
        uVar4 = 0;
        FUN_00429a6d(0x150);
        FUN_0042ad2b(uVar4,uVar6);
        ppvVar1 = ExceptionList;
      }
    }
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x330) {
    ExceptionList = &local_10;
    _memset(local_228,0,0x10);
    local_222 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_224 = 0x28c;
    local_21c = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
    FUN_0055f2dd(local_228,0x10);
    (**(code **)(**(int **)(param_1 + 0x27c74) + 0x60))(0);
    ppvVar1 = ExceptionList;
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x331) {
    ExceptionList = &local_10;
    _memset(local_23c,0,0x14);
    local_236 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_238 = 0xe0e;
    local_230 = (uint)*(ushort *)(*(int *)(param_1 + 0x4c) + 0x680);
    local_22c = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
    FUN_0055f2dd(local_23c,0x14);
    (**(code **)(**(int **)(param_1 + 0x27c74) + 0x60))(0);
    ppvVar1 = ExceptionList;
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x35e) {
    ExceptionList = &local_10;
    _memset(local_250,0,0x14);
    local_24a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_24c = 0xe12;
    local_244 = (uint)*(ushort *)(*(int *)(param_1 + 0x4c) + 0x680);
    local_240 = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
    FUN_0055f2dd(local_250,0x14);
    (**(code **)(**(int **)(param_1 + 0x27c74) + 0x60))(0);
    ppvVar1 = ExceptionList;
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x332) {
    ExceptionList = &local_10;
    (**(code **)(**(int **)(param_1 + 0x27c74) + 0x60))(0);
    ppvVar1 = ExceptionList;
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x10) {
    ExceptionList = &local_10;
    uVar4 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x27474) = uVar4;
    local_258 = __ftol();
    local_25c = __ftol();
    bVar2 = FUN_00433a60(local_258,local_25c);
    _local_254 = CONCAT31(uStack_253,bVar2);
    if ((bVar2 & 0x10) == 0) {
      FUN_00403df2(&DAT_00a35478,2000);
      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
      ppvVar1 = ExceptionList;
    }
    else {
      _memset(local_26c,0,0x10);
      local_266 = (undefined2)DAT_013b71e8[0x317];
      local_268 = 0x290;
      local_260 = 0;
      FUN_0055f2dd(local_26c,0x10);
      ppvVar1 = ExceptionList;
    }
  }
  else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x39f) {
    ExceptionList = &local_10;
    _memset(local_280,0,0x14);
    local_27c = 0x39f;
    local_27a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_274 = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
    local_270 = 4;
    FUN_0055f2dd(local_280,0x14);
    *(undefined4 *)(param_1 + 0x26e94) = 0;
    ppvVar1 = ExceptionList;
  }
  else {
    ppvVar1 = &local_10;
    if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) != 0x18d) {
      if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x373) {
        local_28c = *(uint *)(*(int *)(param_1 + 0x8c) + 0x1e8) >> 0x10;
        local_290 = *(uint *)(*(int *)(param_1 + 0x8c) + 0x1e8) & 0xffff;
        local_284 = *(int **)(param_1 + 0x2873c);
        local_288 = 0;
        ExceptionList = &local_10;
        local_288 = (**(code **)(*local_284 + 0xb4))(local_28c,local_290);
        ppvVar1 = ExceptionList;
        if (local_288 != 0) {
          local_294 = FUN_0054cd07(*(undefined4 *)(local_288 + 0x670),0x26);
          local_298 = (int)**(short **)(local_288 + 0x670);
          FUN_00465f85(local_288,local_294,local_298,local_28c,local_290);
          ppvVar1 = ExceptionList;
        }
      }
      else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0xd) {
        ExceptionList = &local_10;
        _memset(local_2ac,0,0x14);
        local_2a8 = 0x28b;
        local_2a6 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        local_2a0 = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
        local_29c = 0;
        FUN_0055f2dd(local_2ac,0x14);
        ppvVar1 = ExceptionList;
      }
      else if ((((*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0xf) ||
                (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 10)) ||
               (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x6ce)) ||
              ((*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0xe9 ||
               (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x3a)))) {
        ExceptionList = &local_10;
        _memset(local_2c0,0,0x14);
        local_2bc = 0x28b;
        local_2ba = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        local_2b4 = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
        local_2b0 = 1;
        FUN_0055f2dd(local_2c0,0x14);
        ppvVar1 = ExceptionList;
      }
      else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x286) {
        ExceptionList = &local_10;
        FUN_0044ef69(*(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8));
        ppvVar1 = ExceptionList;
      }
      else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x54) {
        ExceptionList = &local_10;
        local_2c8 = (**(code **)(*DAT_0092e654 + 8))();
        if ((uint)(local_2c8 - *(int *)(param_1 + 0x27460)) < 1000) {
          ExceptionList = local_10;
          return 1;
        }
        local_2cc = (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18 + 0x54;
        iVar3 = FUN_00555790(0x54,*(undefined1 *)((int)DAT_013b71e8 + 0x9d3),
                             *(undefined1 *)
                              ((int)DAT_013b71e8 +
                              ((int)(local_2cc + (local_2cc >> 0x1f & 7U)) >> 3) + 0x745));
        if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar3) {
          local_344 = *(undefined4 *)(param_1 + 0x27ae8);
          pvVar5 = operator_new(0xe50);
          local_8 = 1;
          if (pvVar5 == (void *)0x0) {
            local_3b0 = 0;
          }
          else {
            local_3b0 = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777
                                     ,1,0);
          }
          local_8 = 0xffffffff;
          local_348 = local_3b0;
          if (local_3b0 != 0) {
            FUN_00408d33(local_3b0);
          }
          if (DAT_005ccf98 == 0) {
            ExceptionList = local_10;
            return 1;
          }
          iVar3 = FUN_00429a6d(0x21);
          if (iVar3 == 0) {
            ExceptionList = local_10;
            return 1;
          }
          uVar6 = 0;
          uVar4 = 0;
          FUN_00429a6d(0x21);
          FUN_0042ad2b(uVar4,uVar6);
          ExceptionList = local_10;
          return 1;
        }
        local_2c4 = 0x30;
        _memset(local_32c,0,0x60);
        local_328 = 0x39d;
        local_326 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        local_320 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        local_31c = *(undefined2 *)(param_1 + 0x28794);
        local_31a = *(undefined2 *)(param_1 + 0x28798);
        local_312 = 0xffff;
        local_314 = 0x54;
        local_30f = 0;
        local_310 = 0xff;
        local_318 = local_31c;
        local_316 = local_31a;
        FUN_0055f2dd(local_32c,local_2c4);
        *(int *)(param_1 + 0x27460) = local_2c8;
        _DAT_0092ec48 = local_2c8;
        _memset(local_340,0,0x14);
        local_33a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        local_33c = 0x37a;
        local_332 = *(undefined2 *)(param_1 + 0x275e0);
        local_330 = *(undefined2 *)(param_1 + 0x275e2);
        FUN_0055f2dd(local_340,0x14);
        *(undefined2 *)(param_1 + 0x275e0) = 0xffff;
        *(undefined2 *)(param_1 + 0x275e2) = 0xffff;
        if (*(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2f8) != 0) {
          *(undefined1 *)(*(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2f8) + 0x28f4) = 1;
        }
        ppvVar1 = ExceptionList;
        if (*(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2fc) != 0) {
          *(undefined1 *)(*(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2fc) + 0x28f4) = 1;
          ppvVar1 = ExceptionList;
        }
      }
      else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x26) {
        local_34c = *(uint *)(*(int *)(param_1 + 0x8c) + 0x1e8) >> 0x10;
        local_350 = *(uint *)(*(int *)(param_1 + 0x8c) + 0x1e8) & 0xffff;
        ExceptionList = &local_10;
        FUN_0045061f(local_34c,local_350);
        ppvVar1 = ExceptionList;
      }
      else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x342) {
        ExceptionList = &local_10;
        FUN_004754e0();
        ppvVar1 = ExceptionList;
      }
      else {
        if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 99) {
          ExceptionList = &local_10;
          *(undefined1 *)(param_1 + 0x27458) = 1;
          uVar4 = (**(code **)(*DAT_0092e654 + 8))();
          *(undefined4 *)(param_1 + 0x27148) = uVar4;
          ExceptionList = local_10;
          return 1;
        }
        if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0xe4) {
          ExceptionList = &local_10;
          *(undefined1 *)(param_1 + 0x271d2) = 1;
          uVar4 = (**(code **)(*DAT_0092e654 + 8))();
          *(undefined4 *)(param_1 + 0x2714c) = uVar4;
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0xc351) {
          ExceptionList = &local_10;
          _memset(local_360,0,0x10);
          local_35c = 0x37e;
          local_35a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
          local_354 = (uint)*(ushort *)(*(int *)(param_1 + 0x8c) + 0x1e8);
          FUN_0055f2dd(local_360,0x10);
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x55f) {
          ExceptionList = &local_10;
          FUN_00463752();
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x17df) {
          ExceptionList = &local_10;
          FUN_00463832();
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1802) {
          ExceptionList = &local_10;
          FUN_00463899();
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1922) {
          ExceptionList = &local_10;
          FUN_004639e6();
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1952) {
          ExceptionList = &local_10;
          FUN_00463aa8();
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x1972) {
          ExceptionList = &local_10;
          FUN_00464004();
          ppvVar1 = ExceptionList;
        }
        else if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x37a) {
          if (*(int *)(param_1 + 0x2873c) == 0) {
            return 1;
          }
          if (*(int *)(param_1 + 0x28730) == 0) {
            return 1;
          }
          local_364 = DAT_005ccf10;
          ExceptionList = &local_10;
          if (DAT_005ccf10 != 0) {
            ExceptionList = &local_10;
            local_368 = (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc0))
                                  (*(undefined4 *)(*(int *)(DAT_005ccf10 + 0x1e4) + 0x1e4),
                                   *(undefined4 *)(*(int *)(DAT_005ccf10 + 0x1e4) + 0x400));
            local_36c = (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc4))
                                  (*(undefined4 *)(*(int *)(local_364 + 0x1e4) + 0x1e4));
            if (local_36c == -1) {
              local_36c = (short)*(undefined4 *)(local_364 + 0x1d0) +
                          (short)*(undefined4 *)(local_364 + 0x1d4) * 9;
            }
            _memset(local_380,0,0x14);
            local_37a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
            local_37c = 0x37a;
            local_374 = *(undefined2 *)(*(int *)(param_1 + 0x28730) + 0x404);
            local_372 = local_368;
            local_370 = local_36c;
            FUN_0055f2dd(local_380,0x14);
          }
          DAT_005ccf10 = 0;
          ppvVar1 = ExceptionList;
        }
        else {
          ppvVar1 = &local_10;
          if (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x10f) {
            ExceptionList = &local_10;
            _memset(local_394,0,0x14);
            local_390 = 0x28b;
            local_38e = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
            local_388 = *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8);
            local_384 = 0;
            FUN_0055f2dd(local_394,0x14);
            ppvVar1 = ExceptionList;
          }
        }
      }
    }
  }
  ExceptionList = ppvVar1;
  if (DAT_013b722c == 1) {
    (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(*(undefined4 *)(param_1 + 0x27b2c));
  }
  (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(0);
  ExceptionList = local_10;
  return 1;
}

