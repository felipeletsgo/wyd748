// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005161af | Name: FUN_005161af


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005161af(int *param_1)

{
  float fVar1;
  int *piVar2;
  int iVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  uint uVar7;
  void *pvVar8;
  undefined4 uVar9;
  float10 fVar10;
  int local_c4;
  undefined4 local_94;
  int local_44;
  int local_38;
  float local_2c;
  float local_28;
  float local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2032;
  local_10 = ExceptionList;
  if (param_1[0x85] != 0) {
    return;
  }
  local_18 = DAT_0067cf38;
  if (param_1[0x18] == 0) {
    return;
  }
  local_14 = 0x3f800000;
  ExceptionList = &local_10;
  iVar6 = FUN_00494dcf();
  if (iVar6 == 30000) {
    piVar2 = *(int **)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54);
    FUN_0049cb58(&local_2c,param_1[10],param_1[0xb]);
    local_1c = 0;
    iVar6 = *(int *)(DAT_0067cf38 + 0x44);
    if (((*(int *)(iVar6 + 0x164) != 0) && (*(int *)(iVar6 + 0x164) != 2)) &&
       (*(int *)(iVar6 + 0x164) != 0xc)) {
      for (local_38 = 0; local_38 < 2; local_38 = local_38 + 1) {
        iVar6 = *(int *)(DAT_0067cf38 + 0x70 + local_38 * 4);
        if (iVar6 != 0) {
          iVar3 = *(int *)(iVar6 + 0x28);
          for (local_44 = 0; local_44 < iVar3; local_44 = local_44 + 1) {
            if (*(int *)(*(int *)(iVar6 + 0x2c + local_44 * 4) + 0x28) == 1) {
              fVar10 = (float10)FUN_004d0150(param_1 + 10);
              fVar1 = (float)fVar10;
              if (fVar1 < _DAT_005a4104) {
                local_2c = *(float *)(DAT_005ccf9c + 0x2a6e4) * _DAT_005a4158 +
                           ((_DAT_005a4104 - fVar1) *
                           *(float *)(*(int *)(iVar6 + 0x2c + local_44 * 4) + 0x58)) / _DAT_005a40f0
                ;
                local_28 = *(float *)(DAT_005ccf9c + 0x2a6e8) * _DAT_005a4158 +
                           ((_DAT_005a4104 - fVar1) *
                           *(float *)(*(int *)(iVar6 + 0x2c + local_44 * 4) + 0x5c)) / _DAT_005a40f0
                ;
                local_24 = *(float *)(DAT_005ccf9c + 0x2a6ec) * _DAT_005a4158 +
                           ((_DAT_005a4104 - fVar1) *
                           *(float *)(*(int *)(iVar6 + 0x2c + local_44 * 4) + 0x60)) / _DAT_005a40f0
                ;
                local_1c = 1;
                break;
              }
            }
          }
        }
      }
    }
    if (local_1c == 0) {
      local_2c = *(float *)(DAT_005ccf9c + 0x2a6e4) * _DAT_005a34a0 + local_2c * _DAT_005a34a0;
      local_28 = *(float *)(DAT_005ccf9c + 0x2a6e8) * _DAT_005a34a0 + local_28 * _DAT_005a34a0;
      local_24 = *(float *)(DAT_005ccf9c + 0x2a6ec) * _DAT_005a34a0 + local_24 * _DAT_005a34a0;
      local_20 = local_14;
    }
    *(undefined4 *)(param_1[0x18] + 0x2b4) = local_14;
    *(undefined4 *)(param_1[0x18] + 0x2c4) = local_14;
    iVar6 = param_1[0x18];
    *(float *)(iVar6 + 0x288) = local_2c;
    *(float *)(iVar6 + 0x28c) = local_28;
    *(float *)(iVar6 + 0x290) = local_24;
    *(undefined4 *)(iVar6 + 0x294) = local_20;
    *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
    if ((*(int **)(DAT_0067cf38 + 0x7c) == param_1) && (piVar2 != param_1)) {
      if (((uint)param_1[8] < 1000) || ((*(byte *)((int)param_1 + 0x44a) & 0xf) != 0xf)) {
        if (((uint)param_1[8] < 1000) || (iVar6 = FUN_00516105(), iVar6 == 0)) {
          if ((char)param_1[0x1c9] == '\0') {
            if ((char)param_1[0x92] == '\x01') {
              param_1[0x179] = -0x7700ff01;
            }
            else if (((short)param_1[0x1a0] == 0) ||
                    (((short)param_1[0x1a0] != (short)piVar2[0x1a0] &&
                     (*(short *)(DAT_013b71e8 + 0xf12) != (short)param_1[0x1a0])))) {
              iVar6 = (**(code **)(*param_1 + 0x50))();
              if ((iVar6 == 0) && (*(int *)(param_1[0x10d] + 0x228) == -0x10000)) {
                param_1[0x179] = -0x77010000;
              }
              else {
                param_1[0x179] = -0x77000001;
              }
            }
            else {
              param_1[0x179] = -0x77ff0001;
            }
          }
          else {
            param_1[0x179] = -0x77ff0100;
          }
        }
        else {
          param_1[0x179] = -0x77ff0100;
        }
      }
      else {
        param_1[0x179] = -0x77ff0100;
      }
      if ((((((*(byte *)((int)param_1 + 0x44a) & 0xf) == 0xf) &&
            (*(char *)((int)piVar2 + 0x79b) != '\0')) && (*(char *)((int)piVar2 + 0x79b) != '\x03'))
          && ((param_1[0x66] != 0 && (piVar2[0x66] != 0)))) &&
         (*(char *)((int)param_1 + 0x79b) != *(char *)((int)piVar2 + 0x79b))) {
        param_1[0x179] = -0x77010000;
      }
    }
    if (((DAT_0092e664 == 0) && (*(char *)((int)param_1 + 0x77d) == '\x01')) &&
       ((iVar6 = (**(code **)(*DAT_0092e654 + 8))(), param_1[0x1d0] == 3 &&
        (iVar3 = param_1[0x18b], uVar7 = __ftol(), uVar7 < (uint)(iVar6 - iVar3))))) {
      pvVar8 = operator_new(0x124);
      local_8 = 0;
      if (pvVar8 == (void *)0x0) {
        local_c4 = 0;
      }
      else {
        local_c4 = FUN_004cb900(0x2b,1000,0x3f000000,0x3f000000,0x3f000000,0x3a03126f,0);
      }
      local_8 = 0xffffffff;
      if (local_c4 != 0) {
        uVar9 = FUN_00430f50(param_1[10],(float)param_1[0xc] + _DAT_005a4158,param_1[0xb]);
        FUN_004310a0(uVar9);
        *(undefined4 *)(local_c4 + 0x6c) = 1;
        FUN_0054ac09(local_c4);
        param_1[0x18b] = iVar6;
      }
    }
    if ((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) {
      *(float *)(param_1[0x18] + 0x2b8) =
           _DAT_005a34a0 * 0.6 + *(float *)(DAT_005ccf9c + 0x2a6f4) * _DAT_005a4154;
      *(float *)(param_1[0x18] + 700) =
           _DAT_005a34a0 * 0.6 + *(float *)(DAT_005ccf9c + 0x2a6f8) * _DAT_005a4154;
      *(float *)(param_1[0x18] + 0x2c0) =
           _DAT_005a34a0 * 0.6 + *(float *)(DAT_005ccf9c + 0x2a6fc) * _DAT_005a4154;
      ExceptionList = local_10;
      return;
    }
    if ((char)param_1[0x1e2] == '\x01') {
      local_20 = local_14;
      local_2c = local_2c * _DAT_005a4154 + _DAT_005a4158;
      local_28 = local_28 * _DAT_005a4154;
      local_24 = local_24 * _DAT_005a4154;
      iVar6 = param_1[0x18];
      *(float *)(iVar6 + 0x2b8) = local_2c;
      *(float *)(iVar6 + 700) = local_28;
      *(float *)(iVar6 + 0x2c0) = local_24;
      *(undefined4 *)(iVar6 + 0x2c4) = local_14;
      iVar6 = param_1[0x18];
      iVar3 = param_1[0x18];
      *(undefined4 *)(iVar3 + 0x2a8) = *(undefined4 *)(iVar6 + 0x2b8);
      *(undefined4 *)(iVar3 + 0x2ac) = *(undefined4 *)(iVar6 + 700);
      *(undefined4 *)(iVar3 + 0x2b0) = *(undefined4 *)(iVar6 + 0x2c0);
      *(undefined4 *)(iVar3 + 0x2b4) = *(undefined4 *)(iVar6 + 0x2c4);
      iVar6 = param_1[0x18];
      *(float *)(iVar6 + 0x288) = local_2c;
      *(float *)(iVar6 + 0x28c) = local_28;
      *(float *)(iVar6 + 0x290) = local_24;
      *(undefined4 *)(iVar6 + 0x294) = local_14;
      if ((char)param_1[0x1e7] < '\x01') {
        ExceptionList = local_10;
        return;
      }
      if (param_1[0x67] == 0) {
        ExceptionList = local_10;
        return;
      }
      FUN_0058f220(param_1[0x67] + 0x288,param_1[0x18] + 0x288,0x44);
      ExceptionList = local_10;
      return;
    }
    if (*(char *)((int)param_1 + 0x77f) == '\x01') {
      iVar6 = param_1[0x18];
      *(undefined4 *)(iVar6 + 0x288) = 0;
      *(undefined4 *)(iVar6 + 0x28c) = 0x3ecccccd;
      *(undefined4 *)(iVar6 + 0x290) = 0x3f666666;
      *(undefined4 *)(iVar6 + 0x294) = local_14;
      iVar6 = param_1[0x18];
      iVar3 = param_1[0x18];
      *(undefined4 *)(iVar3 + 0x2b8) = *(undefined4 *)(iVar6 + 0x288);
      *(undefined4 *)(iVar3 + 700) = *(undefined4 *)(iVar6 + 0x28c);
      *(undefined4 *)(iVar3 + 0x2c0) = *(undefined4 *)(iVar6 + 0x290);
      *(undefined4 *)(iVar3 + 0x2c4) = *(undefined4 *)(iVar6 + 0x294);
      iVar6 = param_1[0x18];
      iVar3 = param_1[0x18];
      *(undefined4 *)(iVar3 + 0x2a8) = *(undefined4 *)(iVar6 + 0x2b8);
      *(undefined4 *)(iVar3 + 0x2ac) = *(undefined4 *)(iVar6 + 700);
      *(undefined4 *)(iVar3 + 0x2b0) = *(undefined4 *)(iVar6 + 0x2c0);
      *(undefined4 *)(iVar3 + 0x2b4) = *(undefined4 *)(iVar6 + 0x2c4);
      if ((char)param_1[0x1e7] < '\x01') {
        ExceptionList = local_10;
        return;
      }
      if (param_1[0x67] == 0) {
        ExceptionList = local_10;
        return;
      }
      FUN_0058f220(param_1[0x67] + 0x288,param_1[0x18] + 0x288,0x44);
      ExceptionList = local_10;
      return;
    }
    if ((char)param_1[0x1df] == '\x01') {
      iVar6 = param_1[0x18];
      *(undefined4 *)(iVar6 + 0x2b8) = 0;
      *(undefined4 *)(iVar6 + 700) = 0x3e99999a;
      *(undefined4 *)(iVar6 + 0x2c0) = 0;
      *(undefined4 *)(iVar6 + 0x2c4) = local_14;
      iVar6 = param_1[0x18];
      iVar3 = param_1[0x18];
      *(undefined4 *)(iVar3 + 0x2a8) = *(undefined4 *)(iVar6 + 0x2b8);
      *(undefined4 *)(iVar3 + 0x2ac) = *(undefined4 *)(iVar6 + 700);
      *(undefined4 *)(iVar3 + 0x2b0) = *(undefined4 *)(iVar6 + 0x2c0);
      *(undefined4 *)(iVar3 + 0x2b4) = *(undefined4 *)(iVar6 + 0x2c4);
      iVar6 = param_1[0x18];
      *(undefined4 *)(iVar6 + 0x288) = 0;
      *(undefined4 *)(iVar6 + 0x28c) = 0x3f4ccccd;
      *(undefined4 *)(iVar6 + 0x290) = 0;
      *(undefined4 *)(iVar6 + 0x294) = local_14;
      if ((char)param_1[0x1e7] < '\x01') {
        ExceptionList = local_10;
        return;
      }
      if (param_1[0x67] == 0) {
        ExceptionList = local_10;
        return;
      }
      FUN_0058f220(param_1[0x67] + 0x288,param_1[0x18] + 0x288,0x44);
      ExceptionList = local_10;
      return;
    }
    if (*(char *)((int)param_1 + 0x781) == '\x01') {
      iVar6 = (**(code **)(*DAT_0092e654 + 8))();
      fVar1 = ((float)((uint)(iVar6 + param_1[8] * 300) % 1000) / _DAT_005a43c0) * _DAT_005a4148;
      fVar10 = (float10)FUN_00423b00(fVar1 + fVar1);
      fVar1 = (float)(fVar10 * (float10)_DAT_005a34a0 + (float10)_DAT_005a34a0);
      fVar4 = fVar1 / _DAT_005a365c;
      iVar6 = param_1[0x18];
      *(float *)(iVar6 + 0x2b8) = fVar1 / _DAT_005a365c;
      *(float *)(iVar6 + 700) = fVar4;
      *(undefined4 *)(iVar6 + 0x2c0) = 0;
      *(undefined4 *)(iVar6 + 0x2c4) = local_14;
      iVar6 = param_1[0x18];
      iVar3 = param_1[0x18];
      *(undefined4 *)(iVar3 + 0x2a8) = *(undefined4 *)(iVar6 + 0x2b8);
      *(undefined4 *)(iVar3 + 0x2ac) = *(undefined4 *)(iVar6 + 700);
      *(undefined4 *)(iVar3 + 0x2b0) = *(undefined4 *)(iVar6 + 0x2c0);
      *(undefined4 *)(iVar3 + 0x2b4) = *(undefined4 *)(iVar6 + 0x2c4);
      iVar6 = param_1[0x18];
      *(float *)(iVar6 + 0x288) = fVar1;
      *(float *)(iVar6 + 0x28c) = fVar4;
      *(undefined4 *)(iVar6 + 0x290) = 0;
      *(undefined4 *)(iVar6 + 0x294) = local_14;
      if ((char)param_1[0x1e7] < '\x01') {
        ExceptionList = local_10;
        return;
      }
      if (param_1[0x67] == 0) {
        ExceptionList = local_10;
        return;
      }
      FUN_0058f220(param_1[0x67] + 0x288,param_1[0x18] + 0x288,0x44);
      ExceptionList = local_10;
      return;
    }
    *(float *)(param_1[0x18] + 0x2b8) =
         local_2c * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f4) * _DAT_005a40ec;
    *(float *)(param_1[0x18] + 700) =
         local_2c * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f8) * _DAT_005a40ec;
    *(float *)(param_1[0x18] + 0x2c0) =
         local_2c * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6fc) * _DAT_005a40ec;
    if ((param_1[0x93] == 0x20) && (*(short *)((int)param_1 + 0x1d2) == 2)) {
      *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3f266666;
      *(undefined4 *)(param_1[0x18] + 700) = 0;
      *(undefined4 *)(param_1[0x18] + 0x2c0) = 0;
    }
  }
  else if (param_1[0x8d] == 0) {
    if (param_1[0x8e] == 1) {
      *(undefined4 *)(param_1[0x18] + 0x288) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x28c) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x290) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3e99999a;
      *(undefined4 *)(param_1[0x18] + 700) = 0x3f000000;
      *(undefined4 *)(param_1[0x18] + 0x2c0) = 0;
    }
    else {
      fVar1 = *(float *)(DAT_005ccf9c + 0x2a6e4) * _DAT_005a40ec + _DAT_005a4158;
      fVar4 = *(float *)(DAT_005ccf9c + 0x2a6e8) * _DAT_005a40ec + _DAT_005a4158;
      fVar5 = *(float *)(DAT_005ccf9c + 0x2a6ec) * _DAT_005a40ec + _DAT_005a4158;
      iVar6 = param_1[0x18];
      *(float *)(iVar6 + 0x288) = fVar1;
      *(float *)(iVar6 + 0x28c) = fVar4;
      *(float *)(iVar6 + 0x290) = fVar5;
      *(undefined4 *)(iVar6 + 0x294) = local_94;
      iVar6 = param_1[0x18];
      *(float *)(iVar6 + 0x2b8) = fVar1;
      *(float *)(iVar6 + 700) = fVar4;
      *(float *)(iVar6 + 0x2c0) = fVar5;
      *(undefined4 *)(iVar6 + 0x2c4) = local_94;
      *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
    }
  }
  else {
    *(undefined4 *)(param_1[0x18] + 0x288) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x28c) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x290) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 700) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2c0) = 0x3f800000;
  }
  if (('\0' < *(char *)((int)param_1 + 0x79b)) && (param_1[0x66] != 0)) {
    FUN_0058f220(param_1[0x66] + 0x288,param_1[0x18] + 0x288,0x44);
  }
  if (('\0' < (char)param_1[0x1e7]) && (param_1[0x67] != 0)) {
    FUN_0058f220(param_1[0x67] + 0x288,param_1[0x18] + 0x288,0x44);
  }
  ExceptionList = local_10;
  return;
}

