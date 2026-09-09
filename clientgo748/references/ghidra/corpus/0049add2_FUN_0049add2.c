// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049add2 | Name: FUN_0049add2


undefined4 FUN_0049add2(int param_1)

{
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  undefined4 uVar4;
  int extraout_ECX;
  uint uVar5;
  undefined4 local_418c;
  undefined4 *local_417c;
  int local_4144;
  int local_4140;
  int local_413c;
  int local_4138;
  int local_4134;
  int local_4130;
  int local_412c;
  int local_4128;
  int local_4124;
  undefined1 local_4120 [16384];
  undefined1 local_120 [128];
  undefined4 *local_a0;
  undefined1 local_9c [112];
  undefined4 uStackY_2c;
  undefined4 uStackY_28;
  undefined4 uStackY_24;
  undefined1 *puVar6;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a01ac;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0058f730();
  if (*(int *)(extraout_ECX + 0x40) == 0) {
    ExceptionList = local_10;
    return 0;
  }
  _memset(local_120,0,0x80);
  _memset(local_9c,0,0x80);
  if (param_1 == 0) {
    if ((*(int *)(*(int *)(extraout_ECX + 0x40) + 0xac) != 0) ||
       (*(char *)(*(int *)(extraout_ECX + 0x40) + 0x20a30) != '\x01')) {
      ExceptionList = local_10;
      return 0;
    }
    uStackY_24 = 0x49aeb8;
    FUN_0058f078();
    uStackY_24 = 0x49aed4;
    FUN_0058f078();
  }
  else if (param_1 == 1) {
    if ((*(int *)(*(int *)(extraout_ECX + 0x40) + 0xb0) != 0) ||
       (*(char *)(*(int *)(extraout_ECX + 0x40) + 0x20a31) != '\x01')) {
      ExceptionList = local_10;
      return 0;
    }
    uStackY_24 = 0x49af56;
    FUN_0058f078();
    uStackY_24 = 0x49af72;
    FUN_0058f078();
  }
  else if (param_1 == 2) {
    if ((*(int *)(*(int *)(extraout_ECX + 0x40) + 0xb4) != 0) ||
       (*(char *)(*(int *)(extraout_ECX + 0x40) + 0x20a32) != '\x01')) {
      ExceptionList = local_10;
      return 0;
    }
    uStackY_24 = 0x49aff4;
    FUN_0058f078();
    uStackY_24 = 0x49b010;
    FUN_0058f078();
  }
  else {
    if (param_1 != 3) {
      ExceptionList = local_10;
      return 0;
    }
    if ((*(int *)(*(int *)(extraout_ECX + 0x40) + 0xb8) != 0) ||
       (*(char *)(*(int *)(extraout_ECX + 0x40) + 0x20a33) != '\x01')) {
      ExceptionList = local_10;
      return 0;
    }
    uStackY_24 = 0x49b092;
    FUN_0058f078();
    uStackY_24 = 0x49b0ae;
    FUN_0058f078();
  }
  _memset(local_4120,0,0x4000);
  uVar5 = *(int *)(extraout_ECX + 0x78) + 1U & 0x80000001;
  if ((int)uVar5 < 0) {
    uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
  }
  if (*(int *)(extraout_ECX + 0x68 + uVar5 * 4) == 0) {
    for (local_4124 = 0; local_4124 < 0x80; local_4124 = local_4124 + 1) {
      FUN_0058f220();
    }
  }
  else if ((*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a20) ==
            *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a20) + 1) &&
          (*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a24) ==
           *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a24))) {
    for (local_4128 = 0; local_4128 < 0x80; local_4128 = local_4128 + 1) {
      FUN_0058f220();
    }
  }
  else if ((*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a20) ==
            *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a20)) &&
          (*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a24) ==
           *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a24) + 1)) {
    for (local_412c = 0; local_412c < 0x80; local_412c = local_412c + 1) {
      FUN_0058f220();
    }
  }
  else if ((*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a20) ==
            *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a20) + -1) &&
          (*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a24) ==
           *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a24))) {
    for (local_4130 = 0; local_4130 < 0x80; local_4130 = local_4130 + 1) {
      FUN_0058f220();
    }
  }
  else if ((*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a20) ==
            *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a20)) &&
          (*(int *)(*(int *)(extraout_ECX + 0x68 + uVar5 * 4) + 0x20a24) ==
           *(int *)(*(int *)(extraout_ECX + 0x40) + 0x20a24) + -1)) {
    for (local_4134 = 0; local_4134 < 0x80; local_4134 = local_4134 + 1) {
      FUN_0058f220();
    }
  }
  pvVar2 = operator_new(0x20a70);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_417c = (undefined4 *)0x0;
  }
  else {
    local_417c = (undefined4 *)FUN_00532390();
  }
  local_8 = 0xffffffff;
  local_a0 = local_417c;
  puVar6 = local_120;
  iVar3 = FUN_00533dd7();
  if (iVar3 == 0) {
    if ((local_a0 != (undefined4 *)0x0) && (local_a0 != (undefined4 *)0x0)) {
      (**(code **)*local_a0)();
    }
    uVar4 = 0;
  }
  else {
    if ((*(int *)(extraout_ECX + 0x68 + (int)puVar6 * 4) != 0) &&
       (*(int *)(extraout_ECX + 0x68 + (int)puVar6 * 4) != 0)) {
      puVar1 = *(undefined4 **)(extraout_ECX + 0x68 + (int)puVar6 * 4);
      if (puVar1 != (undefined4 *)0x0) {
        puVar6 = (undefined1 *)0x1;
        (**(code **)*puVar1)();
      }
      *(undefined4 *)(extraout_ECX + 0x68 + (int)puVar6 * 4) = 0;
    }
    if ((*(int *)(extraout_ECX + 0x70 + (int)puVar6 * 4) != 0) &&
       (*(int *)(extraout_ECX + 0x70 + (int)puVar6 * 4) != 0)) {
      puVar1 = *(undefined4 **)(extraout_ECX + 0x70 + (int)puVar6 * 4);
      if (puVar1 != (undefined4 *)0x0) {
        puVar6 = (undefined1 *)0x1;
        (**(code **)*puVar1)();
      }
      *(undefined4 *)(extraout_ECX + 0x70 + (int)puVar6 * 4) = 0;
    }
    *(undefined4 **)(extraout_ECX + 0x68 + (int)puVar6 * 4) = local_a0;
    FUN_0054ac09();
    if (*(int *)(extraout_ECX + 0x68 + *(int *)(extraout_ECX + 0x78) * 4) != 0) {
      FUN_00534a75();
    }
    iVar3 = 0x404c;
    pvVar2 = operator_new(0x404c);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_418c = 0;
    }
    else {
      iVar3 = *(int *)(extraout_ECX + 0x68 + iVar3 * 4);
      local_418c = FUN_00540ea3();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(extraout_ECX + 0x70 + iVar3 * 4) = local_418c;
    puVar6 = local_9c;
    iVar3 = FUN_00541065();
    if (iVar3 == 0) {
      if (*(int *)(extraout_ECX + 0x70 + (int)puVar6 * 4) != 0) {
        puVar1 = *(undefined4 **)(extraout_ECX + 0x70 + (int)puVar6 * 4);
        if (puVar1 != (undefined4 *)0x0) {
          (**(code **)*puVar1)();
        }
        *(undefined4 *)(extraout_ECX + 0x70 + (int)puVar6 * 4) = 0;
      }
      FUN_00431159();
      if (*(int *)(extraout_ECX + 0x54) == 0) {
        uStackY_24 = 0;
        uStackY_28 = 0xc;
        uStackY_2c = 0x49b7a0;
        FUN_0049ed58();
      }
      *(undefined4 *)(extraout_ECX + 0x54) = 1;
      uVar4 = 0;
    }
    else {
      if (param_1 == 3) {
        if (*(int *)(extraout_ECX + 0x70 + *(int *)(extraout_ECX + 0x78) * 4) != 0) {
          FUN_0054ace4();
        }
      }
      else if (*(int *)(extraout_ECX + 0x70 + *(int *)(extraout_ECX + 0x78) * 4) != 0) {
        FUN_0054ad41();
      }
      FUN_004b7f0d();
      uStackY_24 = 0x49b854;
      _memset((void *)(extraout_ECX + 0x94),0,4);
      if (param_1 == 0) {
        for (local_4138 = 0; local_4138 < 0x80; local_4138 = local_4138 + 1) {
          uStackY_24 = 0x49b8c9;
          FUN_0058f220();
          uStackY_24 = 0x49b8fe;
          FUN_0058f220();
        }
        DAT_005b8924 = __ftol();
        DAT_005b8928 = __ftol();
      }
      else if (param_1 == 1) {
        for (local_413c = 0; local_413c < 0x80; local_413c = local_413c + 1) {
          uStackY_24 = 0x49b9bb;
          FUN_0058f220();
          uStackY_24 = 0x49b9f0;
          FUN_0058f220();
        }
        DAT_005b8924 = __ftol();
        DAT_005b8928 = __ftol();
      }
      else if (param_1 == 2) {
        for (local_4140 = 0; local_4140 < 0x80; local_4140 = local_4140 + 1) {
          uStackY_24 = 0x49baa5;
          FUN_0058f220();
          uStackY_24 = 0x49bae0;
          FUN_0058f220();
        }
        DAT_005b8924 = __ftol();
        DAT_005b8928 = __ftol();
      }
      else if (param_1 == 3) {
        for (local_4144 = 0; local_4144 < 0x80; local_4144 = local_4144 + 1) {
          uStackY_24 = 0x49bba7;
          FUN_0058f220();
          uStackY_24 = 0x49bbdc;
          FUN_0058f220();
        }
        DAT_005b8924 = __ftol();
        DAT_005b8928 = __ftol();
      }
      FUN_0054f18b();
      uStackY_24 = 0x49bc53;
      FUN_0058f220();
      FUN_00499e99();
      FUN_00534968();
      *(undefined4 *)(extraout_ECX + 0x200ac) = 0;
      uVar4 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(extraout_ECX + 0x200a8) = uVar4;
      uVar4 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar4;
}

