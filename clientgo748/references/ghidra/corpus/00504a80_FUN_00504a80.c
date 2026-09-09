// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00504a80 | Name: FUN_00504a80


/* WARNING: Removing unreachable block (ram,0x00505f44) */
/* WARNING: Removing unreachable block (ram,0x00505f5e) */
/* WARNING: Removing unreachable block (ram,0x00505f7e) */
/* WARNING: Removing unreachable block (ram,0x00505f85) */
/* WARNING: Removing unreachable block (ram,0x00505f8e) */
/* WARNING: Removing unreachable block (ram,0x00505f95) */
/* WARNING: Removing unreachable block (ram,0x00505fe3) */
/* WARNING: Removing unreachable block (ram,0x00506031) */
/* WARNING: Removing unreachable block (ram,0x00506043) */
/* WARNING: Removing unreachable block (ram,0x0050603a) */
/* WARNING: Removing unreachable block (ram,0x0050604c) */
/* WARNING: Removing unreachable block (ram,0x00506055) */
/* WARNING: Removing unreachable block (ram,0x00506067) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00504a80(int *param_1)

{
  int *piVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  char *pcVar6;
  size_t sVar7;
  size_t sVar8;
  int iVar9;
  float local_80;
  float local_44;
  undefined1 local_38 [12];
  int local_2c;
  float local_28;
  int local_24;
  undefined1 local_20 [8];
  float local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  iVar4 = FUN_00494dcf();
  if (((iVar4 == 30000) && (local_c = DAT_0067cf38, DAT_0067cf38 != 0)) && (param_1[0x85] == 0)) {
    if (*(char *)((int)param_1 + 0x795) == '\x01') {
      (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
      (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
      (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
      if (param_1[0x1f9] != 0) {
        (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
      }
      (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
      if (DAT_005b892c == 2) {
        (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
      }
      (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
      (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
      (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
    }
    else {
      if (((DAT_005b892c == 2) && (DAT_0067cf38 != 0)) &&
         ((*(int *)(DAT_0067cf38 + 0x27ad4) != 0 && (*(int *)(DAT_0067cf38 + 0x27ae0) != 0)))) {
        if ((((char)param_1[0x1e7] == '\x01') && ((short)param_1[0x68] != 0x1b)) &&
           (((short)param_1[0x68] != 0x1c &&
            ((((short)param_1[0x68] != 0x1d && ((short)param_1[0x68] != 0x1e)) &&
             (*(int *)(DAT_0067cf38 + 0x26e1c) == 0)))))) {
          (**(code **)(**(int **)(DAT_0067cf38 + 0x27ad4) + 0x60))(1);
          (**(code **)(**(int **)(local_c + 0x27ae0) + 0x60))(1);
        }
        else {
          (**(code **)(**(int **)(DAT_0067cf38 + 0x27ad4) + 0x60))(0);
          (**(code **)(**(int **)(local_c + 0x27ae0) + 0x60))(0);
        }
      }
      (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
      (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
      local_8 = 0;
      if (((*(int *)(local_c + 0x4c) != 0) &&
          (param_1[8] == *(int *)(*(int *)(local_c + 0x4c) + 0x7d4))) &&
         (*(char *)((int)param_1 + 0x245) == '\0')) {
        local_8 = 1;
      }
      if (((*(int *)(local_c + 0x50) == 0) && (*(int **)(local_c + 0x7c) != param_1)) &&
         (local_8 == 0)) {
        (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
        (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
        (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
        if (param_1[0x1f9] != 0) {
          (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
        }
        (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
        if (DAT_005b892c == 2) {
          (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
        }
        (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
        (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
      }
      else {
        if (((0 < DAT_005d040c) && ((uint)param_1[8] < 1000)) && ((short)param_1[0x1a0] != 0)) {
          FUN_0050484a();
        }
        if (param_1[0x106] != 0) {
          if ((*(int **)(local_c + 0x7c) == param_1) ||
             ((((((param_1[0x93] == 1 || (param_1[0x93] == 2)) ||
                 ((param_1[0x93] == 4 || ((param_1[0x93] == 8 || (param_1[0x93] == 0x1a)))))) ||
                ((param_1[0x93] == 0x21 && (*(short *)((int)param_1 + 0x1d2) == 0)))) &&
               ((uint)param_1[8] < 1000)) ||
              ((((((iVar4 = FUN_00516105(), iVar4 != 0 || ((char)param_1[0x92] == '\x01')) ||
                  ((short)param_1[0x1a0] != 0)) || ((uint)param_1[8] < 1000)) ||
                (((short)param_1[0x94] == 0x10f && ((*(byte *)((int)param_1 + 0x44a) & 0xf) != 0))))
               || (local_8 == 1)))))) {
            FUN_00430f10();
            FUN_00430f10();
            FUN_00430f10();
            local_2c = param_1[10];
            local_24 = param_1[0xb];
            local_28 = *(float *)(&DAT_0092e7d4 + param_1[0x17] * 8) * (float)param_1[0xe] +
                       (float)param_1[0xc] + _DAT_005a4158;
            thunk_FUN_00561462(local_38,&local_2c,DAT_005ccf9c + 0x2a62c);
            thunk_FUN_00561462(local_20,local_38,DAT_005ccf9c + 0x2a5ec);
            if ((local_18 < _DAT_005a3534) || (_DAT_005a3660 <= local_18)) {
              (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
              (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
              (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
              (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
              if (param_1[0x1f9] != 0) {
                (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
              }
              (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
              if (DAT_005b892c == 2) {
                (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
              }
              (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
              (**(code **)(*(int *)param_1[0x10b] + 0x60))(0);
              (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
              (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
              if (((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) &&
                 (*(char *)((int)param_1 + 0x245) != '\x01')) {
                (**(code **)(*(int *)param_1[0x10f] + 0x60))(1);
                (**(code **)(*(int *)param_1[0x110] + 0x60))(1);
              }
            }
            else {
              local_10 = __ftol();
              if ((char)param_1[0x1e7] == '\0') {
                local_14 = __ftol();
                iVar4 = __ftol();
                local_14 = local_14 + iVar4 * -3;
              }
              else {
                local_14 = __ftol();
                iVar4 = __ftol();
                local_14 = local_14 + iVar4;
              }
              if (((local_10 < 1) ||
                  (*(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4) <= local_10))
                 || ((local_14 < 1 ||
                     (*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8) <=
                      local_14)))) {
                (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
                (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
                (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
                (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                if (param_1[0x1f9] != 0) {
                  (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
                }
                (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
                if (DAT_005b892c == 2) {
                  (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
                }
                (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
                (**(code **)(*(int *)param_1[0x10b] + 0x60))(0);
                (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
                (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
                if (((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) &&
                   (*(char *)((int)param_1 + 0x245) != '\x01')) {
                  (**(code **)(*(int *)param_1[0x10f] + 0x60))(1);
                  (**(code **)(*(int *)param_1[0x110] + 0x60))(1);
                }
              }
              else {
                if (*(char *)((int)param_1 + 0x196) == '\0') {
                  uVar5 = (**(code **)(*DAT_0092e654 + 8))();
                  FUN_00423b00(((float)(uVar5 % 2000) * _DAT_005a4148) / _DAT_005a441c);
                  iVar4 = __ftol();
                  (**(code **)(*(int *)param_1[0x106] + 0x84))(iVar4 << 0x10 | 0xff000000);
                }
                (**(code **)(*(int *)param_1[0x106] + 0x60))(1);
                if (*(int *)(param_1[0x10f] + 0x28) == 1) {
                  (**(code **)(*(int *)param_1[0x110] + 0x60))(1);
                }
                if (((param_1[0x1f9] != 0) && ((short)param_1[0x220] != 0)) &&
                   (-1 < *(int *)(param_1[0x1f9] + 0x1c4))) {
                  iVar4 = *(int *)(param_1[0x1f9] + 0x1c4);
                  if (((iVar4 < 0) || (0x40 < iVar4)) ||
                     ((param_1[0x1fc] == (*(uint *)(DAT_0092e640 + 8 + iVar4 * 0xc) & 0xffff) &&
                      (*(int *)(DAT_0092e640 + 8 + iVar4 * 0xc) >> 0x10 == param_1[0x1fb])))) {
                    if (param_1[0x21e] == 0) {
                      (**(code **)(*(int *)param_1[0x1f9] + 0x60))(1);
                    }
                  }
                  else {
                    *(undefined4 *)(param_1[0x1f9] + 0x1c4) = 0xffffffff;
                    (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
                  }
                }
                (**(code **)(*(int *)param_1[0x10a] + 0x60))(1);
                if (*(char *)((int)param_1 + 0x245) == '\x01') {
                  (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
                }
                if (((uint)param_1[8] < 1000) &&
                   ((0 < *(short *)((int)param_1 + 0x192) || (0 < (short)param_1[0x65])))) {
                  (**(code **)(*(int *)param_1[0x107] + 0x60))(1);
                  iVar4 = __ftol();
                  if ((iVar4 >> 7 < 0x11) ||
                     ((iVar4 = __ftol(), 0x13 < iVar4 >> 7 || (iVar4 = __ftol(), iVar4 >> 7 < 0x1e))
                     )) {
                    bVar3 = false;
                  }
                  else {
                    bVar3 = true;
                  }
                  if ((bVar3) && (*(int **)(local_c + 0x4c) != param_1)) {
                    (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
                  }
                  iVar4 = __ftol();
                  if ((iVar4 >> 7 == 0x11) && (iVar4 = __ftol(), iVar4 >> 7 == 0x1c)) {
                    bVar3 = true;
                  }
                  else {
                    bVar3 = false;
                  }
                  if (bVar3) {
                    (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
                  }
                }
                iVar4 = __ftol();
                if (((iVar4 >> 7 < 0x11) || (iVar4 = __ftol(), 0x13 < iVar4 >> 7)) ||
                   (iVar4 = __ftol(), iVar4 >> 7 < 0x1e)) {
                  bVar3 = false;
                }
                else {
                  bVar3 = true;
                }
                if ((bVar3) && (*(int **)(local_c + 0x4c) != param_1)) {
                  if ((uint)param_1[8] < 1000) {
                    (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
                    (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
                    (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
                    if (param_1[0x1f9] != 0) {
                      (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
                    }
                    (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
                  }
                  else if (*(char *)((int)param_1 + 0x249) == '\0') {
                    (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
                    if (param_1[0x1f9] != 0) {
                      (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
                    }
                    (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
                  }
                }
                if ((char)param_1[0x1c9] == '\0') {
                  (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
                  if (DAT_005b892c == 2) {
                    (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
                  }
                }
                else {
                  (**(code **)(*(int *)param_1[0x109] + 0x60))(1);
                  if (DAT_005b892c == 2) {
                    (**(code **)(*(int *)param_1[0x108] + 0x60))(1);
                  }
                }
                if (param_1 != *(int **)(DAT_0067cf38 + 0x4c)) {
                  if ((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) {
                    (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
                    (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                  }
                  else {
                    (**(code **)(*(int *)param_1[0x10d] + 0x60))(1);
                    (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                  }
                }
                if (DAT_005b892c == 2) {
                  local_14 = __ftol();
                  local_44 = 1.0;
                  if ((char)param_1[0x1e7] == '\0') {
                    iVar4 = __ftol();
                    local_14 = local_14 - iVar4;
                  }
                  else {
                    iVar4 = __ftol();
                    local_14 = local_14 + iVar4;
                  }
                  if (DAT_005b4910 == (float)_DAT_005a4c28) {
                    if ((char)param_1[0x1e7] == '\0') {
                      iVar4 = __ftol();
                      local_14 = local_14 + iVar4;
                    }
                    else {
                      iVar4 = __ftol();
                      local_14 = local_14 - iVar4;
                    }
                  }
                  else if ((float)_DAT_005a5620 <= DAT_005b4910) {
                    if ((char)param_1[0x1e7] == '\0') {
                      iVar4 = __ftol();
                      local_14 = local_14 - iVar4;
                    }
                    else {
                      iVar4 = __ftol();
                      local_14 = local_14 + iVar4;
                    }
                  }
                }
                else {
                  local_44 = DAT_005b490c;
                  if ((DAT_005b490c == 0.8) && (local_44 = 1.05, (char)param_1[0x1e7] == '\0')) {
                    iVar4 = __ftol();
                    local_14 = local_14 + iVar4;
                  }
                  if ((char)param_1[0x1e7] != '\0') {
                    iVar4 = __ftol();
                    local_14 = local_14 - iVar4;
                  }
                  if ((float)_DAT_005a5618 <= DAT_005b4910) {
                    local_44 = 1.0;
                    if ((char)param_1[0x1e7] == '\0') {
                      iVar4 = __ftol();
                      local_14 = local_14 - iVar4;
                    }
                    else {
                      iVar4 = __ftol();
                      local_14 = local_14 + iVar4;
                    }
                    if (DAT_005b4910 == 1.28) {
                      local_10 = local_10 + -5;
                    }
                    else if ((float)_DAT_005a5610 < DAT_005b4910) {
                      local_10 = local_10 + -0x14;
                      if ((char)param_1[0x1e7] == '\0') {
                        iVar4 = __ftol();
                        local_14 = local_14 - iVar4;
                      }
                      else {
                        iVar4 = __ftol();
                        local_14 = local_14 + iVar4;
                      }
                    }
                  }
                }
                if ((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) {
                  (**(code **)(*(int *)param_1[0x10f] + 0x60))(1);
                  (**(code **)(*(int *)param_1[0x110] + 0x60))(1);
                }
                else {
                  if ((char)param_1[0x1e7] == '\0') {
                    FUN_004012c6(((float)local_10 - _DAT_005a40fc) - _DAT_005a3834,
                                 ((float)local_14 - _DAT_005a3670 * local_44) + _DAT_005a4104);
                  }
                  else {
                    FUN_004012c6(((float)local_10 - _DAT_005a40fc) - _DAT_005a3834,
                                 ((float)local_14 - _DAT_005a40fc * local_44) + _DAT_005a4104);
                  }
                  FUN_004012c6(((float)local_10 - _DAT_005a40fc) - _DAT_005a3834,
                               ((float)local_14 - _DAT_005a4318 * local_44) + _DAT_005a4104);
                }
                pcVar6 = (char *)(**(code **)(*(int *)param_1[0x106] + 0x88))();
                sVar7 = _strlen(pcVar6);
                _strlen((char *)(param_1 + 0x1c9));
                pcVar6 = (char *)(**(code **)(*(int *)param_1[0x10a] + 0x88))();
                sVar8 = _strlen(pcVar6);
                FUN_004012c6((float)local_10 - *(float *)(param_1[0x10b] + 0x54) / _DAT_005a365c,
                             (float)local_14 - _DAT_005a5574);
                if (DAT_005b892c == 2) {
                  FUN_004012c6((float)local_10 - (_DAT_005a4324 * local_44) / _DAT_005a365c,
                               (float)local_14);
                  FUN_004012c6((float)local_10 - (_DAT_005a4340 * local_44) / _DAT_005a365c,
                               (float)local_14 - _DAT_005a4304);
                }
                else {
                  FUN_004012c6((float)local_10 - (_DAT_005a4be0 * DAT_005b490c) / _DAT_005a40f0,
                               (float)local_14);
                }
                local_80 = (float)local_14;
                if (((char)param_1[0x1e7] != '\0') && (iVar4 = FUN_0040c0f0(), iVar4 == 0)) {
                  local_80 = local_80 - _DAT_005a4314;
                }
                if (DAT_005b892c == 2) {
                  local_44 = 1.0;
                }
                else {
                  local_44 = DAT_005b490c;
                }
                FUN_004012c6((float)local_10 -
                             ((float)(int)((sVar7 + 2) * 6) * local_44) / _DAT_005a365c,local_80);
                if (param_1[0x1f9] != 0) {
                  FUN_004012c6(((float)local_10 -
                               ((float)(int)((sVar7 + 2) * 6) * local_44) / _DAT_005a365c) -
                               _DAT_005a3670,local_80 + _DAT_005a365c);
                }
                FUN_004012c6((float)local_10 -
                             ((float)(int)((sVar8 + 2) * 6) * local_44) / _DAT_005a365c,
                             local_80 - _DAT_005a5608 * local_44);
                iVar4 = __ftol();
                if (((iVar4 >> 7 < 2) || (iVar4 = __ftol(), 10 < iVar4 >> 7)) ||
                   (iVar4 = __ftol(), 4 < iVar4 >> 7)) {
                  bVar2 = false;
                }
                else {
                  bVar2 = true;
                }
                if (bVar2) {
                  (**(code **)(*(int *)param_1[0x10d] + 0x60))(1);
                  (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                  (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
                  (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
                }
                else if (((((param_1[0x93] == 1) || (param_1[0x93] == 2)) ||
                          ((param_1[0x93] == 4 || ((param_1[0x93] == 8 || (param_1[0x93] == 0x1a))))
                          )) || ((param_1[0x93] == 0x21 && (*(short *)((int)param_1 + 0x1d2) == 0)))
                         ) || (((short)param_1[0x94] == 0x10f &&
                               ((*(byte *)((int)param_1 + 0x44a) & 0xf) != 0)))) {
                  (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
                  (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                  (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
                  (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
                }
                piVar1 = *(int **)(DAT_0067cf38 + 0x4c);
                if (piVar1 != (int *)0x0) {
                  if (((((((char)param_1[0x92] == '\x01') ||
                         (iVar4 = (**(code **)(*param_1 + 0x54))(), iVar4 == 1)) ||
                        (('\0' < *(char *)((int)piVar1 + 0x79b) &&
                         ('\0' < *(char *)((int)param_1 + 0x79b))))) ||
                       ((param_1 == piVar1 && ((char)param_1[0x1e7] == '\x01')))) &&
                      (iVar4 = FUN_00516105(), iVar4 == 0)) &&
                     (((((short)param_1[0x94] != 0x39 && (iVar4 = FUN_0040c0f0(), iVar4 == 0)) &&
                       ((**(code **)(*(int *)param_1[0x10d] + 0x60))(1),
                       param_1 != *(int **)(DAT_0067cf38 + 0x4c))) &&
                      ((*(int **)(DAT_0067cf38 + 0x7c) == param_1 && (!bVar3)))))) {
                    if ((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) {
                      (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
                    }
                    (**(code **)(*(int *)param_1[0x110] + 0x60))(1);
                    (**(code **)(*(int *)param_1[0x10f] + 0x60))(1);
                  }
                  if (((param_1 == piVar1) && ((char)param_1[0x1e7] == '\x01')) &&
                     (((short)param_1[0x68] != 0x1b &&
                      ((((short)param_1[0x68] != 0x1c && ((short)param_1[0x68] != 0x1d)) &&
                       ((short)param_1[0x68] != 0x1e)))))) {
                    (**(code **)(*(int *)param_1[0x10e] + 0x60))(1);
                  }
                  else {
                    (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                  }
                  if (((param_1 == piVar1) && ((char)param_1[0x1e7] == '\x01')) &&
                     ((0xf8b < *(short *)(DAT_013b71e8 + 0x7b8) &&
                      (*(short *)(DAT_013b71e8 + 0x7b8) < 4000)))) {
                    (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
                  }
                  *(undefined4 *)(param_1[0x10d] + 0x228) = 0xffff0000;
                  if (DAT_005d03f8 == 0) {
                    if (((('\0' < *(char *)((int)piVar1 + 0x79b)) &&
                         ('\0' < *(char *)((int)param_1 + 0x79b))) &&
                        (*(char *)((int)piVar1 + 0x79b) == *(char *)((int)param_1 + 0x79b))) &&
                       ((999 < (uint)param_1[8] && (DAT_005d0404 == 0)))) {
                      *(undefined4 *)(param_1[0x10d] + 0x228) = 0xff1e821e;
                    }
                  }
                  else {
                    if ((('\0' < *(char *)((int)piVar1 + 0x79b)) &&
                        ('\0' < *(char *)((int)param_1 + 0x79b))) &&
                       (*(char *)((int)piVar1 + 0x79b) == *(char *)((int)param_1 + 0x79b))) {
                      *(undefined4 *)(param_1[0x10d] + 0x228) = 0xff1e821e;
                    }
                    if (DAT_005d0404 == 1) {
                      *(undefined4 *)(param_1[0x10d] + 0x228) = 0xff1e821e;
                    }
                  }
                  iVar4 = __ftol();
                  iVar9 = __ftol();
                  if ((((int)(iVar4 + (iVar4 >> 0x1f & 0x7fU)) >> 7 == 0x13) &&
                      ((int)(iVar9 + (iVar9 >> 0x1f & 0x7fU)) >> 7 == 0xe)) &&
                     ((param_1 != piVar1 &&
                      (*(short *)((int)param_1 + 0x7ba) == *(short *)((int)piVar1 + 0x7ba))))) {
                    *(undefined4 *)(param_1[0x10d] + 0x228) = 0xff00ff00;
                    *(undefined4 *)(param_1[0x10f] + 0x228) = 0xff00ff00;
                  }
                  if (((param_1 == piVar1) || ((char)param_1[0x92] == '\x01')) ||
                     (((short)param_1[0x1a0] != 0 &&
                      (((short)param_1[0x1a0] == (short)piVar1[0x1a0] ||
                       (*(short *)(DAT_013b71e8 + 0xf12) == (short)param_1[0x1a0])))))) {
                    *(undefined4 *)(param_1[0x10d] + 0x228) = 0xff1e821e;
                    *(undefined4 *)(param_1[0x10f] + 0x228) = 0x80006400;
                  }
                }
              }
            }
          }
          else {
            (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
            (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
            (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
            (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
            if (param_1[0x1f9] != 0) {
              (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
            }
            (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
            if (DAT_005b892c == 2) {
              (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
            }
            (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
            (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
            (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
            if (((param_1[0x93] == 0x38) && (*(short *)((int)param_1 + 0x1d2) == 0)) &&
               (*(char *)((int)param_1 + 0x245) != '\x01')) {
              (**(code **)(*(int *)param_1[0x10f] + 0x60))(1);
              (**(code **)(*(int *)param_1[0x110] + 0x60))(1);
            }
          }
        }
        iVar4 = FUN_0040c0f0();
        if (iVar4 == 1) {
          (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
          (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
        }
        else if (((param_1[0x21f] == 0) && (sVar7 = _strlen((char *)(param_1 + 0x1c9)), sVar7 == 0))
                && (((short)param_1[0x220] != 0 &&
                    ((param_1[0x1fc] != -1 && (iVar4 = FUN_0040c0f0(), iVar4 == 0)))))) {
          FUN_0047dee5(param_1 + 0x1f9);
        }
        if (param_1[0x222] == 1) {
          (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
        }
      }
    }
  }
  return;
}

