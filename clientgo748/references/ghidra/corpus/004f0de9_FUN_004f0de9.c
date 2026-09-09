// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f0de9 | Name: FUN_004f0de9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004f0de9(int *param_1,undefined4 param_2)

{
  float10 fVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  void *pvVar5;
  int iVar6;
  uint uVar7;
  float10 fVar8;
  undefined4 uVar9;
  int *local_98;
  int *local_94;
  int *local_90;
  int *local_8c;
  int local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a163f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_004f710b(param_2);
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    (**(code **)(*param_1 + 0x4c))();
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    uVar7 = iVar2 - param_1[0xba];
    uVar4 = __ftol();
    if (uVar7 < 500) {
      fVar8 = (float10)FUN_00423b00(((float)uVar7 / _DAT_005a43cc) * _DAT_005a4148);
      fVar1 = (float10)_DAT_005a40ec;
      param_1[10] = (int)((float)param_1[0xb8] - (float)(fVar8 * fVar1) * (float)param_1[0xbc]);
      param_1[0xb] = (int)((float)(fVar8 * fVar1) * (float)param_1[0xbb] + (float)param_1[0xb9]);
    }
    if ((*(char *)((int)param_1 + 0x2fd) == '\0') && (DAT_005d03f8 == 0)) {
      uVar3 = 1;
    }
    else if ((*(char *)((int)param_1 + 0x2fd) == '\0') && ((char)param_1[0xbf] == '\0')) {
      uVar3 = 1;
    }
    else {
      if ((uint)((char)param_1[0xbf] * 5000 + (int)(((ulonglong)uVar4 % 3) % 2) * 500) < uVar7) {
        pvVar5 = operator_new(0x180);
        local_8 = 0;
        if (pvVar5 == (void *)0x0) {
          local_8c = (int *)0x0;
        }
        else {
          local_8c = (int *)FUN_004c8f70(0x38,100,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
        }
        local_8 = 0xffffffff;
        if (local_8c != (int *)0x0) {
          uVar3 = FUN_00430f50((float)param_1[0xbc] * (float)param_1[0xbe] + (float)param_1[10],
                               (float)param_1[0xc] + (float)param_1[0xbd],
                               (float)param_1[0xb] - (float)param_1[0xbb] * (float)param_1[0xbe]);
          FUN_004310a0(uVar3);
          local_8c[0x1b] = 1;
          (**(code **)(*local_8c + 0x3c))(0xffffee55);
          FUN_0054ac09(local_8c);
        }
        if (((param_1[0x12] == 1) && (0 < *(int *)(DAT_013b71e0 + 0x104))) &&
           (iVar6 = FUN_00429a6d(0x133), iVar6 != 0)) {
          uVar9 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x133);
          FUN_0042ad2b(uVar3,uVar9);
        }
        pvVar5 = operator_new(0xc0);
        local_8 = 1;
        if (pvVar5 == (void *)0x0) {
          local_90 = (int *)0x0;
        }
        else {
          local_90 = (int *)FUN_004df250(4,7,0x3f800000);
        }
        local_8 = 0xffffffff;
        if (local_90 != (int *)0x0) {
          (**(code **)(*local_90 + 0x44))(0x33443300);
          local_90[0x1b] = 1;
          (**(code **)(*local_90 + 0x3c))(param_1[10],param_1[0xb]);
          local_90[0x28] = 1000;
          FUN_0054ac09(local_90);
        }
        if (DAT_0092e660 == 0) {
          for (local_2c = 0; local_2c < 4; local_2c = local_2c + 1) {
            iVar6 = _rand();
            iVar6 = iVar6 % 5;
            pvVar5 = operator_new(0x180);
            local_8 = 2;
            if (pvVar5 == (void *)0x0) {
              local_94 = (int *)0x0;
            }
            else {
              local_94 = (int *)FUN_004c8f70(0,local_2c * -400 + 2000,
                                             (float)iVar6 * _DAT_005a40ec + _DAT_005a4158,
                                             (float)iVar6 * _DAT_005a40ec + _DAT_005a4158,
                                             (float)iVar6 * _DAT_005a40ec + _DAT_005a4158,0x3a83126f
                                             ,1,0x50);
            }
            local_8 = 0xffffffff;
            if (local_94 != (int *)0x0) {
              uVar3 = FUN_00430f50((float)iVar6 * _DAT_005a4214 +
                                   (float)param_1[0xbc] * (float)param_1[0xbe] + (float)param_1[10],
                                   (float)iVar6 * _DAT_005a45d4 +
                                   (float)param_1[0xc] + (float)param_1[0xbd],
                                   (float)iVar6 * _DAT_005a4214 +
                                   ((float)param_1[0xb] -
                                   (float)param_1[0xbb] * (float)param_1[0xbe]));
              uVar3 = FUN_004310a0(uVar3);
              FUN_004310a0(uVar3);
              local_94[0x1b] = 1;
              local_94[0x50] = 0x3c23d70a;
              local_94[0x4f] = 0x3f8ccccd;
              local_94[0x52] = 0xb;
              (**(code **)(*local_94 + 0x3c))(0xffffee55);
              FUN_0054ac09(local_94);
              if (local_2c == 3) {
                pvVar5 = operator_new(0x180);
                local_8 = 3;
                if (pvVar5 == (void *)0x0) {
                  local_98 = (int *)0x0;
                }
                else {
                  local_98 = (int *)FUN_004c8f70(0x3b,800,(float)iVar6 * _DAT_005a40ec +
                                                          _DAT_005a4158,
                                                 (float)iVar6 * _DAT_005a40ec + _DAT_005a4158,
                                                 (float)iVar6 * _DAT_005a40ec + _DAT_005a4158,
                                                 0x3a83126f,1,0x50);
                }
                local_8 = 0xffffffff;
                if (local_98 != (int *)0x0) {
                  uVar3 = FUN_00430f50(((float)param_1[0xbc] * (float)param_1[0xbe] +
                                       (float)param_1[10]) - _DAT_005a45d4,
                                       (float)iVar6 * _DAT_005a45d4 +
                                       (float)param_1[0xc] + (float)param_1[0xbd],
                                       ((float)param_1[0xb] -
                                       (float)param_1[0xbb] * (float)param_1[0xbe]) - _DAT_005a45d4)
                  ;
                  uVar3 = FUN_004310a0(uVar3);
                  FUN_004310a0(uVar3);
                  local_98[0x1b] = 0;
                  local_98[0x50] = 0x3c23d70a;
                  local_98[0x4f] = 0x3f8ccccd;
                  local_98[0x52] = 0xb;
                  if ((*(int *)(DAT_005ccf9c + 0x2a39c) == 1) ||
                     (*(int *)(DAT_005ccf9c + 0x2a3a0) == 1)) {
                    local_98[0x1b] = 1;
                  }
                  (**(code **)(*local_98 + 0x3c))(0xffffffff);
                  FUN_0054ac09(local_98);
                }
              }
            }
          }
        }
        param_1[0xba] = iVar2;
      }
      if (*(char *)((int)param_1 + 0x2fd) == '\x01') {
        *(undefined1 *)((int)param_1 + 0x2fd) = 0;
      }
      uVar3 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}

