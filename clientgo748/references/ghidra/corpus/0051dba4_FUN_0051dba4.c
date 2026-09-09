// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051dba4 | Name: FUN_0051dba4


void __thiscall FUN_0051dba4(int *param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_40;
  float local_2c;
  int local_18;
  int local_8;
  
  if (param_1[0x85] != 0) {
    return;
  }
  if (param_2 != -1) {
    if (param_1[0x18] == 0) {
      return;
    }
    if ((param_2 == 0xe) && ((param_1[0x93] == 0x22 || (param_1[0x93] == 0x26)))) {
      param_2 = 1;
    }
    if (((param_2 == 0x17) || (param_2 == 0x13)) &&
       ((param_1[0x17] == 3 ||
        ((((param_1[0x17] == 8 || (param_1[0x17] == 7)) || (param_1[0x17] == 0x19)) ||
         (param_1[0x17] == 0x1c)))))) {
      param_2 = 0xe;
    }
    iVar1 = (**(code **)(*param_1 + 0x50))();
    if (((iVar1 == 1) && (param_2 == 1)) && (param_1[0x97] == 0)) {
      param_2 = 0;
    }
    if (((param_2 == 0) || (param_2 == 1)) &&
       ((param_1[0x8f] = 0, '\0' < *(char *)((int)param_1 + 0x79b) && (param_1[0x66] != 0)))) {
      if ((char)param_1[0x1e7] < '\x01') {
        FUN_004bd906(0);
      }
      else {
        FUN_004bd906(3);
      }
    }
    if (((param_1[0x17] == 0x1f) && (3 < param_2)) && (param_2 < 7)) {
      param_2 = param_2 + 3;
    }
    if (((param_1[0x17] == 0x15) && (1 < *(ushort *)((int)param_1 + 0x1d2))) && (param_2 == 2)) {
      param_2 = 3;
    }
    if (((param_1[0x17] == 0x18) && ((char)param_1[0x92] == '\x01')) && (param_2 == 2)) {
      param_2 = 3;
    }
    if ((((param_1[0x17] == 0x14) && (*(short *)((int)param_1 + 0x1d2) != 0)) &&
        (*(short *)((int)param_1 + 0x1d6) != 2)) && (param_2 == 3)) {
      param_2 = 2;
    }
    if (((param_1[0x17] == 0x14) && (*(short *)((int)param_1 + 0x1d2) == 4)) && (param_2 == 2)) {
      iVar1 = __ftol();
      *(int *)(param_1[0x18] + 0x2e4) = iVar1 / 2;
      uVar2 = __ftol();
      *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
    }
    if (((param_1[0x17] == 0x14) && ((param_2 == 2 || (param_2 == 3)))) &&
       (*(short *)((int)param_1 + 0x1d2) == 7)) {
      *(undefined4 *)(param_1[0x18] + 0x2e4) = 6;
    }
    if (((((param_1[0x17] == 0) || (param_1[0x17] == 1)) || (param_1[0x17] == 2)) ||
        ((param_1[0x17] == 3 || (param_1[0x17] == 4)))) || (param_1[0x17] == 5)) {
      if ((char)param_1[0x1e7] == '\x01') {
        if (param_1[0x67] != 0) {
          if ((param_1[0x1e8] == 0x14) &&
             (((param_2 == 2 || (param_2 == 3)) && (*(short *)(param_1[0x67] + 0x248) == 7)))) {
            FUN_004bd906(*(undefined4 *)(&DAT_00672498 + param_1[0x1e8] * 0x2a0));
          }
          else if ((param_2 < 4) || (9 < param_2)) {
            FUN_004bd906(*(undefined4 *)(&DAT_00672490 + param_2 * 4 + param_1[0x1e8] * 0x2a0));
          }
          else {
            FUN_004bd906(*(undefined4 *)(&DAT_00672494 + param_1[0x1e8] * 0x2a0));
          }
        }
        if (((short)param_1[0x94] < 0x28) &&
           (((int)(short)param_1[0x94] % 10 == 1 || (5 < (int)(short)param_1[0x94] % 10)))) {
          if ((int)(short)param_1[0x94] % 10 == 1) {
            local_8 = (int)(short)param_1[0x94] / 10;
          }
          else {
            local_8 = (int)(short)param_1[0x94] % 10 + -6;
          }
          if (3 < local_8) {
            local_8 = 0;
          }
          FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                      *(int *)(local_8 * 0x380 + param_1[0x17] * 0x1c0 + 6800000 +
                                              param_2 * 4) * 2 +
                                      param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
        }
        else if (param_1[0x17] == 3) {
          FUN_004bd906(*(undefined4 *)(&DAT_00672500 + param_2 * 4 + param_1[0x17] * 0x2a0));
        }
        else {
          FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                      *(int *)(&DAT_00672500 + param_2 * 4 + param_1[0x17] * 0x2a0)
                                      * 2 + param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
        }
      }
      else if (((short)param_1[0x94] < 0x28) &&
              (((int)(short)param_1[0x94] % 10 == 1 || (5 < (int)(short)param_1[0x94] % 10)))) {
        if ((int)(short)param_1[0x94] % 10 == 1) {
          local_18 = (int)(short)param_1[0x94] / 10;
        }
        else {
          local_18 = (int)(short)param_1[0x94] % 10 + -6;
        }
        if (3 < local_18) {
          local_18 = 0;
        }
        iVar1 = FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                            *(int *)(&DAT_0067c210 +
                                                    param_2 * 4 +
                                                    local_18 * 0x380 + param_1[0x17] * 0x1c0) * 2 +
                                            param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
        if ((iVar1 == 0) &&
           (*(int *)(&DAT_0067c210 + param_2 * 4 + local_18 * 0x380 + param_1[0x17] * 0x1c0) == 0))
        {
          param_1[0x1d0] = param_2;
          return;
        }
      }
      else {
        iVar1 = FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                            *(int *)(&DAT_00672490 +
                                                    param_2 * 4 + param_1[0x17] * 0x2a0) * 2 +
                                            param_1[0x97] * 0x70 + param_1[0x17] * 0x1a40));
        if ((iVar1 == 0) && (*(int *)(&DAT_00672490 + param_2 * 4 + param_1[0x17] * 0x2a0) == 0)) {
          param_1[0x1d0] = param_2;
          return;
        }
      }
      param_1[0x1d0] = param_2;
      if (param_2 == 3) {
        iVar1 = __ftol();
        *(int *)(param_1[0x18] + 0x2e4) = iVar1 / 2;
        uVar2 = __ftol();
        *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        if (('\0' < *(char *)((int)param_1 + 0x79b)) && (param_1[0x66] != 0)) {
          if ((char)param_1[0x1e7] < '\x01') {
            FUN_004bd906(2);
          }
          else {
            FUN_004bd906(3);
          }
        }
        if (((char)param_1[0x1e7] == '\x01') && (param_1[0x67] != 0)) {
          if (param_1[0x1e8] != 0x28) {
            iVar1 = __ftol();
            *(int *)(param_1[0x67] + 0x2e4) = iVar1 / 2;
          }
          if (param_1[0x1e8] == 0x14) {
            *(undefined4 *)(param_1[0x67] + 0x2e4) = 0x14;
          }
          if (param_1[0x1e8] == 0x27) {
            *(undefined4 *)(param_1[0x67] + 0x2e4) = 0x14;
          }
          uVar2 = __ftol();
          *(undefined4 *)(param_1[0x67] + 0x2e4) = uVar2;
        }
        if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28f0) = 0;
        }
        if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28f0) = 0;
        }
        if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28f4) = 0;
        }
        if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28f4) = 0;
        }
      }
      else if (param_2 == 2) {
        if (('\0' < *(char *)((int)param_1 + 0x79b)) && (param_1[0x66] != 0)) {
          if ((char)param_1[0x1e7] < '\x01') {
            FUN_004bd906(1);
          }
          else {
            FUN_004bd906(3);
          }
        }
        if (param_1[0x17] == 2) {
          iVar1 = __ftol();
          *(int *)(param_1[0x18] + 0x2e4) = iVar1 / 2;
        }
        else {
          iVar1 = __ftol();
          *(int *)(param_1[0x18] + 0x2e4) = iVar1 / 2;
        }
        uVar2 = __ftol();
        *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        if (((char)param_1[0x1e7] == '\x01') && (param_1[0x67] != 0)) {
          iVar1 = __ftol();
          *(int *)(param_1[0x67] + 0x2e4) = iVar1 / 2;
          uVar2 = __ftol();
          *(undefined4 *)(param_1[0x67] + 0x2e4) = uVar2;
        }
        if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28f0) = 0;
        }
        if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28f0) = 0;
        }
        if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28f4) = 0;
        }
        if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
          *(undefined1 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28f4) = 0;
        }
      }
      else {
        if (('\0' < *(char *)((int)param_1 + 0x79b)) && (param_1[0x66] != 0)) {
          if ((char)param_1[0x1e7] < '\x01') {
            FUN_004bd906(1);
          }
          else {
            FUN_004bd906(3);
          }
        }
        uVar2 = __ftol();
        *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        if (((char)param_1[0x1e7] == '\x01') && (param_1[0x67] != 0)) {
          uVar2 = __ftol();
          *(undefined4 *)(param_1[0x67] + 0x2e4) = uVar2;
        }
      }
      if ((param_2 < 4) || (9 < param_2)) {
        param_1[0x1d6] = 0;
      }
      else {
        local_2c = 1.0;
        if ((char)param_1[0x1e7] == '\x01') {
          local_2c = 1.2;
        }
        uVar2 = __ftol();
        *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        if (param_1[0x19c] == 1) {
          if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
            if ((param_1[0x58] == 0x29) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
              *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                   local_2c * (float)param_1[0x19f];
            }
            else {
              *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) =
                   local_2c * (float)param_1[0x19e];
            }
          }
          if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
            uVar2 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x80) = uVar2;
          }
        }
        if (param_1[0x19d] == 1) {
          if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
            *(float *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28e8) = local_2c * (float)param_1[0x19f];
          }
          if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
            uVar2 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x80) = uVar2;
          }
        }
        uVar2 = __ftol();
        *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        if (*(char *)((int)param_1 + 0x77f) == '\x01') {
          uVar2 = __ftol();
          *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        }
        if (param_1[0x1d6] < 2) {
          if (param_1[0x1d9] == 1) {
            if (param_1[0x58] == 0x65) {
              uVar2 = __ftol();
              *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
            }
            else {
              uVar2 = __ftol();
              *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
            }
          }
        }
        else {
          uVar2 = __ftol();
          *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        }
      }
      if (param_2 == 0xe) {
        if ((param_1[0x17] == 2) && (param_1[0x97] == 4)) {
          uVar2 = __ftol();
          *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
        }
        if ((((param_1 == *(int **)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54)) &&
             ((char)param_1[0x1e7] == '\x01')) && (param_1[0x1e8] == 0x1f)) &&
           ((DAT_005ccf98 != 0 && (iVar1 = FUN_00429a6d(0x117), iVar1 != 0)))) {
          uVar3 = 0;
          uVar2 = 0;
          FUN_00429a6d(0x117);
          FUN_0042ad2b(uVar2,uVar3);
        }
        if (param_1 == *(int **)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54)) {
          if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x9e), iVar1 != 0)) {
            uVar3 = 0;
            uVar2 = 0;
            FUN_00429a6d(0x9e);
            FUN_0042ad2b(uVar2,uVar3);
          }
        }
        else {
          local_40 = *(undefined4 *)(&DAT_006725a8 + param_1[0x17] * 0x2a0);
          if ((param_1[0x17] == 2) &&
             ((*(short *)((int)param_1 + 0x1d2) == 5 || (*(short *)((int)param_1 + 0x1d2) == 6)))) {
            local_40 = 0xe1;
          }
          if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(local_40), iVar1 != 0)) {
            uVar3 = 0;
            uVar2 = 0;
            FUN_00429a6d(local_40);
            FUN_0042ad2b(uVar2,uVar3);
          }
        }
      }
    }
    else {
      if ((3 < param_2) && (param_2 < 10)) {
        if (param_1[0x19c] == 1) {
          if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
            if ((param_1[0x58] == 0x29) && (*(int *)(param_1[0x18] + 0x2fc) != 0)) {
              *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) = (float)param_1[0x19f] * 1.0;
            }
            else {
              *(float *)(*(int *)(param_1[0x18] + 0x2f8) + 0x28e8) = (float)param_1[0x19e] * 1.0;
            }
          }
          if (*(int *)(param_1[0x18] + 0x2f8) != 0) {
            uVar2 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(*(int *)(param_1[0x18] + 0x2f8) + 0x80) = uVar2;
          }
        }
        if (param_1[0x19d] == 1) {
          if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
            *(float *)(*(int *)(param_1[0x18] + 0x2fc) + 0x28e8) = (float)param_1[0x19f] * 1.0;
          }
          if (*(int *)(param_1[0x18] + 0x2fc) != 0) {
            uVar2 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(*(int *)(param_1[0x18] + 0x2fc) + 0x80) = uVar2;
          }
        }
      }
      if ((((param_1[0x17] == 0x25) && (param_2 == 0xe)) && (DAT_005ccf98 != 0)) &&
         (iVar1 = FUN_00429a6d(0x126), iVar1 != 0)) {
        uVar3 = 0;
        uVar2 = 0;
        FUN_00429a6d(0x126);
        FUN_0042ad2b(uVar2,uVar3);
      }
      if ((param_2 == 3) || (param_2 == 2)) {
        iVar1 = __ftol();
        *(int *)(param_1[0x18] + 0x2e4) = iVar1 / 2;
        if (param_1[0x17] == 0x28) {
          *(undefined4 *)(param_1[0x18] + 0x2e4) = 0x14;
        }
        if ((param_1[0x17] == 0x14) && (param_2 == 3)) {
          *(undefined4 *)(param_1[0x18] + 0x2e4) = 0x14;
        }
        if ((param_1[0x17] == 0x27) && (param_2 == 3)) {
          *(undefined4 *)(param_1[0x18] + 0x2e4) = 0x14;
        }
        uVar2 = __ftol(*(undefined4 *)(param_1[0x18] + 0x2e4),0);
        *(undefined4 *)(param_1[0x18] + 0x2e4) = uVar2;
      }
      if ((char)param_1[0x1e7] == '\x01') {
        if (param_1[0x67] != 0) {
          *(undefined4 *)(param_1[0x67] + 0x2e4) = *(undefined4 *)(param_1[0x18] + 0x2e4);
          FUN_004bd906(*(undefined4 *)(&DAT_00672490 + param_2 * 4 + param_1[0x1e8] * 0x2a0));
        }
        FUN_004bd906(*(undefined4 *)
                      (&DAT_00672490 +
                      (param_2 + (char)param_1[0x1e7] * 0x1c) * 4 + param_1[0x17] * 0x2a0));
      }
      else {
        iVar1 = FUN_004bd906(*(undefined4 *)(&DAT_00672490 + param_2 * 4 + param_1[0x17] * 0x2a0));
        if (iVar1 == 0) {
          if (*(int *)(&DAT_00672490 + param_2 * 4 + param_1[0x17] * 0x2a0) != 0) {
            return;
          }
          param_1[0x1d0] = param_2;
          return;
        }
      }
      param_1[0x1d0] = param_2;
      *(undefined4 *)(param_1[0x18] + 0x2e4) =
           *(undefined4 *)(&DAT_006723b0 + param_2 * 4 + param_1[0x17] * 0x2a0);
      if (((char)param_1[0x1e7] == '\x01') && (param_1[0x67] != 0)) {
        *(undefined4 *)(param_1[0x67] + 0x2e4) =
             *(undefined4 *)(&DAT_006723b0 + param_2 * 4 + param_1[0x1e8] * 0x2a0);
      }
    }
    param_1[0x17a] = param_3;
    iVar1 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x180] = iVar1;
    if ((char)param_1[0x1e0] == '\x01') {
      iVar1 = (**(code **)(*DAT_0092e654 + 8))();
      param_1[0x188] = iVar1;
    }
    return;
  }
  return;
}

