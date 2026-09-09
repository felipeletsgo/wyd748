// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005284a0 | Name: FUN_005284a0


/* WARNING: Removing unreachable block (ram,0x00528c15) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005284a0(int *param_1,int param_2)

{
  int iVar1;
  undefined1 uVar2;
  void *pvVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  bool bVar8;
  bool bVar9;
  float fVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  int local_1a4;
  int local_194;
  int local_18c;
  int local_17c;
  int local_170;
  int local_160;
  int local_140;
  int *local_13c;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_64;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  iVar1 = DAT_0067cf38;
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a22a9;
  local_10 = ExceptionList;
  if ((*(char *)((int)param_1 + 0x795) == '\0') && (*(short *)(param_2 + 4) == 0x368)) {
    ExceptionList = &local_10;
    *(short *)(DAT_013b71e8 + 0x73a) = *(short *)(DAT_013b71e8 + 0x73a) - (short)DAT_00931a60;
    local_2c = 0;
    if ((param_1[0x93] == 4) || (param_1[0x93] == 8)) {
      local_2c = 1;
    }
    pvVar3 = operator_new(0x22c);
    local_8 = 0;
    if (pvVar3 == (void *)0x0) {
      local_13c = (int *)0x0;
    }
    else {
      uVar15 = 0;
      uVar14 = 0;
      puVar4 = (undefined4 *)FUN_00430f50(0,0,0);
      uVar11 = *puVar4;
      uVar12 = puVar4[1];
      uVar13 = puVar4[2];
      puVar4 = (undefined4 *)FUN_00430f50(0,0,0);
      local_13c = (int *)FUN_004d5480(param_1[0x17],*puVar4,puVar4[1],puVar4[2],uVar11,uVar12,uVar13
                                      ,uVar14,uVar15);
    }
    local_8 = 0xffffffff;
    if (local_13c != (int *)0x0) {
      if (((char)param_1[0x1e7] < '\x01') || (param_1[0x67] == 0)) {
        FUN_0058f220(local_13c + 0x58,(int)param_1 + 0x1d2,0x20);
      }
      else {
        local_13c[0x17] = param_1[0x1e8];
        FUN_0058f220(local_13c + 0x58,(int)param_1 + 0x1a2,0x20);
        local_13c[0x88] = param_1[0x17];
        FUN_0058f220(local_13c + 0x7f,(int)param_1 + 0x1d2,0x20);
      }
      local_13c[0x69] = 0x3f000000;
      local_13c[0x6a] = 0x3f000000;
      local_13c[0x6b] = 0x3f000000;
      local_13c[0xe] = param_1[0xe];
      (**(code **)(*local_13c + 0x78))(local_2c);
      local_13c[0x75] = 1;
      local_13c[0x65] = 3000;
      (**(code **)(*local_13c + 0x3c))(param_1[10],(float)param_1[0xc] + _DAT_005a4214,param_1[0xb])
      ;
      if (((char)param_1[0x1e7] < '\x01') || (param_1[0x67] == 0)) {
        FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                    *(int *)(&DAT_00672490 +
                                            param_1[0x1d0] * 4 + param_1[0x17] * 0x2a0) * 2 +
                                    param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
      }
      else {
        if (local_13c[0x18] != 0) {
          FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                      *(int *)(&DAT_00672490 +
                                              param_1[0x1d0] * 4 + param_1[0x1e8] * 0x2a0) * 2 +
                                      param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
        }
        if (local_13c[0x87] != 0) {
          FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                      *(int *)(&DAT_00672500 +
                                              param_1[0x1d0] * 4 + param_1[0x17] * 0x2a0) * 2 +
                                      param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
        }
      }
      local_13c[0x68] = param_1[0xd];
      local_13c[0x7d] = local_13c[0x68];
      if (param_1[0x58] != 0x65) {
        local_13c[0x7d] = (int)((float)param_1[0xd] + _DAT_005a4290);
      }
      local_13c[0x66] = 1;
      local_13c[0x67] = 0;
      FUN_0054ac09(local_13c);
    }
  }
  else {
    ExceptionList = &local_10;
    if ((*(char *)((int)param_1 + 0x795) == '\0') &&
       (ExceptionList = &local_10, *(int *)(param_2 + 0x14) != 5)) {
      local_30 = 1;
      if (*(int *)(param_2 + 0x14) == 6) {
        local_30 = 2;
      }
      ExceptionList = &local_10;
      pvVar3 = operator_new(0xac);
      local_8 = 1;
      if (pvVar3 == (void *)0x0) {
        local_140 = 0;
      }
      else {
        puVar4 = (undefined4 *)
                 FUN_00430f50(param_1[10],(float)param_1[0xc] + _DAT_005a439c,param_1[0xb]);
        local_140 = FUN_004ee730(*puVar4,puVar4[1],puVar4[2],local_30);
      }
      local_8 = 0xffffffff;
      if (local_140 != 0) {
        FUN_0054ac09(local_140);
      }
    }
  }
  iVar7 = DAT_0067cf38;
  if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
    iVar6 = *(int *)(DAT_013b71e8 + 0x1b084);
    bVar8 = (uint)((int)(uint)*(ushort *)(param_2 + 0x1a) >> 7) < 0x1a;
    if (bVar8) {
      *(undefined4 *)(DAT_0067cf38 + 0x28e74) = 0;
    }
    else {
      *(undefined4 *)(DAT_0067cf38 + 0x28e74) = 1;
    }
    iVar5 = __ftol();
    bVar9 = 0x19 < iVar5 >> 7;
    if ((bVar8) || (bVar9)) {
      if ((bVar9) && (bVar8)) {
        *(undefined4 *)(iVar6 + 0xc0) = 0x41700000;
        *(float *)(iVar6 + 0x34) = *(float *)(iVar6 + 0x34) + _DAT_005a3834;
        if (_DAT_005a3970 < *(float *)(iVar6 + 0x34)) {
          *(undefined4 *)(iVar6 + 0x34) = 0x41700000;
        }
      }
    }
    else {
      *(undefined4 *)(iVar6 + 0xc0) = 0x41300000;
      if (_DAT_005a4394 < *(float *)(iVar6 + 0x34)) {
        *(undefined4 *)(iVar6 + 0x34) = 0x41300000;
      }
    }
    if (*(int *)(iVar7 + 0x24) == 30000) {
      iVar6 = __ftol();
      iVar5 = __ftol();
      if (((((iVar6 >> 7 == 0x19) && (iVar5 >> 7 == 0xd)) &&
           (0x19 < (uint)((int)(uint)*(ushort *)(param_2 + 0x18) >> 7))) &&
          (((uint)((int)(uint)*(ushort *)(param_2 + 0x18) >> 7) < 0x1f &&
           (7 < (uint)((int)(uint)*(ushort *)(param_2 + 0x1a) >> 7))))) &&
         ((uint)((int)(uint)*(ushort *)(param_2 + 0x1a) >> 7) < 0xd)) {
        bVar8 = true;
      }
      else {
        bVar8 = false;
      }
      if (bVar8) {
        FUN_00465e42(1);
      }
      *(uint *)(iVar7 + 0x275b0) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(iVar7 + 0x275b4) = (uint)*(ushort *)(param_2 + 0x1a);
      *(uint *)(iVar7 + 0x28794) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(iVar7 + 0x28798) = (uint)*(ushort *)(param_2 + 0x1a);
    }
    param_1[0x121] = (uint)*(ushort *)(param_2 + 0x18);
    param_1[0x122] = (uint)*(ushort *)(param_2 + 0x1a);
  }
  FUN_00430f20((float)*(ushort *)(param_2 + 0x18) + _DAT_005a34a0,
               (float)*(ushort *)(param_2 + 0x1a) + _DAT_005a34a0);
  uVar11 = local_14;
  iVar7 = FUN_0049c70f(local_18,local_14);
  (**(code **)(*param_1 + 0x3c))(local_18,(float)iVar7 * _DAT_005a4214,uVar11);
  if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
    if (*(int *)(iVar1 + 0x24) == 30000) {
      *(uint *)(iVar1 + 0x275b0) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(iVar1 + 0x275b4) = (uint)*(ushort *)(param_2 + 0x1a);
      *(uint *)(iVar1 + 0x28794) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(iVar1 + 0x28798) = (uint)*(ushort *)(param_2 + 0x1a);
    }
  }
  else {
    param_1[0x121] = (uint)*(ushort *)(param_2 + 0x18);
    param_1[0x122] = (uint)*(ushort *)(param_2 + 0x1a);
  }
  *(undefined1 *)((int)param_1 + 0x245) = 0;
  if (((*(char *)((int)param_1 + 0x795) == '\0') && (*(int *)(param_2 + 0x14) != 5)) &&
     (*(short *)(param_2 + 4) != 0x368)) {
    if ((((param_1[0x93] == 1) || (param_1[0x93] == 2)) || (param_1[0x93] == 4)) ||
       ((param_1[0x93] == 8 || (param_1[0x93] == 0x1a)))) {
      pvVar3 = operator_new(0x94);
      local_8 = 2;
      if (pvVar3 == (void *)0x0) {
        local_160 = 0;
      }
      else {
        uVar12 = 0;
        uVar11 = 0;
        fVar10 = (float)*(ushort *)(param_2 + 0x1a) + _DAT_005a34a0;
        iVar7 = FUN_0049c70f(param_1[10],param_1[0xb]);
        puVar4 = (undefined4 *)
                 FUN_00430f50((float)*(ushort *)(param_2 + 0x18) + _DAT_005a34a0,
                              (float)iVar7 * _DAT_005a4214 + _DAT_005a439c,fVar10);
        local_160 = FUN_004d8ef0(*puVar4,puVar4[1],puVar4[2],uVar11,uVar12);
      }
      local_8 = 0xffffffff;
      if (local_160 != 0) {
        FUN_0054ac09(local_160);
      }
    }
    else {
      pvVar3 = operator_new(0x94);
      local_8 = 3;
      if (pvVar3 == (void *)0x0) {
        local_170 = 0;
      }
      else {
        uVar12 = 0;
        uVar11 = 1;
        fVar10 = (float)*(ushort *)(param_2 + 0x1a) + _DAT_005a34a0;
        iVar7 = FUN_0049c70f(param_1[10],param_1[0xb]);
        puVar4 = (undefined4 *)
                 FUN_00430f50((float)*(ushort *)(param_2 + 0x18) + _DAT_005a34a0,
                              (float)iVar7 * _DAT_005a4214 + _DAT_005a439c,fVar10);
        local_170 = FUN_004d8ef0(*puVar4,puVar4[1],puVar4[2],uVar11,uVar12);
      }
      local_8 = 0xffffffff;
      if (local_170 != 0) {
        FUN_0054ac09(local_170);
      }
    }
    if (((DAT_005ccf98 != 0) && (*(int **)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54) == param_1)) &&
       (iVar7 = FUN_00429a6d(0x97), iVar7 != 0)) {
      uVar12 = 0;
      uVar11 = 0;
      FUN_00429a6d(0x97);
      FUN_0042ad2b(uVar11,uVar12);
    }
    local_64 = 1;
    if (*(int *)(param_2 + 0x14) == 6) {
      local_64 = 2;
    }
    if (((*(int *)(param_2 + 0x14) == 6) && (param_1[0x93] == 0x3e)) &&
       (*(short *)((int)param_1 + 0x1d2) == 2)) {
      iVar7 = param_1[0xb];
      iVar6 = FUN_0049c70f(param_1[10],param_1[0xb]);
      (**(code **)(*param_1 + 0x3c))(param_1[10],(float)iVar6 * _DAT_005a4214 - _DAT_005a365c,iVar7)
      ;
      iVar7 = param_1[0xb];
      iVar6 = FUN_0049c70f(param_1[10],param_1[0xb]);
      FUN_00430f50(param_1[10],(float)iVar6 * _DAT_005a4214 + _DAT_005a4158,iVar7);
      pvVar3 = operator_new(0x94);
      local_8 = 4;
      if (pvVar3 == (void *)0x0) {
        local_17c = 0;
      }
      else {
        local_17c = FUN_004e6590(local_7c,local_78,local_74,4,0x3dcccccd);
      }
      local_8 = 0xffffffff;
      if (local_17c != 0) {
        FUN_0054ac09(local_17c);
      }
    }
    else {
      pvVar3 = operator_new(0xac);
      local_8 = 5;
      if (pvVar3 == (void *)0x0) {
        local_18c = 0;
      }
      else {
        fVar10 = (float)*(ushort *)(param_2 + 0x1a) + _DAT_005a34a0;
        iVar7 = FUN_0049c70f(param_1[10],param_1[0xb]);
        puVar4 = (undefined4 *)
                 FUN_00430f50((float)*(ushort *)(param_2 + 0x18) + _DAT_005a34a0,
                              (float)iVar7 * _DAT_005a4214 + _DAT_005a439c,fVar10);
        local_18c = FUN_004ee730(*puVar4,puVar4[1],puVar4[2],local_64);
      }
      local_8 = 0xffffffff;
      if (local_18c != 0) {
        FUN_0054ac09(local_18c);
      }
    }
  }
  if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
    iVar7 = *(int *)(DAT_013b71e8 + 0x1b084);
    uVar11 = *(undefined4 *)(iVar7 + 0x28);
    FUN_00499ea6();
    if (*(short *)(param_2 + 4) == 0x368) {
      *(undefined4 *)(iVar7 + 0x28) = uVar11;
    }
    if (*(int *)(iVar1 + 0x24) == 30000) {
      uVar2 = FUN_00433a60(*(undefined2 *)(param_2 + 0x18),*(undefined2 *)(param_2 + 0x1a));
      *(undefined1 *)(iVar1 + 0x26ea8) = uVar2;
    }
    iVar7 = FUN_0049c70f(local_18,local_14);
    (**(code **)(*param_1 + 0x3c))(local_18,(float)iVar7 * _DAT_005a4214,local_14);
  }
  if (*(short *)(param_2 + 4) == 0x368) {
    FUN_0051dba4(1,0);
    if (((DAT_005ccf98 != 0) && (*(int **)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54) == param_1)) &&
       (iVar7 = FUN_00429a6d(0xaf), iVar7 != 0)) {
      uVar12 = 0;
      uVar11 = 0;
      FUN_00429a6d(0xaf);
      FUN_0042ad2b(uVar11,uVar12);
    }
  }
  else {
    FUN_0051dba4(0xe,0);
    if (*(char *)((int)param_1 + 0x795) == '\0') {
      pvVar3 = operator_new(0x90);
      local_8 = 6;
      if (pvVar3 == (void *)0x0) {
        local_194 = 0;
      }
      else {
        uVar11 = 0;
        puVar4 = (undefined4 *)FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
        local_194 = FUN_004d19a0(*puVar4,puVar4[1],puVar4[2],uVar11);
      }
      local_8 = 0xffffffff;
      if (local_194 != 0) {
        FUN_0054ac09(local_194);
      }
      if (*(char *)((int)param_1 + 0x249) == '\0') {
        pvVar3 = operator_new(0xac);
        local_8 = 7;
        if (pvVar3 == (void *)0x0) {
          local_1a4 = 0;
        }
        else {
          uVar11 = 1;
          fVar10 = (float)*(ushort *)(param_2 + 0x1a) + _DAT_005a34a0;
          iVar7 = FUN_0049c70f(param_1[10],param_1[0xb]);
          puVar4 = (undefined4 *)
                   FUN_00430f50((float)*(ushort *)(param_2 + 0x18) + _DAT_005a34a0,
                                (float)iVar7 * _DAT_005a4214 + _DAT_005a439c,fVar10);
          local_1a4 = FUN_004ee730(*puVar4,puVar4[1],puVar4[2],uVar11);
        }
        local_8 = 0xffffffff;
        if (local_1a4 != 0) {
          FUN_0054ac09(local_1a4);
        }
      }
    }
  }
  param_1[0x222] = 0;
  if (((iVar1 != 0) && (*(int **)(iVar1 + 0x4c) == param_1)) && (*(int *)(iVar1 + 0x26e1c) == 1)) {
    FUN_0047fa1d();
  }
  ExceptionList = local_10;
  return 1;
}

