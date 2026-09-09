// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004faf92 | Name: FUN_004faf92


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004faf92(int param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined2 extraout_var;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uStack_3ec;
  undefined4 uStack_3e8;
  undefined4 uStack_3e4;
  undefined4 uStack_3e0;
  undefined4 uStack_3dc;
  undefined4 uStack_3d8;
  undefined4 uStack_3d4;
  undefined4 uStack_3d0;
  undefined4 uStack_3cc;
  undefined4 uStack_3c8;
  undefined4 uStack_3c4;
  undefined4 uStack_3c0;
  undefined4 uStack_3bc;
  undefined4 uStack_3b8;
  undefined4 uStack_3b4;
  undefined4 uStack_3b0;
  undefined4 uStack_3ac;
  undefined4 uStack_3a8;
  undefined4 uStack_3a4;
  undefined4 uStack_3a0;
  undefined4 uStack_39c;
  undefined4 uStack_398;
  undefined4 uStack_394;
  undefined4 uStack_390;
  undefined4 uStack_38c;
  undefined4 uStack_388;
  undefined4 uStack_384;
  undefined4 uStack_380;
  undefined4 uStack_37c;
  undefined4 uStack_374;
  int iStack_254;
  int aiStack_24c [7];
  float afStack_230 [7];
  int iStack_214;
  float afStack_210 [12];
  int iStack_1e0;
  float afStack_1dc [5];
  int iStack_1c8;
  int iStack_1c4;
  float fStack_1c0;
  int iStack_1bc;
  undefined4 uStack_1b8;
  float fStack_1b4;
  int iStack_1b0;
  undefined4 uStack_1ac;
  int iStack_1a8;
  int iStack_1a4;
  int iStack_1a0;
  int iStack_19c;
  undefined4 uStack_198;
  undefined4 uStack_194;
  int iStack_190;
  int iStack_18c;
  int aiStack_188 [15];
  float afStack_14c [28];
  int iStack_dc;
  float afStack_d8 [32];
  undefined1 auStack_58 [2];
  undefined2 uStack_56;
  undefined1 auStack_38 [8];
  undefined1 uStack_30;
  float fStack_28;
  void *local_20;
  int iStack_1c;
  short local_18;
  undefined4 uStack_14;
  void *pvStack_10;
  undefined1 *puStack_c;
  undefined4 uStack_8;
  
  uStack_8 = 0xffffffff;
  puStack_c = &LAB_005a1963;
  pvStack_10 = ExceptionList;
  ExceptionList = &pvStack_10;
  FUN_004fff0b(0);
  if (*(int *)(param_1 + 0x60) != 0) {
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2f4) = 0;
    if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
       (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x84) = 0,
       *(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88) != 0)) {
      (**(code **)(*DAT_013b71e8 + 100))
                (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88));
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88) = 0;
    }
    if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) &&
       (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x84) = 0,
       *(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88) != 0)) {
      (**(code **)(*DAT_013b71e8 + 100))
                (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88));
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88) = 0;
    }
  }
  local_18 = 0;
  if ((*(short *)(param_1 + 0x252) < 0x1037) || (0x1068 < *(short *)(param_1 + 0x252))) {
    uVar3 = FUN_013c2680();
    return uVar3;
  }
  *(undefined2 *)(param_1 + 0x1da) = *(undefined2 *)(param_1 + 0x1d6);
  *(undefined2 *)(param_1 + 0x1de) = *(undefined2 *)(param_1 + 0x1da);
  *(undefined2 *)(param_1 + 0x1e2) = *(undefined2 *)(param_1 + 0x1de);
  *(undefined2 *)(param_1 + 0x1e6) = *(undefined2 *)(param_1 + 0x1e2);
  *(undefined2 *)(param_1 + 0x1dc) = *(undefined2 *)(param_1 + 0x1d8);
  *(undefined2 *)(param_1 + 0x1e0) = *(undefined2 *)(param_1 + 0x1dc);
  *(undefined2 *)(param_1 + 0x1e4) = *(undefined2 *)(param_1 + 0x1e0);
  *(undefined2 *)(param_1 + 0x1e8) = *(undefined2 *)(param_1 + 0x1e4);
  local_20 = (void *)(param_1 + 0x1f2);
  _memset(local_20,0,6);
  _memset((void *)((int)local_20 + 8),0,6);
  if (*(int *)(param_1 + 0x60) != 0) {
    if (*(int *)(param_1 + 0x60) != 0) {
      FUN_00493d20(1);
    }
    *(undefined4 *)(param_1 + 0x60) = 0;
  }
  if ((local_18 == 0) &&
     ((((((*(int *)(param_1 + 0x5c) == 0x14 || (*(int *)(param_1 + 0x5c) == 0x27)) ||
         (*(int *)(param_1 + 0x5c) == 0x15)) ||
        (((*(int *)(param_1 + 0x5c) == 0x16 || (*(int *)(param_1 + 0x5c) == 0x17)) ||
         ((*(int *)(param_1 + 0x5c) == 0x18 ||
          ((*(int *)(param_1 + 0x5c) == 0x28 || (*(int *)(param_1 + 0x5c) == 3)))))))) ||
       (*(int *)(param_1 + 0x5c) == 4)) ||
      ((((*(int *)(param_1 + 0x5c) == 0x19 || (*(int *)(param_1 + 0x5c) == 0x1c)) ||
        (*(int *)(param_1 + 0x5c) == 0x1d)) ||
       ((((*(int *)(param_1 + 0x5c) == 2 || (*(int *)(param_1 + 0x5c) == 6)) ||
         ((*(int *)(param_1 + 0x5c) == 7 ||
          ((*(int *)(param_1 + 0x5c) == 8 || (*(int *)(param_1 + 0x5c) == 0x1e)))))) ||
        ((*(int *)(param_1 + 0x5c) == 0x1f ||
         (((((*(int *)(param_1 + 0x5c) == 0x21 || (*(int *)(param_1 + 0x5c) == 0x24)) ||
            (*(int *)(param_1 + 0x5c) == 0xc)) ||
           ((*(int *)(param_1 + 0x5c) == 0x2b || (*(int *)(param_1 + 0x5c) == 10)))) ||
          (*(int *)(param_1 + 0x5c) == 5)))))))))))) {
    *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(param_1 + 0x1d2);
    *(undefined2 *)(param_1 + 0x1d8) = *(undefined2 *)(param_1 + 0x1d4);
    *(undefined1 *)(param_1 + 499) = *(undefined1 *)(param_1 + 0x1f2);
    *(undefined1 *)(param_1 + 0x1fb) = *(undefined1 *)(param_1 + 0x1fa);
    *(undefined1 *)(param_1 + 0x203) = *(undefined1 *)(param_1 + 0x202);
  }
  if (*(int *)(param_1 + 0x24c) == 0x28) {
    *(undefined2 *)(param_1 + 0x1ee) = 0;
    *(undefined2 *)(param_1 + 0x1ea) = 0;
  }
  if ((*(int *)(param_1 + 0x5c) == 0x1a) || (*(int *)(param_1 + 0x5c) == 0x23)) {
    *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(param_1 + 0x1d2);
    *(undefined2 *)(param_1 + 0x1da) = *(undefined2 *)(param_1 + 0x1d6);
    *(undefined2 *)(param_1 + 0x1d8) = *(undefined2 *)(param_1 + 0x1d4);
    *(undefined2 *)(param_1 + 0x1dc) = *(undefined2 *)(param_1 + 0x1d8);
    *(undefined1 *)(param_1 + 499) = *(undefined1 *)(param_1 + 0x1f2);
    *(undefined1 *)(param_1 + 500) = *(undefined1 *)(param_1 + 499);
    *(undefined1 *)(param_1 + 0x1fb) = *(undefined1 *)(param_1 + 0x1fa);
    *(undefined1 *)(param_1 + 0x1fc) = *(undefined1 *)(param_1 + 0x1fb);
    *(undefined1 *)(param_1 + 0x203) = *(undefined1 *)(param_1 + 0x202);
    *(undefined1 *)(param_1 + 0x204) = *(undefined1 *)(param_1 + 0x203);
  }
  else if (((*(int *)(param_1 + 0x5c) == 0xb) || (*(int *)(param_1 + 0x5c) == 0x25)) ||
          (*(int *)(param_1 + 0x5c) == 0x2c)) {
    *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(param_1 + 0x1d2);
    *(undefined2 *)(param_1 + 0x1da) = *(undefined2 *)(param_1 + 0x1d6);
    *(undefined2 *)(param_1 + 0x1de) = *(undefined2 *)(param_1 + 0x1da);
    *(undefined2 *)(param_1 + 0x1e2) = *(undefined2 *)(param_1 + 0x1de);
    *(undefined2 *)(param_1 + 0x1d8) = *(undefined2 *)(param_1 + 0x1d4);
    *(undefined2 *)(param_1 + 0x1dc) = *(undefined2 *)(param_1 + 0x1d8);
    *(undefined2 *)(param_1 + 0x1e0) = *(undefined2 *)(param_1 + 0x1dc);
    *(undefined2 *)(param_1 + 0x1e4) = *(undefined2 *)(param_1 + 0x1e0);
    *(undefined1 *)(param_1 + 499) = *(undefined1 *)(param_1 + 0x1f2);
    *(undefined1 *)(param_1 + 500) = *(undefined1 *)(param_1 + 499);
    *(undefined1 *)(param_1 + 0x1f5) = *(undefined1 *)(param_1 + 500);
    *(undefined1 *)(param_1 + 0x1f6) = *(undefined1 *)(param_1 + 0x1f5);
    *(undefined1 *)(param_1 + 0x1fb) = *(undefined1 *)(param_1 + 0x1fa);
    *(undefined1 *)(param_1 + 0x1fc) = *(undefined1 *)(param_1 + 0x1fb);
    *(undefined1 *)(param_1 + 0x1fd) = *(undefined1 *)(param_1 + 0x1fc);
    *(undefined1 *)(param_1 + 0x1fe) = *(undefined1 *)(param_1 + 0x1fd);
    *(undefined1 *)(param_1 + 0x203) = *(undefined1 *)(param_1 + 0x202);
    *(undefined1 *)(param_1 + 0x204) = *(undefined1 *)(param_1 + 0x203);
    *(undefined1 *)(param_1 + 0x205) = *(undefined1 *)(param_1 + 0x204);
    *(undefined1 *)(param_1 + 0x206) = *(undefined1 *)(param_1 + 0x205);
  }
  if (((*(int *)(param_1 + 0x24c) == 0x27) || (*(int *)(param_1 + 0x24c) == 0x28)) ||
     (*(int *)(param_1 + 0x24c) == 0x3f)) {
    *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(param_1 + 0x1d2);
    *(undefined2 *)(param_1 + 0x1da) = *(undefined2 *)(param_1 + 0x1d6);
    *(undefined2 *)(param_1 + 0x1de) = *(undefined2 *)(param_1 + 0x1da);
    *(undefined2 *)(param_1 + 0x1e2) = *(undefined2 *)(param_1 + 0x1de);
    *(undefined2 *)(param_1 + 0x1e6) = *(undefined2 *)(param_1 + 0x1e2);
    *(undefined2 *)(param_1 + 0x1d8) = *(undefined2 *)(param_1 + 0x1d4);
    *(undefined2 *)(param_1 + 0x1dc) = *(undefined2 *)(param_1 + 0x1d8);
    *(undefined2 *)(param_1 + 0x1e0) = *(undefined2 *)(param_1 + 0x1dc);
    *(undefined2 *)(param_1 + 0x1e4) = *(undefined2 *)(param_1 + 0x1e0);
    *(undefined2 *)(param_1 + 0x1e8) = *(undefined2 *)(param_1 + 0x1e4);
    *(undefined1 *)(param_1 + 499) = *(undefined1 *)(param_1 + 0x1f2);
    *(undefined1 *)(param_1 + 500) = *(undefined1 *)(param_1 + 499);
    *(undefined1 *)(param_1 + 0x1f5) = *(undefined1 *)(param_1 + 500);
    *(undefined1 *)(param_1 + 0x1f6) = *(undefined1 *)(param_1 + 0x1f5);
    *(undefined1 *)(param_1 + 0x1f7) = *(undefined1 *)(param_1 + 0x1f6);
    *(undefined1 *)(param_1 + 0x1fb) = *(undefined1 *)(param_1 + 0x1fa);
    *(undefined1 *)(param_1 + 0x1fc) = *(undefined1 *)(param_1 + 0x1fb);
    *(undefined1 *)(param_1 + 0x1fd) = *(undefined1 *)(param_1 + 0x1fc);
    *(undefined1 *)(param_1 + 0x1fe) = *(undefined1 *)(param_1 + 0x1fd);
    *(undefined1 *)(param_1 + 0x1ff) = *(undefined1 *)(param_1 + 0x1fe);
    *(undefined1 *)(param_1 + 0x203) = *(undefined1 *)(param_1 + 0x202);
    *(undefined1 *)(param_1 + 0x204) = *(undefined1 *)(param_1 + 0x203);
    *(undefined1 *)(param_1 + 0x205) = *(undefined1 *)(param_1 + 0x204);
    *(undefined1 *)(param_1 + 0x206) = *(undefined1 *)(param_1 + 0x205);
    *(undefined1 *)(param_1 + 0x207) = *(undefined1 *)(param_1 + 0x206);
  }
  else if ((*(int *)(param_1 + 0x24c) == 0x3e) && (*(short *)(param_1 + 0x1d2) == 2)) {
    *(undefined1 *)(param_1 + 0x1f2) = 4;
    *(undefined1 *)(param_1 + 499) = 4;
  }
  uStack_14 = 0;
  if (((((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) ||
       (*(int *)(param_1 + 0x24c) == 0x24)) ||
      ((*(int *)(param_1 + 0x24c) == 0x27 || (*(int *)(param_1 + 0x24c) == 0x28)))) ||
     ((*(int *)(param_1 + 0x24c) == 0x3c || (*(int *)(param_1 + 0x24c) == 0x3f)))) {
    uStack_14 = 1;
  }
  if (*(short *)(param_1 + 0x1d2) == 0x28) {
    if (*(short *)(param_1 + 0x1da) == 0) {
      *(undefined2 *)(param_1 + 0x1da) = 0x28;
    }
    if (*(short *)(param_1 + 0x1de) == 0) {
      *(undefined2 *)(param_1 + 0x1de) = 0x28;
    }
    if (*(short *)(param_1 + 0x1e2) == 0) {
      *(undefined2 *)(param_1 + 0x1e2) = 0x28;
    }
    if (*(short *)(param_1 + 0x1e6) == 0) {
      *(undefined2 *)(param_1 + 0x1e6) = 0x28;
    }
    *(undefined2 *)(param_1 + 0x1d6) = 0x28;
  }
  else if ((*(short *)(param_1 + 0x1d2) == 0x50) || (*(short *)(param_1 + 0x1d2) == 0x40)) {
    if (*(short *)(param_1 + 0x1da) == 0) {
      *(undefined2 *)(param_1 + 0x1da) = 0x28;
    }
    if (*(short *)(param_1 + 0x1de) == 0) {
      *(undefined2 *)(param_1 + 0x1de) = 0x28;
    }
    if (*(short *)(param_1 + 0x1e2) == 0) {
      *(undefined2 *)(param_1 + 0x1e2) = 0x28;
    }
    if (*(short *)(param_1 + 0x1e6) == 0) {
      *(undefined2 *)(param_1 + 0x1e6) = 0x28;
    }
    *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(param_1 + 0x1d2);
  }
  if (((0x1036 < *(short *)(param_1 + 0x252)) && (*(short *)(param_1 + 0x252) < 0x1069)) &&
     (uStack_14 = 0, *(short *)(param_1 + 0x252) == 0x1037)) {
    uStack_14 = 1;
  }
  if (local_18 == 2) {
    uStack_14 = 1;
    *(undefined4 *)(param_1 + 0x5c) = 0;
  }
  pvVar1 = operator_new(0x358);
  uStack_8 = 0;
  if (pvVar1 == (void *)0x0) {
    uStack_374 = 0;
  }
  else {
    uStack_374 = FUN_004bd420(param_1 + 0x1d2,param_1 + 0x1f2,*(undefined4 *)(param_1 + 0x5c),
                              uStack_14,param_1 + 0x202,1,CONCAT22(extraout_var,local_18));
  }
  uStack_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x60) = uStack_374;
  if (*(int *)(param_1 + 0x60) != 0) {
    *(int *)(*(int *)(param_1 + 0x60) + 0x2f4) = param_1;
    FUN_004be7b1();
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 0x28;
    if ((*(int *)(param_1 + 0x5c) != 0x1f) || (_DAT_005a3660 <= *(float *)(param_1 + 0x38))) {
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
    }
    else {
      fStack_28 = _DAT_005a365c - *(float *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
      *(float *)(*(int *)(param_1 + 0x60) + 0x28) = *(float *)(param_1 + 0x38) / fStack_28;
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
    }
  }
  FUN_004faa02(*(undefined4 *)(param_1 + 0x778));
  if (*(int *)(param_1 + 0x198) != 0) {
    if (*(int *)(param_1 + 0x198) != 0) {
      FUN_00493d20(1);
    }
    *(undefined4 *)(param_1 + 0x198) = 0;
  }
  if (((local_18 == 0) && ('\0' < *(char *)(param_1 + 0x79b))) &&
     ((*(int *)(param_1 + 0x5c) == 0 ||
      ((((*(int *)(param_1 + 0x5c) == 1 || (*(int *)(param_1 + 0x5c) == 8)) ||
        (*(int *)(param_1 + 0x5c) == 3)) || (*(int *)(param_1 + 0x5c) == 2)))))) {
    _memset(auStack_58,0,0x20);
    uStack_56 = *(undefined2 *)(param_1 + 0x7aa);
    _memset(auStack_38,0,0x10);
    auStack_38[0] = *(undefined1 *)(param_1 + 0x7ac);
    uStack_30 = *(undefined1 *)(param_1 + 0x7ad);
    pvVar1 = operator_new(0x358);
    uStack_8 = 1;
    if (pvVar1 == (void *)0x0) {
      uStack_37c = 0;
    }
    else {
      uStack_37c = FUN_004bd420(auStack_58,auStack_38,0x55,0,0,1,0);
    }
    uStack_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x198) = uStack_37c;
    if (*(int *)(param_1 + 0x198) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x2f4) = 0;
      FUN_004be7b1();
      *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x2e4) = 0x28;
      *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x24) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x28) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
      FUN_004c0746(1,*(undefined4 *)(param_1 + 0x7a0));
    }
  }
  FUN_00527848();
  if (*(short *)(param_1 + 0x450) < 1) {
    *(undefined4 *)(param_1 + 0x254) = 4;
    FUN_0051dba4(0xc,1);
  }
  else {
    FUN_0051dba4(1,1);
  }
  for (iStack_1c = 0; iStack_1c < 7; iStack_1c = iStack_1c + 1) {
    if (*(int *)(param_1 + 0x56c + iStack_1c * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + iStack_1c * 4));
      *(undefined4 *)(param_1 + 0x56c + iStack_1c * 4) = 0;
    }
    if (*(int *)(param_1 + 0x588 + iStack_1c * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x588 + iStack_1c * 4));
      *(undefined4 *)(param_1 + 0x588 + iStack_1c * 4) = 0;
    }
  }
  for (iStack_1c = 0; iStack_1c < 7; iStack_1c = iStack_1c + 1) {
    if (*(int *)(param_1 + 0x5a4 + iStack_1c * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5a4 + iStack_1c * 4));
      *(undefined4 *)(param_1 + 0x5a4 + iStack_1c * 4) = 0;
    }
  }
  if ((*(int *)(param_1 + 0x24c) == 0x24) || (*(int *)(param_1 + 0x24c) == 0x25)) {
    aiStack_188[7] = 0;
    if (*(int *)(param_1 + 0x24c) == 0x25) {
      aiStack_188[7] = 3;
    }
    if (*(short *)(param_1 + 0x1d6) == 0xb) {
      aiStack_188[7] = 1;
    }
    if (*(short *)(param_1 + 0x1d6) == 10) {
      aiStack_188[7] = 2;
    }
    afStack_d8[0x1c] = -1.7058425e+38;
    afStack_d8[0x1d] = -NAN;
    afStack_d8[0x1e] = -2.8312556e+38;
    afStack_d8[0x1f] = -1.7058253e+38;
    if ((aiStack_188[7] == 2) && (*(short *)(param_1 + 0x1ee) != 0x3a2)) {
      aiStack_188[0] = 3;
      aiStack_188[1] = 6;
      aiStack_188[2] = 4;
      aiStack_188[3] = 7;
      aiStack_188[4] = 5;
      aiStack_188[5] = 6;
      aiStack_188[6] = 7;
      for (iStack_18c = 0; iStack_18c < 7; iStack_18c = iStack_18c + 1) {
        *(undefined4 *)(param_1 + 0x5a4 + iStack_18c * 4) = 0;
        pvVar1 = operator_new(0xcc);
        uStack_8 = 2;
        if (pvVar1 == (void *)0x0) {
          uStack_380 = 0;
        }
        else {
          uVar6 = 0;
          uVar5 = 1;
          uVar3 = 0;
          iVar4 = param_1;
          puVar2 = (undefined4 *)
                   FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                (float)iStack_18c * _DAT_005a4158 +
                                *(float *)(param_1 + 0x30) + _DAT_005a415c,
                                *(undefined4 *)(param_1 + 0x2c));
          uStack_380 = FUN_004d3d20(*puVar2,puVar2[1],puVar2[2],uVar3,iVar4,uVar5,uVar6);
        }
        uStack_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x5a4 + iStack_18c * 4) = uStack_380;
        if (*(int *)(param_1 + 0x5a4 + iStack_18c * 4) != 0) {
          *(int *)(*(int *)(param_1 + 0x5a4 + iStack_18c * 4) + 0x90) = iStack_18c * 0x96;
          *(int *)(*(int *)(param_1 + 0x5a4 + iStack_18c * 4) + 0xb4) = aiStack_188[iStack_18c];
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x5a4 + iStack_18c * 4));
        }
      }
    }
    aiStack_188[8] = 8;
    aiStack_188[9] = 9;
    aiStack_188[10] = 1;
    aiStack_188[0xb] = 6;
    aiStack_188[0xc] = 7;
    aiStack_188[0xd] = 2;
    aiStack_188[0xe] = 3;
    afStack_d8[0] = 0.2;
    afStack_d8[1] = 0.2;
    afStack_d8[2] = 0.8;
    afStack_d8[3] = 0.6;
    afStack_d8[4] = 0.6;
    afStack_d8[5] = 0.8;
    afStack_d8[6] = 0.8;
    afStack_d8[7] = 0.2;
    afStack_d8[8] = 0.2;
    afStack_d8[9] = 0.8;
    afStack_d8[10] = 0.6;
    afStack_d8[0xb] = 0.6;
    afStack_d8[0xc] = 0.8;
    afStack_d8[0xd] = 0.8;
    afStack_d8[0xe] = 0.2;
    afStack_d8[0xf] = 0.2;
    afStack_d8[0x10] = 0.8;
    afStack_d8[0x11] = 0.6;
    afStack_d8[0x12] = 0.6;
    afStack_d8[0x13] = 0.8;
    afStack_d8[0x14] = 0.8;
    afStack_d8[0x15] = 0.2;
    afStack_d8[0x16] = 0.2;
    afStack_d8[0x17] = 0.8;
    afStack_d8[0x18] = 0.6;
    afStack_d8[0x19] = 0.6;
    afStack_d8[0x1a] = 0.8;
    afStack_d8[0x1b] = 0.8;
    afStack_14c[0] = 0.3;
    afStack_14c[1] = 0.3;
    afStack_14c[2] = 1.0;
    afStack_14c[3] = 0.8;
    afStack_14c[4] = 0.8;
    afStack_14c[5] = 1.0;
    afStack_14c[6] = 1.0;
    afStack_14c[7] = 0.3;
    afStack_14c[8] = 0.3;
    afStack_14c[9] = 1.0;
    afStack_14c[10] = 0.8;
    afStack_14c[0xb] = 0.8;
    afStack_14c[0xc] = 1.0;
    afStack_14c[0xd] = 1.0;
    afStack_14c[0xe] = 0.3;
    afStack_14c[0xf] = 0.3;
    afStack_14c[0x10] = 1.0;
    afStack_14c[0x11] = 0.8;
    afStack_14c[0x12] = 0.8;
    afStack_14c[0x13] = 1.0;
    afStack_14c[0x14] = 1.0;
    afStack_14c[0x15] = 0.3;
    afStack_14c[0x16] = 0.3;
    afStack_14c[0x17] = 1.0;
    afStack_14c[0x18] = 0.8;
    afStack_14c[0x19] = 0.8;
    afStack_14c[0x1a] = 1.0;
    afStack_14c[0x1b] = 1.0;
    for (iStack_dc = 0; iStack_dc < 7; iStack_dc = iStack_dc + 1) {
      if ((iStack_dc != 2) || ((aiStack_188[7] != 1 && (aiStack_188[7] != 2)))) {
        if ((4 < iStack_dc) && (aiStack_188[7] != 2)) break;
        pvVar1 = operator_new(0x180);
        uStack_8 = 3;
        if (pvVar1 == (void *)0x0) {
          uStack_384 = 0;
        }
        else {
          uStack_384 = FUN_004c8f70(0x65,0,afStack_d8[aiStack_188[7] * 7 + iStack_dc] *
                                           *(float *)(param_1 + 0x38),
                                    afStack_14c[aiStack_188[7] * 7 + iStack_dc] *
                                    *(float *)(param_1 + 0x38),
                                    afStack_d8[aiStack_188[7] * 7 + iStack_dc] *
                                    *(float *)(param_1 + 0x38),0,8,0x50);
        }
        uStack_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x56c + iStack_dc * 4) = uStack_384;
        if (*(int *)(param_1 + 0x56c + iStack_dc * 4) != 0) {
          (**(code **)(**(int **)(param_1 + 0x56c + iStack_dc * 4) + 0x3c))
                    (afStack_d8[aiStack_188[7] + 0x1c]);
          *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_dc * 4) + 0x6c) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_dc * 4) + 0x104) = 0;
          FUN_004310a0(param_1 + 100 + aiStack_188[iStack_dc + 8] * 0xc);
          *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_dc * 4) + 0x10c) = 0;
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_dc * 4));
        }
      }
    }
  }
  for (iStack_1c = 0; iStack_1c < 4; iStack_1c = iStack_1c + 1) {
    if (*(int *)(param_1 + 0x5c0 + iStack_1c * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5c0 + iStack_1c * 4));
      *(undefined4 *)(param_1 + 0x5c0 + iStack_1c * 4) = 0;
    }
  }
  if (((((*(int *)(param_1 + 0x24c) == 0x15) && (*(short *)(param_1 + 0x1d2) == 10)) ||
       ((*(int *)(param_1 + 0x24c) == 0x1c && (*(short *)(param_1 + 0x1d2) == 2)))) ||
      (((*(int *)(param_1 + 0x24c) == 0x19 && (*(short *)(param_1 + 0x1d2) == 3)) &&
       (*(short *)(param_1 + 0x1d4) == 8)))) ||
     ((*(int *)(param_1 + 0x24c) == 0x19 && (*(short *)(param_1 + 0x1d2) == 0xc)))) {
    for (iStack_190 = 0; iStack_190 < 4; iStack_190 = iStack_190 + 1) {
      if (*(int *)(param_1 + 0x5c0 + iStack_190 * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5c0 + iStack_190 * 4));
        *(undefined4 *)(param_1 + 0x5c0 + iStack_190 * 4) = 0;
      }
      pvVar1 = operator_new(0x1b4);
      uStack_8 = 4;
      if (pvVar1 == (void *)0x0) {
        uStack_388 = 0;
      }
      else {
        uStack_388 = FUN_0052ff60(6,3,param_1);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x5c0 + iStack_190 * 4) = uStack_388;
      if (*(int *)(param_1 + 0x5c0 + iStack_190 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x5c0 + iStack_190 * 4) + 0x38))();
        (**(code **)(**(int **)(param_1 + 0x5c0 + iStack_190 * 4) + 0x40))
                  (0,((float)iStack_190 * _DAT_005a4148) / _DAT_005a3794,0);
        (**(code **)(**(int **)(param_1 + 0x5c0 + iStack_190 * 4) + 0x3c))
                  ((float)iStack_190 * _DAT_005a4158 + *(float *)(param_1 + 0x28),
                   (float)iStack_190 * _DAT_005a4158 + *(float *)(param_1 + 0x30) + _DAT_005a430c,
                   (float)iStack_190 * _DAT_005a4158 + *(float *)(param_1 + 0x2c));
        *(float *)(*(int *)(param_1 + 0x5c0 + iStack_190 * 4) + 0x1a0) =
             *(float *)(*(int *)(param_1 + 0x5c0 + iStack_190 * 4) + 0x1a0) * _DAT_005a34a0;
        *(float *)(*(int *)(param_1 + 0x5c0 + iStack_190 * 4) + 0x19c) =
             *(float *)(*(int *)(param_1 + 0x5c0 + iStack_190 * 4) + 0x19c) * _DAT_005a34a0;
        *(float *)(*(int *)(param_1 + 0x5c0 + iStack_190 * 4) + 0x198) =
             (float)iStack_190 + _DAT_005a366c;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x5c0 + iStack_190 * 4));
      }
    }
  }
  if (((*(int *)(param_1 + 0x24c) == 0x22) || (*(int *)(param_1 + 0x24c) == 0x17)) ||
     ((*(int *)(param_1 + 0x24c) == 0x15 && (*(short *)(param_1 + 0x1d2) == 10)))) {
    *(undefined1 *)(param_1 + 0x78e) = 1;
    iStack_1a0 = 1;
    uStack_194 = 0x47;
    uStack_198 = 0xffffaaff;
    if (*(int *)(param_1 + 0x24c) == 0x17) {
      iStack_1a0 = 2;
      *(undefined1 *)(param_1 + 0x78e) = 0;
      uStack_198 = 0xff33ff66;
    }
    else if (*(int *)(param_1 + 0x24c) == 0x15) {
      uStack_194 = 0x3c;
      uStack_198 = 0xffee8800;
    }
    for (iStack_19c = 0; iStack_19c < iStack_1a0; iStack_19c = iStack_19c + 1) {
      if (*(int *)(param_1 + 0x56c + iStack_19c * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + iStack_19c * 4));
        *(undefined4 *)(param_1 + 0x56c + iStack_19c * 4) = 0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 5;
      if (pvVar1 == (void *)0x0) {
        uStack_38c = 0;
      }
      else {
        uStack_38c = FUN_004c8f70(uStack_194,0,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_19c * 4) = uStack_38c;
      if (*(int *)(param_1 + 0x56c + iStack_19c * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_19c * 4) + 0x3c))(uStack_198);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_19c * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_19c * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_19c * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_19c * 4));
      }
    }
  }
  if ((*(int *)(param_1 + 0x24c) == 0x1d) && (*(short *)(param_1 + 0x1d2) == 1)) {
    iStack_1a8 = 1;
    for (iStack_1a4 = 0; iStack_1a4 < iStack_1a8; iStack_1a4 = iStack_1a4 + 1) {
      if (*(int *)(param_1 + 0x56c + iStack_1a4 * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + iStack_1a4 * 4));
        *(undefined4 *)(param_1 + 0x56c + iStack_1a4 * 4) = 0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 6;
      if (pvVar1 == (void *)0x0) {
        uStack_390 = 0;
      }
      else {
        uStack_390 = FUN_004c8f70(0x47,0,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_1a4 * 4) = uStack_390;
      if (*(int *)(param_1 + 0x56c + iStack_1a4 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_1a4 * 4) + 0x3c))(0xffff00ff);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1a4 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1a4 * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1a4 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_1a4 * 4));
      }
    }
  }
  if ((*(int *)(param_1 + 0x5c) == 0x26) && (*(int *)(param_1 + 0x56c) == 0)) {
    uStack_1ac = 0xff005588;
    for (iStack_1b0 = 0; iStack_1b0 < 7; iStack_1b0 = iStack_1b0 + 1) {
      fStack_1b4 = 1.2;
      if (iStack_1b0 == 0) {
        fStack_1b4 = 1.0;
      }
      if (iStack_1b0 == 5) {
        fStack_1b4 = 3.0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 7;
      if (pvVar1 == (void *)0x0) {
        uStack_394 = 0;
      }
      else {
        uStack_394 = FUN_004c8f70(0x65,0,fStack_1b4 * *(float *)(param_1 + 0x38),
                                  fStack_1b4 * *(float *)(param_1 + 0x38),
                                  fStack_1b4 * *(float *)(param_1 + 0x38),0,8,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_1b0 * 4) = uStack_394;
      if (*(int *)(param_1 + 0x56c + iStack_1b0 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_1b0 * 4) + 0x3c))(0xffff5500);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1b0 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1b0 * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100 + iStack_1b0 * 0xc);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1b0 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_1b0 * 4));
      }
    }
  }
  if (((*(char *)(param_1 + 0x79c) != '\0') && (*(int *)(param_1 + 0x7a0) == 0x26)) &&
     (*(int *)(param_1 + 0x588) == 0)) {
    uStack_1b8 = 0xff005588;
    for (iStack_1bc = 0; iStack_1bc < 7; iStack_1bc = iStack_1bc + 1) {
      fStack_1c0 = 1.2;
      if (iStack_1bc == 0) {
        fStack_1c0 = 1.0;
      }
      if (iStack_1bc == 5) {
        fStack_1c0 = 3.0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 8;
      if (pvVar1 == (void *)0x0) {
        uStack_398 = 0;
      }
      else {
        uStack_398 = FUN_004c8f70(0x65,0,fStack_1c0 * *(float *)(param_1 + 0x38),
                                  fStack_1c0 * *(float *)(param_1 + 0x38),
                                  fStack_1c0 * *(float *)(param_1 + 0x38),0,8,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x588 + iStack_1bc * 4) = uStack_398;
      if (*(int *)(param_1 + 0x588 + iStack_1bc * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x588 + iStack_1bc * 4) + 0x3c))(0xffff5500);
        *(undefined4 *)(*(int *)(param_1 + 0x588 + iStack_1bc * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x588 + iStack_1bc * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 0xdc + iStack_1bc * 0xc);
        *(undefined4 *)(*(int *)(param_1 + 0x588 + iStack_1bc * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x588 + iStack_1bc * 4));
      }
    }
  }
  if ((*(int *)(param_1 + 0x24c) == 0x20) && (*(short *)(param_1 + 0x1d2) == 2)) {
    for (iStack_1c4 = 0; iStack_1c4 < 6; iStack_1c4 = iStack_1c4 + 1) {
      if (*(int *)(param_1 + 0x56c + iStack_1c4 * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + iStack_1c4 * 4));
        *(undefined4 *)(param_1 + 0x56c + iStack_1c4 * 4) = 0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 9;
      if (pvVar1 == (void *)0x0) {
        uStack_39c = 0;
      }
      else {
        uStack_39c = FUN_004c8f70(0xb,0,_DAT_005a4678 * *(float *)(param_1 + 0x38),
                                  _DAT_005a4418 * *(float *)(param_1 + 0x38),
                                  _DAT_005a4678 * *(float *)(param_1 + 0x38),0,8,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_1c4 * 4) = uStack_39c;
      if (*(int *)(param_1 + 0x56c + iStack_1c4 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_1c4 * 4) + 0x3c))(0xffaa8800);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1c4 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1c4 * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100 + iStack_1c4 * 0xc);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1c4 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_1c4 * 4));
      }
    }
  }
  if (((*(int *)(param_1 + 0x24c) == 0x10) && (*(short *)(param_1 + 0x1d2) == 0)) &&
     ((*(short *)(param_1 + 0x1d4) == 1 && (*(char *)(param_1 + 0x7a8) != '\x04')))) {
    afStack_1dc[4] = 0.4;
    for (iStack_1c8 = 8; iStack_1c8 < 10; iStack_1c8 = iStack_1c8 + 1) {
      if (*(int *)(param_1 + 0x56c + iStack_1c8 * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + iStack_1c8 * 4));
        *(undefined4 *)(param_1 + 0x56c + iStack_1c8 * 4) = 0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 10;
      if (pvVar1 == (void *)0x0) {
        uStack_3a0 = 0;
      }
      else {
        uStack_3a0 = FUN_004c8f70(0x65,0,afStack_1dc[4] * *(float *)(param_1 + 0x38),
                                  afStack_1dc[4] * *(float *)(param_1 + 0x38),
                                  afStack_1dc[4] * *(float *)(param_1 + 0x38),0,8,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_1c8 * 4) = uStack_3a0;
      if (*(int *)(param_1 + 0x56c + iStack_1c8 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_1c8 * 4) + 0x3c))(0xff88ffaa);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1c8 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1c8 * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100 + iStack_1c8 * 0xc);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1c8 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_1c8 * 4));
      }
    }
  }
  if (*(int *)(param_1 + 0x24c) == 0x27) {
    afStack_1dc[0] = 7.84727e-44;
    afStack_1dc[1] = 7.84727e-44;
    afStack_1dc[2] = 1.41531e-43;
    afStack_1dc[3] = 1.41531e-43;
    afStack_210[8] = 1.4013e-45;
    afStack_210[9] = 1.4013e-45;
    afStack_210[10] = 1.12104e-44;
    afStack_210[0xb] = 1.12104e-44;
    for (iStack_1e0 = 0; iStack_1e0 < 4; iStack_1e0 = iStack_1e0 + 1) {
      if (*(int *)(param_1 + 0x56c + iStack_1e0 * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + iStack_1e0 * 4));
        *(undefined4 *)(param_1 + 0x56c + iStack_1e0 * 4) = 0;
      }
      pvVar1 = operator_new(0x180);
      uStack_8 = 0xb;
      if (pvVar1 == (void *)0x0) {
        uStack_3a4 = 0;
      }
      else {
        uStack_3a4 = FUN_004c8f70(afStack_1dc[iStack_1e0],0,
                                  _DAT_005a3660 * *(float *)(param_1 + 0x38),
                                  _DAT_005a3660 * *(float *)(param_1 + 0x38),
                                  _DAT_005a3660 * *(float *)(param_1 + 0x38),0,
                                  afStack_210[iStack_1e0 + 8],0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_1e0 * 4) = uStack_3a4;
      if (*(int *)(param_1 + 0x56c + iStack_1e0 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_1e0 * 4) + 0x3c))(0xffff5500);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1e0 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1e0 * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_1e0 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_1e0 * 4));
      }
    }
  }
  if (((*(int *)(param_1 + 0x24c) == 0x26) && ('\0' < *(char *)(param_1 + 0x79b))) &&
     (*(int *)(param_1 + 0x570) == 0)) {
    aiStack_24c[0] = 8;
    aiStack_24c[1] = 9;
    aiStack_24c[2] = 1;
    aiStack_24c[3] = 6;
    aiStack_24c[4] = 7;
    aiStack_24c[5] = 2;
    aiStack_24c[6] = 3;
    afStack_210[0] = 0.2;
    afStack_210[1] = 0.2;
    afStack_210[2] = 0.8;
    afStack_210[3] = 0.6;
    afStack_210[4] = 0.6;
    afStack_210[5] = 0.8;
    afStack_210[6] = 0.8;
    afStack_230[0] = 0.3;
    afStack_230[1] = 0.3;
    afStack_230[2] = 1.0;
    afStack_230[3] = 0.8;
    afStack_230[4] = 0.8;
    afStack_230[5] = 1.0;
    afStack_230[6] = 1.0;
    afStack_210[7] = -1.7058529e+38;
    if (*(short *)(param_1 + 0x1ee) == 0x3a2) {
      afStack_210[7] = -1.7084906e+38;
    }
    for (iStack_214 = 1; iStack_214 < 7; iStack_214 = iStack_214 + 1) {
      pvVar1 = operator_new(0x180);
      uStack_8 = 0xc;
      if (pvVar1 == (void *)0x0) {
        uStack_3a8 = 0;
      }
      else {
        uStack_3a8 = FUN_004c8f70(0x7b,0,afStack_210[iStack_214] * *(float *)(param_1 + 0x38),
                                  afStack_230[iStack_214] * *(float *)(param_1 + 0x38),
                                  afStack_210[iStack_214] * *(float *)(param_1 + 0x38),0,1,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_214 * 4) = uStack_3a8;
      if (*(int *)(param_1 + 0x56c + iStack_214 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_214 * 4) + 0x3c))(afStack_210[7]);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_214 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_214 * 4) + 0x104) = 2;
        FUN_004310a0(param_1 + 100 + aiStack_24c[iStack_214] * 0xc);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_214 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_214 * 4));
      }
    }
  }
  if (((*(int *)(param_1 + 0x24c) == 0x21) && (*(short *)(param_1 + 0x1d2) == 1)) &&
     ((DAT_005ccfa0 == 2 && (*(int *)(param_1 + 0x56c) == 0)))) {
    for (iStack_254 = 0; iStack_254 < 7; iStack_254 = iStack_254 + 1) {
      pvVar1 = operator_new(0x180);
      uStack_8 = 0xd;
      if (pvVar1 == (void *)0x0) {
        uStack_3ac = 0;
      }
      else {
        uStack_3ac = FUN_004c8f70(0x65,0,_DAT_005a365c * *(float *)(param_1 + 0x38),
                                  _DAT_005a40f0 * *(float *)(param_1 + 0x38),
                                  _DAT_005a365c * *(float *)(param_1 + 0x38),0,8,0x50);
      }
      uStack_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x56c + iStack_254 * 4) = uStack_3ac;
      if (*(int *)(param_1 + 0x56c + iStack_254 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x56c + iStack_254 * 4) + 0x3c))(0xffff5500);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_254 * 4) + 0x6c) = 1;
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_254 * 4) + 0x104) = 0;
        FUN_004310a0(param_1 + 100 + iStack_254 * 0xc);
        *(undefined4 *)(*(int *)(param_1 + 0x56c + iStack_254 * 4) + 0x10c) = 0;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x56c + iStack_254 * 4));
      }
    }
  }
  if (*(int *)(param_1 + 0x530) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x530));
    *(undefined4 *)(param_1 + 0x530) = 0;
  }
  if (((DAT_0092e660 == 0) && (*(int *)(DAT_005ccf9c + 0x2a39c) == 0)) &&
     (*(int *)(DAT_005ccf9c + 0x2a3a0) == 0)) {
    if (((((*(int *)(param_1 + 0x5c) == 0) || (*(int *)(param_1 + 0x5c) == 1)) ||
         (*(int *)(param_1 + 0x5c) == 2)) ||
        ((*(int *)(param_1 + 0x5c) == 3 || (*(int *)(param_1 + 0x5c) == 4)))) ||
       (*(int *)(param_1 + 0x5c) == 5)) {
      if (*(char *)(param_1 + 0x79c) < '\x01') {
        pvVar1 = operator_new(0xc0);
        uStack_8 = 0xf;
        if (pvVar1 == (void *)0x0) {
          uStack_3b4 = 0;
        }
        else {
          uStack_3b4 = FUN_004df250(3,4,*(undefined4 *)(param_1 + 0x38));
        }
        *(undefined4 *)(param_1 + 0x530) = uStack_3b4;
      }
      else {
        pvVar1 = operator_new(0xc0);
        uStack_8 = 0xe;
        if (pvVar1 == (void *)0x0) {
          uStack_3b0 = 0;
        }
        else {
          uStack_3b0 = FUN_004df250(5,4,*(undefined4 *)(param_1 + 0x38));
        }
        *(undefined4 *)(param_1 + 0x530) = uStack_3b0;
      }
    }
    else if (*(int *)(param_1 + 0x5c) == 0x26) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x10;
      if (pvVar1 == (void *)0x0) {
        uStack_3b8 = 0;
      }
      else {
        uStack_3b8 = FUN_004df250(5,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3b8;
    }
    else if ((*(int *)(param_1 + 0x5c) == 0x14) && (*(short *)(param_1 + 0x1d6) == 0)) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x11;
      if (pvVar1 == (void *)0x0) {
        uStack_3bc = 0;
      }
      else {
        uStack_3bc = FUN_004df250(6,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3bc;
    }
    else if (*(int *)(param_1 + 0x5c) == 0x27) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x12;
      if (pvVar1 == (void *)0x0) {
        uStack_3c0 = 0;
      }
      else {
        uStack_3c0 = FUN_004df250(6,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3c0;
    }
    else if (((*(int *)(param_1 + 0x5c) == 0xb) || (*(int *)(param_1 + 0x5c) == 0x23)) ||
            (*(int *)(param_1 + 0x5c) == 0x2a)) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x13;
      if (pvVar1 == (void *)0x0) {
        uStack_3c4 = 0;
      }
      else {
        uStack_3c4 = FUN_004df250(6,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3c4;
    }
    else if (((*(int *)(param_1 + 0x5c) == 0x24) || (*(int *)(param_1 + 0x5c) == 0x22)) ||
            (*(int *)(param_1 + 0x5c) == 0x2c)) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x14;
      if (pvVar1 == (void *)0x0) {
        uStack_3c8 = 0;
      }
      else {
        uStack_3c8 = FUN_004df250(8,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3c8;
    }
    else if ((*(int *)(param_1 + 0x5c) == 7) || (*(int *)(param_1 + 0x5c) == 0x1d)) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x15;
      if (pvVar1 == (void *)0x0) {
        uStack_3cc = 0;
      }
      else {
        uStack_3cc = FUN_004df250(4,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3cc;
    }
    else if (((*(int *)(param_1 + 0x5c) == 0x18) || (*(int *)(param_1 + 0x5c) == 0x20)) ||
            (*(int *)(param_1 + 0x5c) == 0xc)) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x16;
      if (pvVar1 == (void *)0x0) {
        uStack_3d0 = 0;
      }
      else {
        uStack_3d0 = FUN_004df250(2,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3d0;
    }
    else if (*(int *)(param_1 + 0x5c) == 0x25) {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x17;
      if (pvVar1 == (void *)0x0) {
        uStack_3d4 = 0;
      }
      else {
        uStack_3d4 = FUN_004df250(2,0xcb,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3d4;
    }
    else {
      pvVar1 = operator_new(0xc0);
      uStack_8 = 0x18;
      if (pvVar1 == (void *)0x0) {
        uStack_3d8 = 0;
      }
      else {
        uStack_3d8 = FUN_004df250(3,4,*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x530) = uStack_3d8;
    }
    uStack_8 = 0xffffffff;
    if (*(int *)(param_1 + 0x530) != 0) {
      puVar2 = (undefined4 *)FUN_00430f20(0,0);
      (**(code **)(**(int **)(param_1 + 0x530) + 0x3c))(*puVar2,puVar2[1]);
      FUN_0054ac09(*(undefined4 *)(param_1 + 0x530));
    }
  }
  FUN_004fa508();
  if (*(int *)(param_1 + 0x514) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x514));
    *(undefined4 *)(param_1 + 0x514) = 0;
  }
  if (*(int *)(param_1 + 0x518) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x518));
    *(undefined4 *)(param_1 + 0x518) = 0;
  }
  if (*(int *)(param_1 + 0x51c) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x51c));
    *(undefined4 *)(param_1 + 0x51c) = 0;
  }
  if (*(int *)(param_1 + 0x520) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x520));
    *(undefined4 *)(param_1 + 0x520) = 0;
  }
  if (*(int *)(param_1 + 0x524) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x524));
    *(undefined4 *)(param_1 + 0x524) = 0;
  }
  if (*(int *)(param_1 + 0x24c) < 9) {
    pvVar1 = operator_new(0xb0);
    uStack_8 = 0x19;
    if (pvVar1 == (void *)0x0) {
      uStack_3dc = 0;
    }
    else {
      uStack_3dc = FUN_004e88a0(param_1,1);
    }
    uStack_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x51c) = uStack_3dc;
    pvVar1 = operator_new(0xb0);
    uStack_8 = 0x1a;
    if (pvVar1 == (void *)0x0) {
      uStack_3e0 = 0;
    }
    else {
      uStack_3e0 = FUN_004e88a0(param_1,0);
    }
    uStack_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x520) = uStack_3e0;
    pvVar1 = operator_new(0xb0);
    uStack_8 = 0x1b;
    if (pvVar1 == (void *)0x0) {
      uStack_3e4 = 0;
    }
    else {
      uStack_3e4 = FUN_004e88a0(param_1,4);
    }
    uStack_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x524) = uStack_3e4;
    pvVar1 = operator_new(0xb0);
    uStack_8 = 0x1c;
    if (pvVar1 == (void *)0x0) {
      uStack_3e8 = 0;
    }
    else {
      uStack_3e8 = FUN_004e88a0(param_1,2);
    }
    uStack_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x514) = uStack_3e8;
    pvVar1 = operator_new(0xb0);
    uStack_8 = 0x1d;
    if (pvVar1 == (void *)0x0) {
      uStack_3ec = 0;
    }
    else {
      uStack_3ec = FUN_004e88a0(param_1,3);
    }
    uStack_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x518) = uStack_3ec;
  }
  if (*(int *)(param_1 + 0x51c) != 0) {
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x51c));
  }
  if (*(int *)(param_1 + 0x520) != 0) {
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x520));
  }
  if (*(int *)(param_1 + 0x524) != 0) {
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x524));
  }
  if (*(int *)(param_1 + 0x514) != 0) {
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x514));
  }
  if (*(int *)(param_1 + 0x518) != 0) {
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x518));
  }
  ExceptionList = pvStack_10;
  return 1;
}

