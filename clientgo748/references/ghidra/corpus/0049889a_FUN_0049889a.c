// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049889a | Name: FUN_0049889a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0049889a(int *param_1,undefined4 param_2,int param_3)

{
  bool bVar1;
  int *piVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  void *pvVar6;
  int iVar7;
  size_t sVar8;
  int local_3b0;
  undefined4 local_3ac;
  int local_3a8;
  int local_3a4;
  int local_3a0;
  int local_394;
  undefined1 local_31c [96];
  int local_2bc;
  int local_2b8;
  int *local_2b4;
  int local_2b0;
  undefined1 local_2ac [128];
  undefined1 local_22c [128];
  int local_1ac;
  int local_1a8;
  _SYSTEMTIME local_1a4;
  int *local_194;
  int local_190;
  int local_18c;
  int *local_188;
  int local_184;
  int local_180;
  int local_17c;
  int local_178;
  int *local_174;
  int local_16c;
  undefined1 local_168 [128];
  undefined1 local_e8 [128];
  int *local_68;
  int local_64;
  int local_60;
  uint local_5c;
  int local_58;
  int local_54;
  uint local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int *local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0160;
  local_10 = ExceptionList;
  if (DAT_0067cf38 != param_1) {
    return 0;
  }
  local_18 = param_3;
  ExceptionList = &local_10;
  local_14 = (**(code **)(*DAT_0092e654 + 8))();
  if ((local_18 == 0) && ((param_1[0x9b85] == 0 || (param_1[0x9b85] + 15000U < local_14)))) {
    iVar3 = FUN_0040c0f0();
    if (iVar3 == 0) {
      FUN_00403df2(&DAT_00a347f8,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    if (param_1[9] != 0x7532) {
      (**(code **)(*DAT_013b71e8 + 0x54))(7);
    }
    param_1[0x9b85] = 0;
    ExceptionList = local_10;
    return 1;
  }
  if (((local_18 == 0) && (local_14 <= param_1[0x9b85] + 15000U)) && (param_1[0x9b86] == 1)) {
    if (DAT_0067cf38[9] == 30000) {
      local_1c = DAT_0067cf38;
      (**(code **)(*DAT_0067cf38 + 4))(0x52a,DAT_0067cf38 + 0xa2ef);
    }
    param_1[0x9b85] = 0;
    param_1[0x9b86] = 0;
    ExceptionList = local_10;
    return 1;
  }
  if (local_18 == 0) {
    iVar3 = FUN_0040c0f0();
    if (iVar3 == 0) {
      FUN_00403df2(&DAT_00a347f8,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    if (param_1[9] != 0x7532) {
      (**(code **)(*DAT_013b71e8 + 0x54))(7);
    }
    param_1[0x9b85] = 0;
    ExceptionList = local_10;
    return 1;
  }
  if (*(short *)(local_18 + 4) == 0x194) {
    DAT_013b71e8[0x6c24] = 1;
    (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a38378,0x1202,0);
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  if (*(short *)(local_18 + 4) == 0xbbf) {
    local_24 = local_18;
    local_2c = *(int *)(local_18 + 0xac);
    local_28 = *(int *)(local_18 + 0xb0);
    local_20 = ((local_2c / 7) * 3 + 0xdc + (local_28 / 3) * 7) % 3;
    if (local_20 == 0) {
      *(undefined2 *)(local_18 + 4) = 0xfbc;
    }
    else if (local_20 == 1) {
      *(undefined2 *)(local_18 + 4) = 0x13bd;
    }
    else if (local_20 == 2) {
      *(undefined2 *)(local_18 + 4) = 0x7be;
    }
  }
  if (*(short *)(local_18 + 4) == 0xfbc) {
    local_34 = local_18;
    local_30 = (int)(*(uint *)(local_18 + 0x68) ^ 0xffffffff) % 0x28;
    if (local_30 == 0x17) {
      local_30 = 0x16;
    }
    _DAT_005ccf88 = *(uint *)(local_18 + 0xc + local_30 * 4);
  }
  if (*(short *)(local_18 + 4) == 0x13bd) {
    local_40 = local_18;
    local_3c = *(int *)(local_18 + 0x38);
    local_38 = *(int *)(local_18 + 0x88);
    local_44 = (local_3c * local_38 + local_3c) % 400;
    _DAT_005ccf88 = *(uint *)(&DAT_005c12b0 + local_44 * 4) ^ 0xffff;
    iVar3 = FUN_004ff388();
    if (iVar3 == 2) {
      _DAT_005ccf88 = *(uint *)(&DAT_005c18fc + local_44 * 4) ^ 0xffff;
    }
  }
  iVar3 = local_18;
  if (*(short *)(local_18 + 4) == 0x7be) {
    local_58 = local_18;
    local_50 = *(uint *)(local_18 + 0x84);
    local_54 = (int)local_50 % 0x1e;
    local_4c = *(int *)(local_18 + 0xc + local_54 * 4);
    local_5c = local_50 & 0x80000003;
    if ((int)local_5c < 0) {
      local_5c = (local_5c - 1 | 0xfffffffc) + 1;
    }
    local_48 = local_4c % 5;
    _DAT_005ccf88 = *(uint *)(&DAT_005be888 + local_48 * 4 + local_5c * 0x14);
  }
  if ((*(short *)(local_18 + 6) != 0) ||
     (((*(short *)(local_18 + 4) != 0x101 && (*(short *)(local_18 + 4) != 0x102)) &&
      (*(short *)(local_18 + 4) != 0x104)))) {
    if (*(short *)(local_18 + 4) != 0xd1d) {
      if ((param_1[10] != 0) &&
         (iVar3 = (**(code **)(*(int *)param_1[10] + 4))(param_2,param_3), iVar3 == 1)) {
        ExceptionList = local_10;
        return 1;
      }
      FUN_0054ab2a(param_2,param_3);
      ExceptionList = local_10;
      return 0;
    }
    *(undefined1 *)(local_18 + 0x6b) = 0;
    *(undefined1 *)(local_18 + 0x6a) = 0;
    piVar2 = DAT_0067cf38;
    if ((((DAT_0067cf38[0x9ebd] != 0) && (iVar7 = FUN_00494dcf(), iVar7 == 30000)) &&
        (DAT_005b892c == 2)) && (*(int *)(piVar2[0x9ebd] + 0x1e8) == 0)) {
      ExceptionList = local_10;
      return 1;
    }
    iVar7 = piVar2[0x9eba];
    sVar8 = _strlen((char *)(iVar3 + 0xc));
    pvVar6 = operator_new(0xe50);
    local_8 = 5;
    if (pvVar6 == (void *)0x0) {
      local_3b0 = 0;
    }
    else {
      local_3b0 = FUN_00407203(iVar3 + 0xc,0xff00cd00,0,0,(float)(int)sVar8 * _DAT_005a44a4,
                               0x41800000,0,0xffffff00,1,0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(local_3b0 + 0xe4c) = 0;
    if ((local_3b0 != 0) && (iVar7 != 0)) {
      FUN_00408d33(local_3b0);
    }
    iVar3 = (**(code **)(*DAT_0092e654 + 8))();
    piVar2[0x9d19] = iVar3;
    ExceptionList = local_10;
    return 1;
  }
  if (*(short *)(local_18 + 4) != 0x101) {
    ExceptionList = local_10;
    return 1;
  }
  local_60 = local_18;
  *(undefined1 *)(local_18 + 0x6b) = 0;
  *(undefined1 *)(local_18 + 0x6a) = 0;
  piVar2 = DAT_0067cf38;
  if ((param_1[9] == 0x7531) && (*(char *)(local_18 + 0xc) == '^')) {
    _memset(local_e8,0,0x80);
    FUN_0058f078(local_e8,&DAT_005b901c,local_60 + 0xd);
    (**(code **)(*(int *)param_1[0x24] + 0x8c))(local_e8,0,0);
    (**(code **)(*(int *)param_1[0x24] + 0x60))(1);
    goto LAB_00499a74;
  }
  if (*(char *)(local_18 + 0xc) == '^') {
    _memset(local_168,0,0x80);
    FUN_0058f078(local_168,&DAT_005b9020,local_60 + 0xd);
    FUN_00403df2(local_60 + 0xc,7000);
    (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    goto LAB_00499a74;
  }
  if ((*(char *)(local_18 + 0xc) == '!') && (*(char *)(local_18 + 0xd) == '#')) {
    for (local_16c = 2; local_16c < 6; local_16c = local_16c + 1) {
      if ((*(char *)(local_18 + local_16c + 0xc) < '0') ||
         ('9' < *(char *)(local_18 + local_16c + 0xc))) {
        *(undefined1 *)(local_18 + local_16c + 0xc) = 0x30;
      }
    }
    piVar2[0x9da9] =
         ((int)*(char *)(local_18 + 0xe) - 0x30U & 0xffff) * 10 +
         ((int)*(char *)(local_18 + 0xf) - 0x30U & 0xffff);
    piVar2[0x9daa] =
         ((int)*(char *)(local_18 + 0x10) - 0x30U & 0xffff) * 100 +
         ((int)*(char *)(local_18 + 0x11) - 0x30U & 0xffff) * 10 +
         ((int)*(char *)(local_18 + 0x12) - 0x30U & 0xffff);
    ExceptionList = local_10;
    return 1;
  }
  if (((*(char *)(local_18 + 0xd) != '!') || (*(char *)(local_18 + 0xe) != '!')) ||
     (*(char *)(local_18 + 0xf) != '!')) {
    if (((*(char *)(local_18 + 0xd) == '!') && (*(char *)(local_18 + 0xe) == '!')) &&
       (*(char *)(local_18 + 0xf) == '#')) {
      if ((DAT_0067cf38[9] == 30000) && (*(char *)(local_18 + 0x10) == 'E')) {
        local_188 = DAT_0067cf38;
        if (DAT_0067cf38[0x9daf] != 0) {
          local_18c = (**(code **)(*DAT_013b71e8 + 0x34))(DAT_0067cf38[0x9daf]);
          if (local_18c != 0) {
            *(undefined2 *)(local_18c + 0x450) = 0;
            FUN_0052355c();
          }
          local_188[0x9daf] = 0;
        }
        for (local_184 = 0; local_184 < 5; local_184 = local_184 + 1) {
          *(undefined1 *)(local_60 + local_184 + 0xc) = 0x5f;
        }
      }
    }
    else if ((*(char *)(local_18 + 0xc) == '!') && (*(char *)(local_18 + 0xd) == '!')) {
      if (DAT_0067cf38[9] == 30000) {
        local_194 = DAT_0067cf38;
        for (local_190 = 2; local_190 < 8; local_190 = local_190 + 1) {
          if ((*(char *)(local_18 + local_190 + 0xc) < '0') ||
             ('9' < *(char *)(local_18 + local_190 + 0xc))) {
            *(undefined1 *)(local_18 + local_190 + 0xc) = 0x30;
          }
        }
        *(short *)(piVar2 + 0x9da5) =
             (*(char *)(local_18 + 0xe) + -0x30) * 10 + *(char *)(local_18 + 0xf) + -0x30;
        *(short *)((int)piVar2 + 0x2768e) =
             (*(char *)(local_18 + 0x10) + -0x30) * 10 + *(char *)(local_18 + 0x11) + -0x30;
        *(short *)(piVar2 + 0x9da6) =
             (*(char *)(local_18 + 0x12) + -0x30) * 10 + *(char *)(local_18 + 0x13) + -0x30;
        iVar3 = (**(code **)(*DAT_0092e654 + 8))();
        local_194[0x9da7] = iVar3;
        ExceptionList = local_10;
        return 1;
      }
    }
    else if (*(char *)(local_18 + 0xc) == '!') {
      GetLocalTime(&local_1a4);
      if (DAT_0067cf38[9] == 30000) {
        local_2b4 = DAT_0067cf38;
        pvVar6 = operator_new(0xe50);
        local_8 = 1;
        if (pvVar6 == (void *)0x0) {
          local_3a0 = 0;
        }
        else {
          local_3a0 = FUN_00407203(&DAT_005b9024,0xffffffff,0,0,0x43960000,0x41800000,0,0x77777777,1
                                   ,0);
        }
        local_8 = 0xffffffff;
        local_1ac = local_3a0;
        if (local_3a0 != 0) {
          FUN_00408d33(local_3a0);
        }
        FUN_0058f078(local_22c,s_SMS___02d__02d__02d__005b9028,local_1a4.wHour,local_1a4.wMinute,
                     local_1a4.wSecond);
        pvVar6 = operator_new(0xe50);
        local_8 = 2;
        if (pvVar6 == (void *)0x0) {
          local_3a4 = 0;
        }
        else {
          local_3a4 = FUN_00407203(local_22c,0xffbbffff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        local_2b0 = local_3a4;
        if (local_3a4 != 0) {
          FUN_00408d33(local_3a4);
        }
        FUN_0058f078(local_2ac,&DAT_005b9040,local_60 + 0xd);
        pvVar6 = operator_new(0xe50);
        local_8 = 3;
        if (pvVar6 == (void *)0x0) {
          local_3a8 = 0;
        }
        else {
          local_3a8 = FUN_00407203(local_2ac,0xffbbffcc,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        local_1a8 = local_3a8;
        if (local_3a8 != 0) {
          FUN_00408d33(local_3a8);
        }
        if (local_2b4[0x9f3c] != 0) {
          (**(code **)(*(int *)local_2b4[0x9f3c] + 0x60))(1);
        }
      }
      else {
        for (local_2b8 = 0; local_2b8 < 0x62; local_2b8 = local_2b8 + 1) {
          if (((char)DAT_013b71e8[local_2b8 * 0x21 + 0x6c26] == '\0') &&
             ((char)DAT_013b71e8[(local_2b8 + 1) * 0x21 + 0x6c26] == '\0')) {
            DAT_013b71e8[local_2b8 * 0x21 + 0x6c25] = -1;
            FUN_0058f078(DAT_013b71e8 + local_2b8 * 0x21 + 0x6c26,&DAT_005b9044);
            DAT_013b71e8[(local_2b8 + 1) * 0x21 + 0x6c25] = -0x440001;
            FUN_0058f078(DAT_013b71e8 + (local_2b8 + 1) * 0x21 + 0x6c26,
                         s_SMS___02d__02d__02d__005b9048,local_1a4.wHour,local_1a4.wMinute,
                         local_1a4.wSecond);
            DAT_013b71e8[(local_2b8 + 2) * 0x21 + 0x6c25] = -0x440034;
            FUN_0058f078(DAT_013b71e8 + (local_2b8 + 2) * 0x21 + 0x6c26,&DAT_005b9060,local_60 + 0xd
                        );
            break;
          }
        }
      }
      FUN_00403df2(local_60 + 0xd,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    else if (((*(char *)(local_18 + 0xc) == '2') && (*(char *)(local_18 + 0xd) == '0')) &&
            (*(char *)(local_18 + 0xe) == '0')) {
      local_2bc = param_1[0x13];
      if (local_2bc == 0) {
        FUN_0058f078(local_31c,s__s__d_005b9070,local_18 + 0xc,DAT_013b71e8[0x6bd4]);
      }
      else {
        uVar5 = __ftol();
        uVar5 = __ftol(uVar5);
        FUN_0058f078(local_31c,s__s__d__d__d_005b9064,local_60 + 0xc,DAT_013b71e8[0x6bd4],uVar5);
      }
      FUN_00403df2(local_31c,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    else {
      FUN_00403df2(local_18 + 0xc,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    goto LAB_00499a74;
  }
  local_174 = (int *)param_1[0x13];
  iVar3 = __ftol();
  if ((iVar3 >> 7 == 0x1f) && (iVar3 = __ftol(), iVar3 >> 7 == 0x1f)) {
    bVar1 = true;
  }
  else {
    bVar1 = false;
  }
  if (bVar1) {
    for (local_178 = 0; local_178 < 5; local_178 = local_178 + 1) {
      for (local_17c = 0; local_17c < 5; local_17c = local_17c + 1) {
        pvVar6 = operator_new(0x1eac);
        local_8 = 0;
        if (pvVar6 == (void *)0x0) {
          local_394 = 0;
        }
        else {
          iVar3 = local_178;
          puVar4 = (undefined4 *)
                   FUN_00430f50(((float)local_174[10] - _DAT_005a3670) + (float)(local_178 * 5),
                                0x40e00000,
                                ((float)local_174[0xb] - _DAT_005a3670) + (float)(local_17c * 5));
          local_394 = FUN_004d01b0(*puVar4,puVar4[1],puVar4[2],iVar3);
        }
        local_8 = 0xffffffff;
        local_180 = local_394;
        if (local_394 != 0) {
          FUN_0054ac09(local_394);
        }
      }
    }
  }
  iVar3 = (**(code **)(*local_174 + 0x50))();
  if (iVar3 == 1) {
LAB_00499223:
    bVar1 = true;
  }
  else {
    iVar3 = __ftol();
    if ((iVar3 >> 7 == 0x1f) && (iVar3 = __ftol(), iVar3 >> 7 == 0x1f)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (bVar1) goto LAB_00499223;
    bVar1 = false;
  }
  if (bVar1) {
    FUN_00403df2(local_60 + 0xc,4000);
    (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
  }
LAB_00499a74:
  local_64 = 0;
  local_64 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1501);
  if (local_64 != 0) {
    pvVar6 = operator_new(0xe50);
    local_8 = 4;
    if (pvVar6 == (void *)0x0) {
      local_3ac = 0;
    }
    else {
      local_3ac = FUN_00407203(local_60 + 0xc,0xffccaaff,0,0,0x43960000,0x41800000,0,0x77777777,1,0)
      ;
    }
    local_8 = 0xffffffff;
    FUN_00408d33(local_3ac);
    iVar3 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x9d19] = iVar3;
  }
  if (DAT_0067cf38[9] == 30000) {
    iVar3 = _strcmp((char *)(local_60 + 0xc),s_Whisper___Off_005b9078);
    if (iVar3 == 0) {
      FUN_0044edbc(0);
    }
    iVar3 = _strcmp((char *)(local_60 + 0xc),s_Whisper___On_005b9088);
    if (iVar3 == 0) {
      FUN_0044edbc(1);
    }
    iVar3 = _strcmp((char *)(local_60 + 0xc),s_Party_Chatting___Off_005b9098);
    if (iVar3 == 0) {
      FUN_0044ee1f(0);
    }
    iVar3 = _strcmp((char *)(local_60 + 0xc),s_Party_Chatting___On_005b90b0);
    if (iVar3 == 0) {
      FUN_0044ee1f(1);
    }
    iVar3 = _strcmp((char *)(local_60 + 0xc),s_Guild_Chatting___Off_005b90c4);
    if (iVar3 == 0) {
      FUN_0044ee77(0);
    }
    iVar3 = _strcmp((char *)(local_60 + 0xc),s_Guild_Chatting___On_005b90dc);
    if (iVar3 == 0) {
      FUN_0044ee77(1);
    }
  }
  local_68 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x1201);
  if (local_68 != (int *)0x0) {
    (**(code **)(*local_68 + 100))(1);
  }
  ExceptionList = local_10;
  return 1;
}

