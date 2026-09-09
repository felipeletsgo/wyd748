// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aec3d | Name: FUN_004aec3d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004aec3d(int param_1)

{
  float fVar1;
  int iVar2;
  float10 fVar3;
  float local_44;
  float local_3c;
  float local_38;
  int local_2c;
  undefined4 local_28;
  int local_24;
  undefined1 local_20;
  undefined1 uStack_1f;
  ushort uStack_1e;
  int local_18;
  int local_14;
  int local_10;
  float local_c;
  int local_8;
  
  if (*(int *)(param_1 + 8) != 0) {
    _memset(&local_2c,0,0x14);
    local_8 = (**(code **)(**(int **)(param_1 + 8) + 0x24))
                        (*(undefined4 *)(param_1 + 8),0x14,&local_2c);
    if (local_8 < 0) {
      local_8 = (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(undefined4 *)(param_1 + 8));
      while (local_8 == -0x7ff8ffe2) {
        local_8 = (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(undefined4 *)(param_1 + 8));
      }
    }
    else {
      if (DAT_005b4928 == 0) {
        *(int *)(param_1 + 0xc) = local_2c;
        *(int *)(param_1 + 0x14) = local_24;
        *(int *)(param_1 + 0x18) = local_24;
      }
      else {
        *(int *)(param_1 + 0xc) = -local_2c;
        *(int *)(param_1 + 0x14) = -local_24;
        *(int *)(param_1 + 0x18) = local_24;
      }
      *(undefined4 *)(param_1 + 0x10) = local_28;
      *(uint *)(param_1 + 0x1c) = CONCAT11(uStack_1f,local_20) & 0x80;
      *(uint *)(param_1 + 0x20) = CONCAT21(uStack_1e,uStack_1f) & 0x80;
      *(uint *)(param_1 + 0x24) = uStack_1e & 0x80;
      if (*(int *)(param_1 + 0x1c) != 0) {
        FUN_004af854();
      }
      if ((*(int *)(param_1 + 0x1c) == 0) && (*(int *)(param_1 + 0x28) != 0)) {
        FUN_004af910();
      }
      if (*(int *)(param_1 + 0x20) != 0) {
        FUN_004af91b();
      }
      if ((*(int *)(param_1 + 0x20) == 0) && (*(int *)(param_1 + 0x2c) != 0)) {
        FUN_004afa01();
      }
      if ((*(int *)(param_1 + 0x434) == 1) && (*(int *)(param_1 + 0x20) == 0)) {
        *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x10) * 3;
        *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x10) * 3;
      }
      if (DAT_013b736c != 0) {
        *(undefined4 *)(param_1 + 0x14) = 0;
        *(undefined4 *)(param_1 + 0x18) = 0;
      }
      local_10 = *(int *)(DAT_013b71e8 + 0x1b084);
      local_c = *(float *)(local_10 + 0xbc);
      if (_DAT_005a45ec <= *(float *)(local_10 + 0x2c)) {
        local_c = *(float *)(local_10 + 0xbc);
      }
      local_14 = DAT_0067cf38;
      if (DAT_0067cf38 != 0) {
        local_18 = *(int *)(DAT_0067cf38 + 0x24);
        if (((((local_18 == 30000) || (local_18 == 0x7534)) || (local_18 == 0x7535)) ||
            (local_18 == 0x7531)) && (*(short *)(DAT_0067cf38 + 0x20098) < 0)) {
          if (*(int *)(local_10 + 0x68) == 0) {
            if (((*(int *)(local_10 + 0x24) == 0) && (*(int *)(param_1 + 0x24) != 0)) &&
               (*(int *)(DAT_0067cf38 + 0x40) != 0)) {
              *(float *)(local_10 + 0x2c) =
                   *(float *)(local_10 + 0x2c) - (float)*(int *)(param_1 + 0x10) * _DAT_005a45e8;
              if (*(float *)(local_10 + 0x2c) < _DAT_005a45e4) {
                *(undefined4 *)(local_10 + 0x2c) = 0xbf7c430e;
              }
              if (_DAT_005a45e0 < *(float *)(local_10 + 0x2c)) {
                *(undefined4 *)(local_10 + 0x2c) = 0x3f400000;
              }
              *(float *)(local_10 + 0x28) =
                   (float)*(int *)(param_1 + 0xc) * _DAT_005a43e8 + *(float *)(local_10 + 0x28);
              if (_DAT_005a4290 < *(float *)(local_10 + 0x28)) {
                *(float *)(local_10 + 0x28) = *(float *)(local_10 + 0x28) - _DAT_005a4290;
              }
              if (*(float *)(local_10 + 0x28) < _DAT_005a3534) {
                *(float *)(local_10 + 0x28) = *(float *)(local_10 + 0x28) + _DAT_005a4290;
              }
              if (((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x4c) != 0)) &&
                 (*(char *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x79c) != '\0')) {
                local_38 = 0.45;
                iVar2 = *(int *)(DAT_0067cf38 + 0x4c);
                if (((*(int *)(iVar2 + 0x7a0) == 0x27) || (*(int *)(iVar2 + 0x7a0) == 0x28)) ||
                   ((*(int *)(iVar2 + 0x7a0) == 0x14 && (*(short *)(iVar2 + 0x1a0) != 3)))) {
                  local_38 = 0.23;
                }
                else if (*(int *)(iVar2 + 0x7a0) == 0x26) {
                  local_38 = 0.22;
                }
                if (local_38 < *(float *)(local_10 + 0x2c)) {
                  *(float *)(local_10 + 0x2c) = local_38;
                }
              }
            }
            if (*(int *)(DAT_013b71e0 + 0x10c) == 0) {
              if ((*(int *)(local_10 + 0x24) == 0) || (*(int *)(local_10 + 0x24) == 1)) {
                if ((*(int *)(param_1 + 0x18) < 0) && (local_c < *(float *)(local_10 + 0x34))) {
                  *(float *)(local_10 + 0x34) =
                       (float)*(int *)(param_1 + 0x14) / _DAT_005a453c + *(float *)(local_10 + 0x34)
                  ;
                  *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
                }
                if (*(float *)(local_10 + 0x34) < local_c) {
                  *(float *)(local_10 + 0x34) = local_c;
                  *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
                }
                if ((0 < *(int *)(param_1 + 0x18)) &&
                   (*(float *)(local_10 + 0x34) < *(float *)(local_10 + 0xc0))) {
                  *(float *)(local_10 + 0x34) =
                       (float)*(int *)(param_1 + 0x14) / _DAT_005a453c + *(float *)(local_10 + 0x34)
                  ;
                  *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
                }
                if (*(float *)(local_10 + 0xc0) < *(float *)(local_10 + 0x34)) {
                  *(undefined4 *)(local_10 + 0x34) = *(undefined4 *)(local_10 + 0xc0);
                  *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
                }
              }
            }
            else if ((*(int *)(DAT_013b71e0 + 0x10c) == 1) &&
                    ((*(int *)(local_10 + 0x24) == 0 || (*(int *)(local_10 + 0x24) == 1)))) {
              if ((*(int *)(param_1 + 0x14) < 0) && (local_c < *(float *)(local_10 + 0x34))) {
                *(float *)(local_10 + 0x34) =
                     (float)*(int *)(param_1 + 0x14) / _DAT_005a453c + *(float *)(local_10 + 0x34);
                *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
              }
              if (*(float *)(local_10 + 0x34) < local_c) {
                *(float *)(local_10 + 0x34) = local_c;
                *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
              }
              if ((0 < *(int *)(param_1 + 0x14)) &&
                 (*(float *)(local_10 + 0x34) < *(float *)(local_10 + 0xc0))) {
                *(float *)(local_10 + 0x34) =
                     (float)*(int *)(param_1 + 0x14) / _DAT_005a453c + *(float *)(local_10 + 0x34);
                *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
              }
              if (*(float *)(local_10 + 0xc0) < *(float *)(local_10 + 0x34)) {
                *(undefined4 *)(local_10 + 0x34) = *(undefined4 *)(local_10 + 0xc0);
                *(undefined4 *)(local_10 + 0x38) = *(undefined4 *)(local_10 + 0x34);
              }
            }
            *(undefined4 *)(local_10 + 0x70) = *(undefined4 *)(local_10 + 0x28);
            *(undefined4 *)(local_10 + 0x74) = *(undefined4 *)(local_10 + 0x2c);
          }
          else if (*(int *)(local_10 + 0x6c) == 10) {
            iVar2 = (**(code **)(*DAT_0092e654 + 8))();
            fVar3 = (float10)FUN_00423b00((((float)(uint)(iVar2 - *(int *)(local_10 + 0x68)) /
                                           _DAT_005a35c0) * _DAT_005a4148) / _DAT_005a365c +
                                          _DAT_005a45dc);
            local_3c = (float)(fVar3 + (float10)_DAT_005a3660);
            if (_DAT_005a3660 <= local_3c) {
              local_3c = 1.0;
            }
            *(float *)(local_10 + 0x2c) =
                 (_DAT_005a3660 - local_3c) * _DAT_005a4214 - _DAT_005a437c * local_3c;
            *(undefined4 *)(local_10 + 0x74) = *(undefined4 *)(local_10 + 0x2c);
            *(float *)(local_10 + 0x28) =
                 _DAT_005a437c * local_3c + (_DAT_005a3660 - local_3c) * _DAT_005a4148;
            *(undefined4 *)(local_10 + 0x70) = *(undefined4 *)(local_10 + 0x28);
            *(float *)(local_10 + 0x34) =
                 local_3c * *(float *)(local_10 + 0xc0) + (_DAT_005a3660 - local_3c) * _DAT_005a45d8
            ;
            *(float *)(local_10 + 0x38) =
                 local_3c * *(float *)(local_10 + 0xc0) + (_DAT_005a3660 - local_3c) * _DAT_005a45d8
            ;
          }
          else {
            iVar2 = (**(code **)(*DAT_0092e654 + 8))();
            local_44 = (float)(uint)(iVar2 - *(int *)(local_10 + 0x68)) / _DAT_005a43c0;
            if (_DAT_005a3660 < local_44) {
              local_44 = 1.0;
            }
            fVar1 = local_44 * _DAT_005a4148 * _DAT_005a3e9c;
            fVar3 = (float10)FUN_00423b00(fVar1);
            *(float *)(local_10 + 0x2c) =
                 (float)(((float10)_DAT_005a3660 - (float10)local_44) *
                         fVar3 * (float10)_DAT_005a45d4 * (float10)*(int *)(local_10 + 0x6c) +
                        (float10)*(float *)(local_10 + 0x74));
            fVar3 = (float10)FUN_00423b00(fVar1);
            *(float *)(local_10 + 0x28) =
                 (float)(((float10)_DAT_005a3660 - (float10)local_44) *
                         fVar3 * (float10)_DAT_005a45d4 * (float10)*(int *)(local_10 + 0x6c) +
                        (float10)*(float *)(local_10 + 0x70));
          }
        }
        FUN_0058f220(param_1 + 0x28,param_1 + 0x1c,0xc);
      }
    }
  }
  return 1;
}

