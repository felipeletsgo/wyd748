// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e2552 | Name: FUN_004e2552


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e2552(int *param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  void *pvVar7;
  int local_88;
  int local_84;
  int local_80;
  float local_78;
  float local_74;
  int local_38;
  int local_34;
  undefined4 local_2c;
  undefined4 local_24;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0edf;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar3 = (**(code **)(*DAT_0092e654 + 8))();
  iVar4 = (**(code **)(*param_1 + 0x38))();
  if (iVar4 == 0) {
    uVar5 = 0;
  }
  else {
    fVar1 = (float)(uint)(iVar3 - param_1[0x20]) / (float)param_1[0x21];
    if (DAT_0067cf38 == 0) {
      uVar5 = 0;
    }
    else {
      if (_DAT_005a3660 < fVar1) {
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
      else if ((uint)param_1[0x23] < (uint)(iVar3 - param_1[0x22])) {
        for (local_1c = 0; local_1c < 8; local_1c = local_1c + 1) {
          FUN_00430f50(fVar1 * *(float *)(&DAT_005bb3c8 + local_1c * 4) * (float)param_1[0x26] +
                       (float)param_1[0x1d],0,
                       fVar1 * *(float *)(&DAT_005bb3e8 + local_1c * 4) * (float)param_1[0x26] +
                       (float)param_1[0x1f]);
          puVar6 = (undefined4 *)FUN_00430f20(local_2c,local_24);
          iVar4 = FUN_0049c70f(*puVar6,puVar6[1]);
          fVar2 = (float)iVar4 * _DAT_005a4214;
          local_34 = 0;
          iVar4 = DAT_013b71e8[0x6c21];
          if (*(int *)(iVar4 + 0x94) == 0) {
            if ((((*(float *)(iVar4 + 0x98) - 0.0 < (float)param_1[0x1d]) &&
                 ((float)param_1[0x1d] < *(float *)(iVar4 + 0xa0) + 0.0)) &&
                (*(float *)(iVar4 + 0x9c) - 0.0 < (float)param_1[0x1f])) &&
               ((float)param_1[0x1f] < *(float *)(iVar4 + 0xa4) + 0.0)) {
              local_34 = FUN_0053df40(local_2c,fVar2,local_24,0x3f800000);
            }
          }
          else {
            if (*(float *)(iVar4 + 0xa8) - (float)param_1[0x1d] <= _DAT_005a3534) {
              local_74 = -(*(float *)(iVar4 + 0xa8) - (float)param_1[0x1d]);
            }
            else {
              local_74 = *(float *)(iVar4 + 0xa8) - (float)param_1[0x1d];
            }
            if (*(float *)(iVar4 + 0xac) - (float)param_1[0x1f] <= _DAT_005a3534) {
              local_78 = -(*(float *)(iVar4 + 0xac) - (float)param_1[0x1f]);
            }
            else {
              local_78 = *(float *)(iVar4 + 0xac) - (float)param_1[0x1f];
            }
            if (local_74 + local_78 < 18.0) {
              local_34 = FUN_0053df40(local_2c,fVar2,local_24,0x3f800000);
            }
          }
          if (local_34 != 0) {
            local_38 = 0;
            if (param_1[0x25] == 0) {
              pvVar7 = operator_new(0xa0);
              local_8 = 0;
              if (pvVar7 == (void *)0x0) {
                local_80 = 0;
              }
              else {
                local_80 = FUN_004e2a30(local_2c,fVar2,local_24,2,0,param_1[0x24],0x22331100);
              }
              local_8 = 0xffffffff;
              local_38 = local_80;
            }
            else if (param_1[0x25] == 1) {
              pvVar7 = operator_new(0xa0);
              local_8 = 1;
              if (pvVar7 == (void *)0x0) {
                local_84 = 0;
              }
              else {
                local_84 = FUN_004e2a30(local_2c,fVar2,local_24,4,0,0x1122,0x1122);
              }
              local_8 = 0xffffffff;
              local_38 = local_84;
            }
            else if (param_1[0x25] == 2) {
              pvVar7 = operator_new(0xa0);
              local_8 = 2;
              if (pvVar7 == (void *)0x0) {
                local_88 = 0;
              }
              else {
                local_88 = FUN_004e2a30(local_2c,fVar2,local_24,5,0,0x4488,0x1122);
              }
              local_8 = 0xffffffff;
              local_38 = local_88;
            }
            if (local_38 != 0) {
              FUN_0054ac09(local_38);
            }
          }
        }
        param_1[0x22] = iVar3;
      }
      uVar5 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar5;
}

