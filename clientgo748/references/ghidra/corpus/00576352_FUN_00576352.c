// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00576352 | Name: FUN_00576352


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00576352(ushort *param_1,float param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int in_EAX;
  float *pfVar5;
  undefined4 uVar6;
  uint uVar7;
  uint extraout_EDX;
  uint extraout_EDX_00;
  uint uVar8;
  ushort uVar9;
  int iVar10;
  ushort uVar11;
  int iVar12;
  float local_2d4 [64];
  float local_1d4;
  float local_1d0;
  undefined1 local_1cc [4];
  undefined1 local_1c8 [4];
  undefined1 local_1c4 [4];
  undefined1 local_1c0 [20];
  undefined1 local_1ac [4];
  undefined1 local_1a8 [4];
  undefined1 local_1a4 [4];
  undefined1 local_1a0 [4];
  undefined1 local_19c [4];
  undefined1 local_198 [4];
  undefined1 local_194 [4];
  float local_190;
  undefined1 local_18c [4];
  undefined1 local_188 [4];
  undefined1 local_184 [4];
  undefined1 local_180 [172];
  float local_d4;
  float local_d0;
  float local_cc;
  int local_c4;
  undefined1 *local_c0;
  undefined1 *local_bc;
  undefined1 *local_b8;
  undefined1 *local_b4;
  undefined1 *local_b0;
  undefined1 *local_ac;
  int local_a8;
  undefined1 *local_a4;
  undefined1 *local_a0;
  float local_9c [4];
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  undefined1 *local_5c;
  undefined1 *local_58;
  undefined1 *local_54;
  undefined1 *local_50;
  float local_4c;
  float local_48;
  float local_44;
  float fStack_40;
  float local_3c;
  float local_38;
  float local_34;
  undefined1 *local_2c;
  undefined1 *local_28;
  undefined *local_24;
  float local_20;
  float local_1c;
  float local_18;
  float fStack_14;
  int local_10;
  float *local_c;
  float local_8;
  
  if (param_2 == 0.0) {
    local_10 = 4;
  }
  else {
    iVar10 = 0;
    pfVar5 = (float *)(in_EAX + 0xc);
    iVar12 = 0x10;
    do {
      if (*pfVar5 < _DAT_005a67d4) {
        iVar10 = iVar10 + 1;
      }
      pfVar5 = pfVar5 + 4;
      iVar12 = iVar12 + -1;
    } while (iVar12 != 0);
    if (iVar10 == 0x10) {
      param_1[1] = 0xffff;
      param_1[2] = 0xffff;
      param_1[3] = 0xffff;
      *param_1 = 0;
      return 0;
    }
    local_10 = 4 - (uint)(iVar10 != 0);
  }
  pfVar5 = &local_1d4;
  for (iVar12 = 0x40; iVar12 != 0; iVar12 = iVar12 + -1) {
    *pfVar5 = 0.0;
    pfVar5 = pfVar5 + 1;
  }
  FUN_005759c5();
  local_58 = local_1cc + -in_EAX;
  local_b8 = local_1c8 + -in_EAX;
  local_c0 = local_1c4 + -in_EAX;
  local_5c = local_1ac + -in_EAX;
  local_b4 = local_1a8 + -in_EAX;
  local_ac = local_1a4 + -in_EAX;
  local_28 = local_19c + -in_EAX;
  local_bc = local_198 + -in_EAX;
  local_a4 = local_194 + -in_EAX;
  local_2c = local_18c + -in_EAX;
  local_a0 = local_188 + -in_EAX;
  local_b0 = local_184 + -in_EAX;
  pfVar5 = (float *)(in_EAX + 8);
  local_a8 = (int)&local_1d4 - in_EAX;
  uVar8 = 0;
  param_2 = (float)extraout_EDX;
  do {
    local_20 = *(float *)((int)&local_1d4 + uVar8) + pfVar5[-2];
    fVar1 = pfVar5[-1];
    fVar2 = *(float *)(local_1cc + (uVar8 - 4));
    local_1c = fVar1 + fVar2;
    local_18 = *(float *)(local_a8 + (int)pfVar5) + *pfVar5;
    local_c4 = (int)ROUND(local_20 * _DAT_005a6974 + _DAT_005a67d4);
    local_24 = (undefined *)((float)local_c4 * _DAT_005a69a4);
    *(undefined **)((int)local_2d4 + uVar8) = local_24;
    local_50 = (undefined1 *)(int)ROUND((fVar1 + fVar2) * _DAT_005a6970 + _DAT_005a67d4);
    local_8 = (float)(int)local_50 * _DAT_005a69a0;
    *(float *)((int)local_2d4 + uVar8 + 4) = local_8;
    local_54 = (undefined1 *)(int)ROUND(local_18 * _DAT_005a6974 + _DAT_005a67d4);
    fVar1 = (float)(int)local_54 * _DAT_005a69a4;
    local_c = (float *)((uint)param_2 & 3);
    *(float *)(((int)local_2d4 - in_EAX) + (int)pfVar5) = fVar1;
    *(undefined4 *)((int)local_2d4 + (4 - in_EAX) + (int)pfVar5) = 0x3f800000;
    fVar4 = local_20 - (float)local_24;
    fVar3 = local_1c - local_8;
    local_38 = fVar3;
    fVar2 = local_18 - fVar1;
    local_34 = fVar2;
    if (((uint)param_2 & 3) != 3) {
      *(float *)(local_58 + (int)pfVar5) =
           fVar4 * _DAT_005a862c + *(float *)(local_58 + (int)pfVar5);
      *(float *)(local_b8 + (int)pfVar5) =
           fVar3 * _DAT_005a862c + *(float *)(local_b8 + (int)pfVar5);
      *(float *)(local_c0 + (int)pfVar5) =
           fVar2 * _DAT_005a862c + *(float *)(local_c0 + (int)pfVar5);
    }
    if (uVar8 < 0xc0) {
      if (local_c != (float *)0x0) {
        *(float *)(local_5c + (int)pfVar5) =
             fVar4 * _DAT_005a8628 + *(float *)(local_5c + (int)pfVar5);
        *(float *)(local_b4 + (int)pfVar5) =
             fVar3 * _DAT_005a8628 + *(float *)(local_b4 + (int)pfVar5);
        *(float *)(local_ac + (int)pfVar5) =
             fVar2 * _DAT_005a8628 + *(float *)(local_ac + (int)pfVar5);
      }
      *(float *)(local_28 + (int)pfVar5) =
           fVar4 * _DAT_005a8624 + *(float *)(local_28 + (int)pfVar5);
      *(float *)(local_bc + (int)pfVar5) =
           fVar3 * _DAT_005a8624 + *(float *)(local_bc + (int)pfVar5);
      *(float *)(local_a4 + (int)pfVar5) =
           fVar2 * _DAT_005a8624 + *(float *)(local_a4 + (int)pfVar5);
      if (local_c != (float *)0x3) {
        *(float *)(local_2c + (int)pfVar5) =
             fVar4 * _DAT_005a69c0 + *(float *)(local_2c + (int)pfVar5);
        *(float *)(local_a0 + (int)pfVar5) =
             fVar3 * _DAT_005a69c0 + *(float *)(local_a0 + (int)pfVar5);
        *(float *)(local_b0 + (int)pfVar5) =
             fVar2 * _DAT_005a69c0 + *(float *)(local_b0 + (int)pfVar5);
      }
    }
    param_2 = (float)((int)param_2 + 1);
    uVar7 = uVar8 + 0x10;
    *(float *)((int)local_2d4 + uVar8) = (float)local_24 * DAT_005c6d80;
    *(float *)((int)local_2d4 + uVar8 + 4) = local_8 * DAT_005c6d84;
    *(float *)(((int)local_2d4 - in_EAX) + (int)pfVar5) = fVar1 * DAT_005c6d88;
    pfVar5 = pfVar5 + 4;
    uVar8 = uVar7;
  } while (uVar7 < 0x100);
  FUN_00575de7(&local_4c,&local_20,local_2d4,local_10);
  local_d4 = local_4c * _DAT_005c6d90;
  local_d0 = local_48 * _DAT_005c6d94;
  local_cc = local_44 * _DAT_005c6d98;
  local_3c = local_20 * _DAT_005c6d90;
  local_38 = local_1c * _DAT_005c6d94;
  local_34 = local_18 * _DAT_005c6d98;
  uVar6 = FUN_005759e8();
  local_28 = (undefined1 *)uVar6;
  local_2c = (undefined1 *)FUN_005759e8();
  uVar9 = (ushort)uVar6;
  uVar11 = (ushort)local_2c;
  if ((local_10 == 4) && (uVar9 == uVar11)) {
    param_1[2] = 0;
    param_1[3] = 0;
    *param_1 = uVar9;
    param_1[1] = uVar11;
  }
  else {
    FUN_005758cb();
    FUN_005758cb();
    local_4c = local_d4 * DAT_005c6d80;
    local_48 = local_d0 * DAT_005c6d84;
    local_44 = local_cc * DAT_005c6d88;
    local_20 = local_3c * DAT_005c6d80;
    local_1c = local_38 * DAT_005c6d84;
    local_18 = local_34 * DAT_005c6d88;
    if ((local_10 == 3) == uVar9 <= uVar11) {
      *param_1 = uVar9;
      param_1[1] = uVar11;
      local_9c[0] = local_4c;
      local_9c[1] = local_48;
      local_9c[2] = local_44;
      local_9c[3] = fStack_40;
      pfVar5 = &local_20;
    }
    else {
      param_1[1] = uVar9;
      *param_1 = uVar11;
      local_9c[0] = local_20;
      local_9c[1] = local_1c;
      local_9c[2] = local_18;
      local_9c[3] = fStack_14;
      pfVar5 = &local_4c;
    }
    local_8c = *pfVar5;
    local_88 = pfVar5[1];
    local_84 = pfVar5[2];
    local_80 = pfVar5[3];
    local_20 = *pfVar5 - local_9c[0];
    if (local_10 == 3) {
      local_24 = &DAT_005a8618;
      local_78 = (local_88 - local_9c[1]) * _DAT_005a67d4;
      local_74 = (local_84 - local_9c[2]) * _DAT_005a67d4;
      local_70 = (local_80 - local_9c[3]) * _DAT_005a67d4;
      fVar1 = _DAT_005a67d4;
    }
    else {
      local_24 = &DAT_005a8608;
      local_78 = _DAT_005a69ac * (local_88 - local_9c[1]);
      local_74 = _DAT_005a69ac * (local_84 - local_9c[2]);
      local_70 = _DAT_005a69ac * (local_80 - local_9c[3]);
      local_6c = local_20 * _DAT_005a8604 + local_9c[0];
      local_68 = (local_88 - local_9c[1]) * _DAT_005a8604 + local_9c[1];
      local_64 = (local_84 - local_9c[2]) * _DAT_005a8604 + local_9c[2];
      local_60 = (local_80 - local_9c[3]) * _DAT_005a8604 + local_9c[3];
      fVar1 = _DAT_005a69ac;
    }
    local_70 = local_70 + local_9c[3];
    local_74 = local_74 + local_9c[2];
    local_78 = local_78 + local_9c[1];
    local_7c = fVar1 * local_20 + local_9c[0];
    param_2 = local_84 - local_9c[2];
    fVar1 = local_88 - local_9c[1];
    local_8 = (float)(local_10 + -1);
    if (local_10 + -1 < 0) {
      local_8 = local_8 + _DAT_005a67dc;
    }
    if ((short)local_28 == (short)local_2c) {
      fVar2 = 0.0;
    }
    else {
      fVar2 = local_8 / (local_20 * local_20 + fVar1 * fVar1 + param_2 * param_2);
    }
    local_20 = local_20 * fVar2;
    pfVar5 = &local_1d4;
    for (iVar12 = 0x40; iVar12 != 0; iVar12 = iVar12 + -1) {
      *pfVar5 = 0.0;
      pfVar5 = pfVar5 + 1;
    }
    local_1c = fVar2 * fVar1;
    local_18 = fVar2 * param_2;
    FUN_005759c5();
    local_c = &local_1d4;
    local_2c = local_1cc + (-4 - in_EAX);
    local_58 = local_1c0 + -in_EAX;
    local_54 = local_1a0 + -in_EAX;
    iVar12 = (int)local_2d4 + (8 - in_EAX);
    pfVar5 = (float *)(in_EAX + 4);
    local_50 = local_180 + -in_EAX;
    uVar8 = extraout_EDX_00;
    param_2 = (float)extraout_EDX_00;
    do {
      if ((local_10 != 3) || (_DAT_005a67d4 <= pfVar5[2])) {
        local_4c = DAT_005c6d80 * pfVar5[-1] + *local_c;
        local_48 = DAT_005c6d84 * *pfVar5 + *(float *)((int)pfVar5 + local_a8);
        local_44 = DAT_005c6d88 * pfVar5[1] + *(float *)(local_2c + (int)pfVar5);
        fVar1 = (local_48 - local_9c[1]) * local_1c +
                (local_44 - local_9c[2]) * local_18 + (local_4c - local_9c[0]) * local_20;
        if (fVar1 < _DAT_005a67ec == (fVar1 == _DAT_005a67ec)) {
          if (fVar1 < local_8) {
            local_28 = (undefined1 *)(fVar1 + _DAT_005a67d4);
            local_5c = (undefined1 *)(int)ROUND(fVar1 + _DAT_005a67d4);
            iVar10 = *(int *)(local_24 + (int)local_5c * 4);
          }
          else {
            iVar10 = 1;
          }
        }
        else {
          iVar10 = 0;
        }
        uVar8 = uVar8 >> 2 | iVar10 << 0x1e;
        fVar2 = (local_4c - local_9c[iVar10 * 4]) * *(float *)(iVar12 + (int)pfVar5);
        uVar7 = (uint)param_2 & 3;
        fVar3 = (local_48 - local_9c[iVar10 * 4 + 1]) * *(float *)(iVar12 + (int)pfVar5);
        local_38 = fVar3;
        fVar1 = (local_44 - local_9c[iVar10 * 4 + 2]) * *(float *)(iVar12 + (int)pfVar5);
        local_34 = fVar1;
        if (uVar7 != 3) {
          *(float *)(local_b8 + (int)pfVar5) =
               fVar2 * _DAT_005a862c + *(float *)(local_b8 + (int)pfVar5);
          *(float *)(local_c0 + (int)pfVar5) =
               _DAT_005a862c * fVar3 + *(float *)(local_c0 + (int)pfVar5);
          *(float *)(local_58 + (int)pfVar5) =
               fVar1 * _DAT_005a862c + *(float *)(local_58 + (int)pfVar5);
        }
        if ((uint)param_2 < 0xc) {
          if (uVar7 != 0) {
            *(float *)(local_b4 + (int)pfVar5) =
                 fVar2 * _DAT_005a8628 + *(float *)(local_b4 + (int)pfVar5);
            *(float *)(local_ac + (int)pfVar5) =
                 _DAT_005a8628 * fVar3 + *(float *)(local_ac + (int)pfVar5);
            *(float *)(local_54 + (int)pfVar5) =
                 fVar1 * _DAT_005a8628 + *(float *)(local_54 + (int)pfVar5);
          }
          *(float *)(local_bc + (int)pfVar5) =
               fVar2 * _DAT_005a8624 + *(float *)(local_bc + (int)pfVar5);
          *(float *)(local_a4 + (int)pfVar5) =
               fVar3 * _DAT_005a8624 + *(float *)(local_a4 + (int)pfVar5);
          *(float *)(((int)&local_190 - in_EAX) + (int)pfVar5) =
               fVar1 * _DAT_005a8624 + *(float *)(((int)&local_190 - in_EAX) + (int)pfVar5);
          if (uVar7 != 3) {
            *(float *)(local_a0 + (int)pfVar5) =
                 fVar2 * _DAT_005a69c0 + *(float *)(local_a0 + (int)pfVar5);
            *(float *)(local_b0 + (int)pfVar5) =
                 local_38 * _DAT_005a69c0 + *(float *)(local_b0 + (int)pfVar5);
            *(float *)(local_50 + (int)pfVar5) =
                 local_34 * _DAT_005a69c0 + *(float *)(local_50 + (int)pfVar5);
          }
        }
      }
      else {
        uVar8 = uVar8 >> 2 | 0xc0000000;
      }
      param_2 = (float)((int)param_2 + 1);
      local_c = local_c + 4;
      pfVar5 = pfVar5 + 4;
    } while ((uint)param_2 < 0x10);
    *(uint *)(param_1 + 2) = uVar8;
  }
  return 0;
}

