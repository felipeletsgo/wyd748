// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049f0e7 | Name: FUN_0049f0e7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0049f0e7(int *param_1)

{
  undefined1 uVar1;
  char cVar2;
  undefined2 uVar3;
  void *pvVar4;
  int iVar5;
  undefined4 uVar6;
  undefined2 extraout_var;
  undefined2 extraout_var_00;
  undefined2 extraout_var_01;
  undefined2 extraout_var_02;
  undefined4 local_344;
  int local_334;
  int local_330;
  int local_320;
  int local_30c;
  int local_308;
  int local_304;
  undefined4 local_28c;
  undefined4 local_288;
  undefined4 local_284;
  undefined1 local_280 [12];
  undefined1 local_274 [12];
  undefined1 local_268 [2];
  undefined1 local_266;
  undefined1 local_265;
  undefined1 local_264;
  undefined1 local_263;
  undefined1 local_262;
  undefined1 local_261;
  undefined1 local_25f;
  undefined1 local_25e;
  undefined1 local_25d;
  undefined1 local_25c;
  undefined1 local_25b;
  undefined1 local_25a;
  undefined1 local_259;
  int local_258;
  undefined2 local_254;
  undefined2 local_252;
  undefined2 local_250;
  undefined2 local_24e;
  undefined2 local_24c;
  undefined2 local_24a;
  undefined2 local_248;
  undefined2 local_246;
  undefined2 local_244;
  undefined2 local_242;
  undefined2 local_240;
  undefined2 local_23e;
  undefined2 local_23c;
  undefined2 local_23a;
  undefined2 local_238;
  undefined2 local_236;
  undefined2 local_234;
  undefined *local_230 [4];
  undefined1 local_220 [128];
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined4 local_198;
  undefined1 local_194 [12];
  undefined1 local_188 [12];
  undefined *local_17c [4];
  undefined *local_16c;
  int local_168;
  int local_164;
  undefined4 local_160;
  float local_15c;
  int local_158;
  int *local_154;
  int local_150;
  int local_14c;
  int local_148;
  int local_144;
  undefined1 local_140 [4];
  int local_13c;
  int local_138;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  int *local_120;
  int local_11c;
  int local_118;
  undefined1 local_114 [128];
  int local_94;
  undefined1 local_90 [128];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0298;
  local_10 = ExceptionList;
  if (DAT_005b892c == 2) {
    ExceptionList = &local_10;
    (**(code **)(*param_1 + 0x3c))(s_UI_SelCharScene2_txt_005b92c8);
  }
  else {
    ExceptionList = &local_10;
    (**(code **)(*param_1 + 0x3c))(s_UI_SelCharScene_txt_005b92e0);
  }
  *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0x444433;
  *(undefined4 *)(DAT_005ccf9c + 0x2a5e8) = 0;
  (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x50))(0);
  FUN_0058f078(local_114,s_env_character_trn_005b92f4);
  FUN_0058f078(local_90,s_env_character_dat_005b9308);
  DAT_005b8924 = 0x800;
  DAT_005b8928 = 0x800;
  pvVar4 = operator_new(0x20a70);
  local_8 = 0;
  if (pvVar4 == (void *)0x0) {
    local_304 = 0;
  }
  else {
    local_304 = FUN_00532390();
  }
  local_8 = 0xffffffff;
  param_1[0x1a] = local_304;
  iVar5 = FUN_00533dd7(local_114);
  if (iVar5 == 0) {
    if (param_1[0x15] == 0) {
      FUN_0049ed58(0xd,0,0,0,0);
    }
    param_1[0x15] = 1;
  }
  param_1[0x10] = param_1[0x1a];
  FUN_00534968();
  pvVar4 = operator_new(0x404c);
  local_8 = 1;
  if (pvVar4 == (void *)0x0) {
    local_308 = 0;
  }
  else {
    local_308 = FUN_00540ea3(param_1[0x10]);
  }
  local_8 = 0xffffffff;
  param_1[0x1c] = local_308;
  FUN_0054ac09(param_1[0x1c]);
  iVar5 = FUN_00541065(local_90);
  if (iVar5 == 0) {
    FUN_00431159(s_DataFile_Not_Found_005b931c);
    MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_DataFile_Not_Found__005b9340,s_File_Lost_005b9334,0);
    PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
    uVar6 = 0;
  }
  else {
    FUN_0054ac09(param_1[0x1a]);
    pvVar4 = operator_new(0x22a4);
    local_8 = 2;
    if (pvVar4 == (void *)0x0) {
      local_30c = 0;
    }
    else {
      local_30c = FUN_00547890();
    }
    local_8 = 0xffffffff;
    param_1[0x11] = local_30c;
    FUN_0054ac09(param_1[0x11]);
    FUN_00549bbf(3);
    param_1[0x9b8a] = 0;
    param_1[0x9b8b] = 0;
    local_134 = (**(code **)(*(int *)param_1[10] + 0x48))(0x503);
    if (local_134 != 0) {
      *(undefined4 *)(local_134 + 0x3c) = 0;
    }
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x273);
    param_1[0x9ba4] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x272);
    param_1[0x9ba3] = iVar5;
    if (param_1[0x9ba3] != 0) {
      (**(code **)(*(int *)param_1[0x9ba3] + 0x60))(0);
      *(undefined4 *)(param_1[0x9ba3] + 0x60) = 1;
      if (param_1[0x9ba4] != 0) {
        *(undefined4 *)(param_1[0x9ba4] + 0x105c) = 1;
      }
      *(int *)(param_1[10] + 0x138) = param_1[0x9ba3];
    }
    if (DAT_005b892c == 2) {
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1618);
      param_1[0x9ba5] = iVar5;
      *(float *)(param_1[0x9ba5] + 0x50) = *(float *)(param_1[0x9ba5] + 0x50) + _DAT_005a4104;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1607);
      param_1[0x9ba6] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1608);
      param_1[0x9ba7] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1609);
      param_1[0x9ba8] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160a);
      param_1[0x9ba9] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160b);
      param_1[0x9baa] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160c);
      param_1[0x9bab] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160d);
      param_1[0x9bac] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160e);
      param_1[0x9bad] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160f);
      param_1[0x9bae] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1610);
      param_1[0x9baf] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1611);
      param_1[0x9bb0] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1612);
      param_1[0x9bb1] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1613);
      param_1[0x9bb2] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1616);
      param_1[0x9bb5] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1617);
      param_1[0x9bb6] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1628);
      param_1[0x9bb7] = iVar5;
      iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x561);
      param_1[0x9bb8] = iVar5;
      (**(code **)(*(int *)param_1[0x9ba3] + 0x70))
                (((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9ba3] + 0x54) * _DAT_005a34a0) - _DAT_005a3670,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9ba3] + 0x58) * _DAT_005a34a0);
    }
    else {
      local_150 = (**(code **)(*(int *)param_1[10] + 0x48))(0x504);
      if (local_150 != 0) {
        *(undefined4 *)(local_150 + 0x3c) = 0;
      }
    }
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x520);
    param_1[0x9bb3] = iVar5;
    *(undefined4 *)(param_1[0x9bb3] + 0xcd4) = 1;
    local_118 = (**(code **)(*(int *)param_1[10] + 0x48))(0x607);
    local_130 = (**(code **)(*(int *)param_1[10] + 0x48))(0x608);
    if (local_118 != 0) {
      *(undefined4 *)(local_118 + 0x3c) = 0;
    }
    if (local_130 != 0) {
      *(undefined4 *)(local_130 + 0x3c) = 0;
    }
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x620);
    param_1[0x9b9a] = iVar5;
    if (param_1[0x9b9a] != 0) {
      (**(code **)(*(int *)param_1[0x9b9a] + 0x60))(0);
    }
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x621);
    param_1[0x9b9b] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1204);
    param_1[0x9b94] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1208);
    param_1[0x9b95] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1207);
    param_1[0x9b96] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x609);
    param_1[0x9b97] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x502);
    param_1[0x9b98] = iVar5;
    iVar5 = (**(code **)(*(int *)param_1[10] + 0x48))(0x606);
    param_1[0x9b99] = iVar5;
    (**(code **)(*param_1 + 0x68))(1);
    local_94 = DAT_013b71e8 + 4;
    for (local_12c = 0; local_12c < 4; local_12c = local_12c + 1) {
      if ((local_94 != 0) && (*(char *)(local_94 + 0x10 + local_12c * 0x10) != '\0')) {
        local_168 = (int)*(short *)(local_94 + 0xc0 + local_12c * 0x80) % 0x1964;
        if ((local_168 == 0x16) ||
           ((((local_168 == 0x17 || (local_168 == 0x18)) || (local_168 == 0x19)) ||
            (local_168 == 0x20)))) {
          *(undefined2 *)(local_94 + 0xc0 + local_12c * 0x80) = DAT_005d04dc;
        }
        pvVar4 = operator_new(0x894);
        local_8 = 3;
        if (pvVar4 == (void *)0x0) {
          local_320 = 0;
        }
        else {
          local_320 = FUN_004f7ea6(param_1);
        }
        local_8 = 0xffffffff;
        param_1[local_12c + 0x9b8c] = local_320;
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x192) = 0;
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x194) = 0;
        FUN_0058f078(param_1[local_12c + 0x9b8c] + 0x168,&DAT_005b9354,
                     local_94 + 0x10 + local_12c * 0x10);
        FUN_0058f220(param_1[local_12c + 0x9b8c] + 0x444,local_94 + 0x50 + local_12c * 0x1c,0x1c);
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1d2) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xc0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1d4) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xc0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        if (((int)*(short *)(local_94 + 200 + local_12c * 0x80) % 0x1964 < 0xdac) ||
           (0xdae < (int)*(short *)(local_94 + 200 + local_12c * 0x80) % 0x1964)) {
          *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1d6) =
               *(undefined2 *)
                (&DAT_00d44990 +
                ((int)*(short *)(local_94 + 200 + local_12c * 0x80) % 0x1964) * 0x8c);
          *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1d8) =
               *(undefined2 *)
                (&DAT_00d44992 +
                ((int)*(short *)(local_94 + 200 + local_12c * 0x80) % 0x1964) * 0x8c);
        }
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1da) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xd0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1dc) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xd0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1de) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xd8 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1e0) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xd8 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1e2) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xe0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1e4) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xe0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1e6) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xe8 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1e8) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xe8 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1ee) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xf0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1f0) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xf0 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1ea) =
             *(undefined2 *)
              (&DAT_00d44990 + ((int)*(short *)(local_94 + 0xf8 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1ec) =
             *(undefined2 *)
              (&DAT_00d44992 + ((int)*(short *)(local_94 + 0xf8 + local_12c * 0x80) % 0x1964) * 0x8c
              );
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x7ae) =
             *(undefined2 *)(local_94 + 0x128 + local_12c * 0x80);
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x7aa) =
             *(undefined2 *)(&DAT_00d44992 + *(short *)(local_94 + 0x138 + local_12c * 0x80) * 0x8c)
        ;
        uVar3 = FUN_00526cf4(*(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x7aa));
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x7aa) = uVar3;
        if ((((local_168 % 10 == 8) || (local_168 == 0x15)) ||
            ((local_168 == 0x16 ||
             (((local_168 == 0x17 || (local_168 == 0x18)) || (local_168 == 0x19)))))) ||
           (local_168 == 0x20)) {
          *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f2) = 0;
        }
        else {
          uVar1 = FUN_0054e06c(local_94 + 0xc0 + local_12c * 0x80);
          *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f2) = uVar1;
        }
        uVar1 = FUN_0054e06c(local_94 + 200 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 499) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0xd0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 500) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0xd8 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f5) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0xe0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f6) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0xe8 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f7) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0xf0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f9) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0xf8 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1f8) = uVar1;
        uVar1 = FUN_0054e06c(local_94 + 0x138 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x7ac) = uVar1;
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x1fa) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xc0 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x1fb) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 200 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x1fc) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xd0 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x1fd) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xd8 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x1fe) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xe0 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x1ff) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xe8 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x201) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xf0 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x200) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0xf8 + local_12c * 0x80) % 0x1964) * 0x8c];
        *(undefined *)(param_1[local_12c + 0x9b8c] + 0x7ad) =
             (&DAT_00d449da)[((int)*(short *)(local_94 + 0x138 + local_12c * 0x80) % 0x1964) * 0x8c]
        ;
        uVar1 = FUN_0054df49(local_94 + 0xc0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x202) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 200 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x203) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 0xd0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x204) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 0xd8 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x205) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 0xe0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x206) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 0xe8 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x207) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 0xf0 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x209) = uVar1;
        uVar1 = FUN_0054df49(local_94 + 0xf8 + local_12c * 0x80);
        *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x208) = uVar1;
        if (((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fb) == '\0') ||
            (4 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1fb))) ||
           (*(byte *)(param_1[local_12c + 0x9b8c] + 499) < 10)) {
          if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fb) == '\x04') &&
             (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 499))) {
            uVar1 = FUN_004173cf(local_94 + 200 + local_12c * 0x80);
            *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1fb) = uVar1;
          }
        }
        else {
          cVar2 = FUN_004173cf(local_94 + 200 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x1fb) = cVar2 + '\x04';
        }
        if (((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fc) == '\0') ||
            (4 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1fc))) ||
           (*(byte *)(param_1[local_12c + 0x9b8c] + 500) < 10)) {
          if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fc) == '\x04') &&
             (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 500))) {
            uVar1 = FUN_004173cf(local_94 + 0xd0 + local_12c * 0x80);
            *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1fc) = uVar1;
          }
        }
        else {
          cVar2 = FUN_004173cf(local_94 + 0xd0 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x1fc) = cVar2 + '\x04';
        }
        if (((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fd) == '\0') ||
            (4 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1fd))) ||
           (*(byte *)(param_1[local_12c + 0x9b8c] + 0x1f5) < 10)) {
          if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fd) == '\x04') &&
             (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1f5))) {
            uVar1 = FUN_004173cf(local_94 + 0xd8 + local_12c * 0x80);
            *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1fd) = uVar1;
          }
        }
        else {
          cVar2 = FUN_004173cf(local_94 + 0xd8 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x1fd) = cVar2 + '\x04';
        }
        if (((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fe) == '\0') ||
            (4 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1fe))) ||
           (*(byte *)(param_1[local_12c + 0x9b8c] + 0x1f6) < 10)) {
          if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x1fe) == '\x04') &&
             (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1f6))) {
            uVar1 = FUN_004173cf(local_94 + 0xe0 + local_12c * 0x80);
            *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1fe) = uVar1;
          }
        }
        else {
          cVar2 = FUN_004173cf(local_94 + 0xe0 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x1fe) = cVar2 + '\x04';
        }
        if (((*(char *)(param_1[local_12c + 0x9b8c] + 0x1ff) == '\0') ||
            (4 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1ff))) ||
           (*(byte *)(param_1[local_12c + 0x9b8c] + 0x1f7) < 10)) {
          if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x1ff) == '\x04') &&
             (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1f7))) {
            uVar1 = FUN_004173cf(local_94 + 0xe8 + local_12c * 0x80);
            *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x1ff) = uVar1;
          }
        }
        else {
          cVar2 = FUN_004173cf(local_94 + 0xe8 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x1ff) = cVar2 + '\x04';
        }
        if (((*(char *)(param_1[local_12c + 0x9b8c] + 0x201) == '\0') ||
            (4 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x201))) ||
           (*(byte *)(param_1[local_12c + 0x9b8c] + 0x1f9) < 10)) {
          if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x201) == '\x04') &&
             (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1f9))) {
            uVar1 = FUN_004173cf(local_94 + 0xf0 + local_12c * 0x80);
            *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x201) = uVar1;
          }
        }
        else {
          cVar2 = FUN_004173cf(local_94 + 0xf0 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x201) = cVar2 + '\x04';
        }
        if ((*(byte *)(param_1[local_12c + 0x9b8c] + 0x200) < 5) &&
           (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1f8))) {
          cVar2 = FUN_004173cf(local_94 + 0xf8 + local_12c * 0x80);
          *(char *)(param_1[local_12c + 0x9b8c] + 0x200) = cVar2 + '\x04';
        }
        else if ((*(char *)(param_1[local_12c + 0x9b8c] + 0x200) == '\x04') &&
                (9 < *(byte *)(param_1[local_12c + 0x9b8c] + 0x1f8))) {
          uVar1 = FUN_004173cf(local_94 + 0xf8 + local_12c * 0x80);
          *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x200) = uVar1;
        }
        if (*(short *)(local_94 + 0x138 + local_12c * 0x80) < 1) {
          *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x79b) = 0;
        }
        else {
          *(undefined1 *)(param_1[local_12c + 0x9b8c] + 0x79b) = 1;
        }
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x7e0) =
             *(undefined2 *)(local_94 + 0x138 + local_12c * 0x80);
        local_15c = (float)(int)*(short *)(param_1[local_12c + 0x9b8c] + 0x45a);
        FUN_005277a7(local_15c);
        FUN_004faf13(CONCAT22(extraout_var,*(undefined2 *)(local_94 + 0xc0 + local_12c * 0x80)));
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x450) = 1;
        local_158 = FUN_0054cd07(local_94 + 0xf0 + local_12c * 0x80,0x15);
        if (local_158 == 0x29) {
          *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1ea) =
               *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1ee);
          *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1ec) =
               *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x1f0);
        }
        (**(code **)(*(int *)param_1[local_12c + 0x9b8c] + 0x38))();
        FUN_0051bb41(CONCAT22(extraout_var_01,*(undefined2 *)(local_94 + 0xf0 + local_12c * 0x80)),
                     CONCAT22(extraout_var_01,*(undefined2 *)(local_94 + 0xf8 + local_12c * 0x80)));
        if (DAT_005b892c == 2) {
          (**(code **)(*(int *)param_1[local_12c + 0x9b8c] + 0x40))(0,0xc2340000,0);
          CSimpleArray<>();
          CSimpleArray<>();
          CSimpleArray<>();
          uVar6 = FUN_00430f50(0x45005000,0,0x45000333);
          FUN_004310a0(uVar6);
          uVar6 = FUN_00430f50(0x45000333,0,0x45005000);
          FUN_004310a0(uVar6);
          FUN_004a8940(&local_1a0,local_188,local_194,(float)local_12c * _DAT_005a4528);
          (**(code **)(*(int *)param_1[local_12c + 0x9b8c] + 0x3c))(local_1a0,local_19c,local_198);
        }
        else {
          (**(code **)(*(int *)param_1[local_12c + 0x9b8c] + 0x40))
                    (0,((float)-local_12c * _DAT_005a4148) / _DAT_005a4104,0);
          if (local_12c < 2) {
            (**(code **)(*(int *)param_1[local_12c + 0x9b8c] + 0x3c))
                      (param_1[0x9bb9],0,
                       (float)local_12c * _DAT_005a4520 + ((float)param_1[0x9bba] - _DAT_005a4524));
          }
          else {
            (**(code **)(*(int *)param_1[local_12c + 0x9b8c] + 0x3c))
                      ((float)param_1[0x9bb9] - (float)(local_12c + -1) * _DAT_005a4520,0,
                       param_1[0x9bba]);
          }
        }
        uVar3 = *(undefined2 *)(DAT_013b71e8 + 0x2c4 + local_12c * 2);
        *(undefined2 *)(param_1[local_12c + 0x9b8c] + 0x680) = uVar3;
        local_160 = CONCAT22(local_160._2_2_,uVar3) & 0xffff7fff;
        local_17c[0] = &DAT_00a37ff8;
        local_17c[1] = &DAT_00a38078;
        local_17c[2] = &DAT_00a397f8;
        local_17c[3] = &DAT_00a3e1f8;
        local_16c = &DAT_00a380f8;
        local_164 = FUN_00552f42(*(undefined2 *)(DAT_013b71e8 + 4 + local_12c * 2),
                                 *(undefined2 *)(DAT_013b71e8 + 0xc + local_12c * 2));
        local_154 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x510);
        if ((local_164 < 5) && (-1 < local_164)) {
          (**(code **)(*local_154 + 0x80))(local_17c[local_164],0);
        }
        else {
          FUN_0058f078(local_220,s___d___d__005b9358,
                       *(undefined2 *)(DAT_013b71e8 + 4 + local_12c * 2),
                       *(undefined2 *)(DAT_013b71e8 + 0xc + local_12c * 2));
          (**(code **)(*local_154 + 0x80))(local_220,0);
        }
        FUN_0054ac09(param_1[local_12c + 0x9b8c]);
      }
    }
    pvVar4 = operator_new(0x24);
    local_8 = 4;
    if (pvVar4 == (void *)0x0) {
      local_330 = 0;
    }
    else {
      local_330 = FUN_0054a9e0(0);
    }
    local_8 = 0xffffffff;
    param_1[0xe] = local_330;
    FUN_0054ac09(param_1[0xe]);
    local_128 = 0;
    for (local_12c = 0; local_12c < 4; local_12c = local_12c + 1) {
      if (*(char *)(local_94 + 0x10 + local_12c * 0x10) == '\0') {
        local_128 = 1;
        break;
      }
    }
    local_120 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x1205);
    if (local_128 == 0) {
      (**(code **)(*local_120 + 100))(0);
    }
    else {
      (**(code **)(*local_120 + 100))(1);
    }
    local_148 = FUN_0058f716(s_UI_selchar_txt_005b9368,&DAT_005b9364);
    if (local_148 != 0) {
      local_230[0] = &DAT_00a37df8;
      local_230[1] = &DAT_00a37e78;
      local_230[2] = &DAT_00a37ef8;
      local_230[3] = &DAT_00a37f78;
      for (local_12c = 0; local_12c < 4; local_12c = local_12c + 1) {
        local_144 = 0;
        local_258 = FUN_00590a68(local_148,s__d__d__d__d__d__d__d_005b9378,&local_124,&local_11c,
                                 &local_138,&local_144,&local_13c,&local_14c,local_140);
        local_234 = 0;
        _memset(&local_254,0,0x20);
        _memset(local_268,0,0x10);
        local_234 = (undefined2)local_124;
        local_254 = *(undefined2 *)(&DAT_00d44990 + local_124 * 0x8c);
        local_252 = *(undefined2 *)(&DAT_00d44992 + local_124 * 0x8c);
        local_250 = *(undefined2 *)(&DAT_00d44990 + local_11c * 0x8c);
        local_24e = *(undefined2 *)(&DAT_00d44992 + local_11c * 0x8c);
        local_24c = *(undefined2 *)(&DAT_00d44990 + local_138 * 0x8c);
        local_24a = *(undefined2 *)(&DAT_00d44992 + local_138 * 0x8c);
        local_248 = *(undefined2 *)(&DAT_00d44990 + local_138 * 0x8c);
        local_246 = *(undefined2 *)(&DAT_00d44992 + local_138 * 0x8c);
        local_244 = *(undefined2 *)(&DAT_00d44990 + local_138 * 0x8c);
        local_242 = *(undefined2 *)(&DAT_00d44992 + local_138 * 0x8c);
        local_240 = *(undefined2 *)(&DAT_00d44990 + local_138 * 0x8c);
        local_23e = *(undefined2 *)(&DAT_00d44992 + local_138 * 0x8c);
        local_23c = *(undefined2 *)(&DAT_00d44990 + local_13c * 0x8c);
        local_23a = *(undefined2 *)(&DAT_00d44992 + local_13c * 0x8c);
        local_238 = *(undefined2 *)(&DAT_00d44990 + local_14c * 0x8c);
        local_236 = *(undefined2 *)(&DAT_00d44992 + local_14c * 0x8c);
        local_261 = local_140[0];
        local_262 = local_140[0];
        local_263 = local_140[0];
        local_264 = local_140[0];
        local_265 = local_140[0];
        local_266 = local_140[0];
        local_25f = (&DAT_00d449da)[local_138 * 0x8c];
        local_25a = (&DAT_00d449da)[local_13c * 0x8c];
        local_259 = (&DAT_00d449da)[local_14c * 0x8c];
        local_25e = local_25f;
        local_25d = local_25f;
        local_25c = local_25f;
        local_25b = local_25f;
        if (local_258 == -1) break;
        pvVar4 = operator_new(0x894);
        local_8 = 5;
        if (pvVar4 == (void *)0x0) {
          local_334 = 0;
        }
        else {
          local_334 = FUN_004f7ea6(param_1);
        }
        local_8 = 0xffffffff;
        param_1[local_12c + 0x9b90] = local_334;
        FUN_0058f078(param_1[local_12c + 0x9b90] + 0x168,&DAT_005b9390,local_230[local_12c]);
        FUN_0058f220(param_1[local_12c + 0x9b90] + 0x1d2,&local_254,0x20);
        FUN_0058f220(param_1[local_12c + 0x9b90] + 0x1f2,local_268,0x10);
        _memset((void *)(param_1[local_12c + 0x9b90] + 0x202),0,0x10);
        if (0 < local_144) {
          *(undefined1 *)(param_1[local_12c + 0x9b90] + 0x79b) = 1;
          *(undefined2 *)(param_1[local_12c + 0x9b90] + 0x7aa) =
               *(undefined2 *)(&DAT_00d44992 + local_144 * 0x8c);
          *(undefined1 *)(param_1[local_12c + 0x9b90] + 0x7ac) = local_140[0];
          *(undefined *)(param_1[local_12c + 0x9b90] + 0x7ad) = (&DAT_00d449da)[local_144 * 0x8c];
        }
        *(undefined2 *)(param_1[local_12c + 0x9b90] + 0x450) = 1;
        FUN_004faf13(local_234);
        (**(code **)(*(int *)param_1[local_12c + 0x9b90] + 0x38))();
        FUN_0051bb41(CONCAT22(extraout_var_02,(undefined2)local_14c),
                     CONCAT22(extraout_var_00,(undefined2)local_13c));
        if (DAT_005b892c == 2) {
          (**(code **)(*(int *)param_1[local_12c + 0x9b90] + 0x40))(0,0xc2340000,0);
          CSimpleArray<>();
          CSimpleArray<>();
          CSimpleArray<>();
          uVar6 = FUN_00430f50(0x45022000,0,0x4501d333);
          FUN_004310a0(uVar6);
          uVar6 = FUN_00430f50(0x4501d333,0,0x45022000);
          FUN_004310a0(uVar6);
          FUN_004a8940(&local_28c,local_274,local_280,(float)local_12c * _DAT_005a4528);
          (**(code **)(*(int *)param_1[local_12c + 0x9b90] + 0x3c))(local_28c,local_288,local_284);
        }
        else {
          (**(code **)(*(int *)param_1[local_12c + 0x9b90] + 0x40))
                    (0,((float)-local_12c * _DAT_005a4148) / _DAT_005a4104,0);
          if (local_12c < 2) {
            (**(code **)(*(int *)param_1[local_12c + 0x9b90] + 0x3c))
                      (param_1[0x9bbb],0,
                       (float)local_12c * _DAT_005a4520 + ((float)param_1[0x9bbc] - _DAT_005a4524));
          }
          else {
            (**(code **)(*(int *)param_1[local_12c + 0x9b90] + 0x3c))
                      ((float)param_1[0x9bbb] - (float)(local_12c + -1) * _DAT_005a4520,0,
                       param_1[0x9bbc]);
          }
        }
        FUN_0054ac09(param_1[local_12c + 0x9b90]);
      }
      FUN_0058f62c(local_148);
    }
    if (DAT_005b2980 != 0) {
      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
        if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
          if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
            FUN_00493cf0(1);
          }
          *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
        }
        pvVar4 = operator_new(0x18);
        local_8 = 6;
        if (pvVar4 == (void *)0x0) {
          local_344 = 0;
        }
        else {
          local_344 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(DAT_013b71e0 + 0xec) = local_344;
        FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
        FUN_00428ba0(0);
      }
      DAT_005b2980 = 0;
    }
    uVar6 = 1;
  }
  ExceptionList = local_10;
  return uVar6;
}

