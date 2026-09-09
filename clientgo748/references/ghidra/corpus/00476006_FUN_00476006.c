// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00476006 | Name: FUN_00476006


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00476006(int *param_1,uint param_2)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int local_49c;
  int local_498;
  int local_494;
  int local_490;
  undefined1 local_438 [128];
  uint local_3b8;
  int local_3b4;
  uint local_3b0;
  undefined1 local_3ac [4];
  undefined2 local_3a8;
  undefined2 local_3a6;
  undefined1 local_3a0 [16];
  undefined1 local_390 [100];
  undefined1 local_32c [4];
  undefined2 local_328;
  undefined2 local_326;
  undefined1 local_320 [16];
  undefined1 local_310 [100];
  int local_2ac;
  uint local_2a8;
  undefined1 local_2a4 [4];
  undefined2 local_2a0;
  undefined2 local_29e;
  undefined1 local_298 [16];
  undefined1 local_288 [100];
  uint local_224;
  undefined1 local_220 [4];
  undefined4 local_21c;
  undefined2 local_214;
  undefined2 local_210;
  undefined2 local_20e;
  undefined2 local_20c;
  undefined2 local_20a;
  undefined2 local_208;
  undefined2 local_206;
  undefined1 local_204;
  undefined1 local_203;
  undefined1 local_202;
  undefined2 local_1f4;
  undefined2 local_1f2;
  int local_1c0;
  uint local_1bc;
  int local_1b8;
  undefined1 local_1b4 [4];
  undefined2 local_1b0;
  undefined2 local_1ae;
  undefined1 local_1a8 [128];
  uint local_128;
  undefined1 local_124 [128];
  uint local_a4;
  undefined1 local_a0 [4];
  undefined2 local_9c;
  undefined2 local_9a;
  undefined1 local_94 [128];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f7f9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (DAT_013b7220 != 0) {
    if (DAT_013b7220 + 5000U < param_2) {
      ExceptionList = &local_10;
      PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
      ExceptionList = local_10;
      return 1;
    }
    local_14 = ((DAT_013b7220 + 5000) - param_2) / 1000;
    ExceptionList = &local_10;
    if (param_1[0x9c4d] != local_14) {
      ExceptionList = &local_10;
      param_1[0x8028] = 0;
      param_1[0x9c4d] = local_14;
      FUN_0058f078(local_94,&DAT_00a3b178,param_1[0x9c4d] + 1);
      FUN_00403df2(local_94,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      if (DAT_013b736c == 1) {
        FUN_0055ee59(0);
      }
    }
  }
  if (param_1[0x9c4e] != 0) {
    if (param_1[0x9c4e] + 5000U < param_2) {
      _memset(local_a0,0,0xc);
      local_9a = (undefined2)DAT_013b71e8[0x317];
      local_9c = 0x215;
      FUN_0055f2dd(local_a0,0xc);
      param_1[0x9c4e] = 0;
      ExceptionList = local_10;
      return 1;
    }
    local_a4 = ((param_1[0x9c4e] + 5000) - param_2) / 1000;
    if (param_1[0x9c4d] != local_a4) {
      param_1[0x8028] = 0;
      param_1[0x9c4d] = local_a4;
      FUN_0058f078(local_124,&DAT_00a3b1f8,param_1[0x9c4d] + 1);
      FUN_00403df2(local_124,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      if (DAT_013b736c == 1) {
        FUN_0055ee59(0);
      }
    }
  }
  if (param_1[0x9c4f] != 0) {
    if (param_1[0x9c4f] + 5000U < param_2) {
      param_1[0x9c4f] = 0;
      (**(code **)(*DAT_013b71e8 + 0x54))(7);
      ExceptionList = local_10;
      return 1;
    }
    local_128 = ((param_1[0x9c4f] + 5000) - param_2) / 1000;
    if (param_1[0x9c4d] != local_128) {
      param_1[0x8028] = 0;
      param_1[0x9c4d] = local_128;
      FUN_0058f078(local_1a8,&DAT_00a3b178,param_1[0x9c4d] + 1);
      FUN_00403df2(local_1a8,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      if (DAT_013b736c == 1) {
        FUN_0055ee59(0);
      }
    }
  }
  if (param_1[0x9c50] != 0) {
    if ((param_1[0x9c50] + 6000U < param_2) && ((char)param_1[0x9c74] == '\0')) {
      param_1[0x9c50] = 0;
    }
    else if ((param_1[0x9c50] + 5000U < param_2) && ((char)param_1[0x9c74] == '\x01')) {
      local_1ae = *(undefined2 *)(param_1[0x13] + 0x20);
      local_1b0 = 0x289;
      FUN_0055f2dd(local_1b4,0xc);
      *(undefined1 *)(param_1 + 0x9d16) = 0;
      *(undefined1 *)(param_1[0x13] + 0x245) = 0;
      FUN_0051dba4(0xe,0);
      if (*(char *)(param_1[0x13] + 0x795) == '\0') {
        pvVar1 = operator_new(0x90);
        local_8 = 0;
        if (pvVar1 == (void *)0x0) {
          local_490 = 0;
        }
        else {
          uVar3 = 0;
          puVar2 = (undefined4 *)
                   FUN_00430f50(*(undefined4 *)(param_1[0x13] + 0x28),
                                *(undefined4 *)(param_1[0x13] + 0x30),
                                *(undefined4 *)(param_1[0x13] + 0x2c));
          local_490 = FUN_004d19a0(*puVar2,puVar2[1],puVar2[2],uVar3);
        }
        local_8 = 0xffffffff;
        local_1b8 = local_490;
        if (local_490 != 0) {
          FUN_0054ac09(local_490);
        }
      }
      *(undefined1 *)(param_1 + 0x9c74) = 0;
    }
    else {
      local_1bc = ((param_1[0x9c50] + 5000) - param_2) / 1000;
      if (param_1[0x9c4d] != local_1bc) {
        param_1[0x8028] = 0;
        param_1[0x9c4d] = local_1bc;
        pvVar1 = operator_new(0xac);
        local_8 = 1;
        if (pvVar1 == (void *)0x0) {
          local_494 = 0;
        }
        else {
          uVar3 = 1;
          puVar2 = (undefined4 *)
                   FUN_00430f50(*(undefined4 *)(param_1[0x13] + 0x28),
                                *(float *)(param_1[0x13] + 0x30) + _DAT_005a439c,
                                *(undefined4 *)(param_1[0x13] + 0x2c));
          local_494 = FUN_004ee730(*puVar2,puVar2[1],puVar2[2],uVar3);
        }
        local_8 = 0xffffffff;
        local_1c0 = local_494;
        if (local_494 != 0) {
          FUN_0054ac09(local_494);
        }
      }
    }
  }
  if (param_1[0x9c52] != 0) {
    _memset(local_220,0,0x60);
    local_21c = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x39d);
    local_214 = *(undefined2 *)(param_1[0x13] + 0x20);
    local_1f4 = *(undefined2 *)(param_1[0x13] + 0x20);
    local_1f2 = 0xffff;
    local_210 = __ftol();
    local_20e = __ftol();
    local_20c = __ftol();
    local_20a = __ftol();
    if (param_1[0xa1e5] != 0) {
      local_210 = (undefined2)param_1[0xa1e5];
      local_20e = (undefined2)param_1[0xa1e6];
      local_20c = local_210;
      local_20a = local_20e;
    }
    local_206 = 0xffff;
    local_208 = 99;
    local_203 = 0;
    local_204 = 0xff;
    if ((param_1[0x9c52] + 6000U < param_2) && ((char)param_1[0x9d16] == '\0')) {
      param_1[0x9c52] = 0;
    }
    else if ((param_1[0x9c52] + 5000U < param_2) && ((char)param_1[0x9d16] == '\x01')) {
      *(undefined1 *)(param_1 + 0x9d16) = 0;
      local_202 = 0;
      FUN_0055f2dd(local_220,0x30);
    }
    else {
      local_224 = ((param_1[0x9c52] + 5000) - param_2) / 2000;
      if (param_1[0x9c4d] != local_224) {
        param_1[0x8028] = 0;
        param_1[0x9c4d] = local_224;
        local_202 = 1;
        (**(code **)(*param_1 + 4))(local_21c & 0xffff,local_220);
      }
    }
  }
  if (param_1[0x9c51] != 0) {
    if ((param_1[0x9c51] + 6000U < param_2) && (*(char *)((int)param_1 + 0x271d1) == '\0')) {
      param_1[0x9c51] = 0;
    }
    else if ((param_1[0x9c51] + 5000U < param_2) && (*(char *)((int)param_1 + 0x271d1) == '\x01')) {
      if (param_1[0xa303] == 0) {
        FUN_0055f2dd(param_1 + 0x9c55,0x24);
        _memset(param_1 + 0x9c55,0,0x24);
        param_1[0x9d1e] = param_2;
      }
      else {
        param_1[0x9b85] = param_2;
        _memset(local_2a4,0,0x80);
        local_29e = (undefined2)DAT_013b71e8[0x317];
        local_2a0 = 0x334;
        FUN_0058f078(local_298,&PTR_DAT_005b8688);
        FUN_0058f078(local_288,&DAT_005b868c,param_1[0xa303]);
        FUN_0055f2dd(local_2a4,0x80);
        param_1[0xa303] = 0;
      }
      *(undefined1 *)((int)param_1 + 0x271d1) = 0;
    }
    else {
      local_2a8 = ((param_1[0x9c51] + 5000) - param_2) / 1000;
      if (param_1[0x9c4d] != local_2a8) {
        param_1[0x8028] = 0;
        param_1[0x9c4d] = local_2a8;
        pvVar1 = operator_new(0xac);
        local_8 = 2;
        if (pvVar1 == (void *)0x0) {
          local_498 = 0;
        }
        else {
          uVar3 = 1;
          puVar2 = (undefined4 *)
                   FUN_00430f50(*(undefined4 *)(param_1[0x13] + 0x28),
                                *(float *)(param_1[0x13] + 0x30) + _DAT_005a439c,
                                *(undefined4 *)(param_1[0x13] + 0x2c));
          local_498 = FUN_004ee730(*puVar2,puVar2[1],puVar2[2],uVar3);
        }
        local_8 = 0xffffffff;
        local_2ac = local_498;
        if (local_498 != 0) {
          FUN_0054ac09(local_498);
        }
      }
    }
  }
  if (param_1[0x9c53] != 0) {
    if ((param_1[0x9c53] + 6000U < param_2) && (*(char *)((int)param_1 + 0x271d2) == '\0')) {
      param_1[0x9c53] = 0;
    }
    else if ((param_1[0x9c53] + 5000U < param_2) && (*(char *)((int)param_1 + 0x271d2) == '\x01')) {
      _memset(local_32c,0,0x80);
      local_326 = (undefined2)DAT_013b71e8[0x317];
      local_328 = 0x334;
      FUN_0058f078(local_320,&DAT_005b8690);
      FUN_0058f078(local_310,&DAT_005b8698,param_1 + 0x9c95);
      FUN_0055f2dd(local_32c,0x80);
      *(undefined1 *)((int)param_1 + 0x271d2) = 0;
    }
    else if ((param_1[0x9c53] + 5000U < param_2) && (*(char *)((int)param_1 + 0x271d2) == '\x02')) {
      _memset(local_3ac,0,0x80);
      local_3a6 = (undefined2)DAT_013b71e8[0x317];
      local_3a8 = 0x334;
      FUN_0058f078(local_3a0,&DAT_005b869c);
      FUN_0058f078(local_390,&DAT_005b86a4,param_1 + 0x9cd5);
      FUN_0055f2dd(local_3ac,0x80);
      *(undefined1 *)((int)param_1 + 0x271d2) = 0;
    }
    else {
      local_3b0 = ((param_1[0x9c53] + 5000) - param_2) / 1000;
      if (param_1[0x9c4d] != local_3b0) {
        param_1[0x8028] = 0;
        param_1[0x9c4d] = local_3b0;
        pvVar1 = operator_new(0xac);
        local_8 = 3;
        if (pvVar1 == (void *)0x0) {
          local_49c = 0;
        }
        else {
          uVar3 = 1;
          puVar2 = (undefined4 *)
                   FUN_00430f50(*(undefined4 *)(param_1[0x13] + 0x28),
                                *(float *)(param_1[0x13] + 0x30) + _DAT_005a439c,
                                *(undefined4 *)(param_1[0x13] + 0x2c));
          local_49c = FUN_004ee730(*puVar2,puVar2[1],puVar2[2],uVar3);
        }
        local_8 = 0xffffffff;
        local_3b4 = local_49c;
        if (local_49c != 0) {
          FUN_0054ac09(local_49c);
        }
      }
    }
  }
  if (param_1[0x9c54] != 0) {
    if ((param_1[0x9c54] + 6000U < param_2) && (*(char *)((int)param_1 + 0x271d3) == '\0')) {
      param_1[0x9c54] = 0;
    }
    else {
      if ((param_1[0x9c54] + 5000U < param_2) && (*(char *)((int)param_1 + 0x271d3) == '\x01')) {
        FUN_0055f2dd(param_1 + 0x9c75,0x80);
        _memset(param_1 + 0x9c75,0,0x80);
        param_1[0x9c54] = 0;
        ExceptionList = local_10;
        return 1;
      }
      local_3b8 = ((param_1[0x9c54] + 5000) - param_2) / 1000;
      if (param_1[0x9c4d] != local_3b8) {
        param_1[0x8028] = 0;
        param_1[0x9c4d] = local_3b8;
        FUN_0058f078(local_438,&DAT_00a3b0f8,param_1[0x9c4d] + 1);
        FUN_00403df2(local_438,2000);
        (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      }
    }
  }
  ExceptionList = local_10;
  return 0;
}

