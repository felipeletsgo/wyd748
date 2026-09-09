// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004df705 | Name: FUN_004df705


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004df705(int param_1,float param_2,float param_3)

{
  int iVar1;
  float fVar2;
  float fVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  float10 fVar8;
  float10 fVar9;
  int local_5c;
  int local_58;
  int local_4c;
  int local_48;
  int local_40;
  int local_3c;
  int local_2c;
  int local_28;
  int local_1c;
  int local_14;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x84) == 0) {
    uVar4 = 0;
  }
  else {
    *(float *)(param_1 + 0x74) = param_2;
    *(float *)(param_1 + 0x7c) = param_3;
    *(undefined4 *)(param_1 + 0x78) = 0;
    local_8 = __ftol();
    local_c = __ftol();
    local_8 = local_8 - *(int *)(param_1 + 0x94) / 2;
    local_c = local_c - *(int *)(param_1 + 0x94) / 2;
    if ((_DAT_005a3534 <= param_2) ||
       (((float)(local_8 + 1) + (float)(local_8 + 1)) - param_2 <=
        param_2 - ((float)local_8 + (float)local_8))) {
      if ((_DAT_005a3534 < param_2) &&
         (((float)(local_8 + 2) + (float)(local_8 + 2)) - param_2 <
          param_2 - ((float)(local_8 + 1) + (float)(local_8 + 1)))) {
        local_8 = local_8 + 1;
      }
    }
    else {
      local_8 = local_8 + -1;
    }
    if ((param_3 < _DAT_005a3534) &&
       (param_3 - ((float)local_c + (float)local_c) <
        ((float)(local_c + 1) + (float)(local_c + 1)) - param_3)) {
      local_c = local_c + -1;
    }
    if ((_DAT_005a3534 < param_3) &&
       (((float)(local_c + 2) + (float)(local_c + 2)) - param_3 <
        param_3 - ((float)(local_c + 1) + (float)(local_c + 1)))) {
      local_c = local_c + 1;
    }
    iVar1 = *(int *)(DAT_0067cf38 + 0x40);
    if ((DAT_0067cf38 == 0) || (iVar1 == 0)) {
      for (local_48 = 0; local_48 < *(int *)(param_1 + 0x94) + 1; local_48 = local_48 + 1) {
        for (local_4c = 0; local_4c < *(int *)(param_1 + 0x94) + 1; local_4c = local_4c + 1) {
          *(float *)(*(int *)(param_1 + 0x84) +
                    (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(local_8 + local_4c) + (float)(local_8 + local_4c);
          *(float *)(*(int *)(param_1 + 0x84) + 8 +
                    (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(local_c + local_48) + (float)(local_c + local_48);
          fVar2 = (param_2 -
                  *(float *)(*(int *)(param_1 + 0x84) +
                            (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18)) /
                  ((float)*(int *)(param_1 + 0x94) + (float)*(int *)(param_1 + 0x94));
          fVar3 = -(param_3 -
                   *(float *)(*(int *)(param_1 + 0x84) + 8 +
                             (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18)) /
                  ((float)*(int *)(param_1 + 0x94) + (float)*(int *)(param_1 + 0x94));
          fVar8 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x98));
          fVar9 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x98));
          *(float *)(*(int *)(param_1 + 0x84) + 0x10 +
                    (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(((float10)(float)(-fVar8 * (float10)fVar2) - fVar9 * (float10)fVar3) -
                      (float10)_DAT_005a34a0);
          fVar8 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x98));
          fVar9 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x98));
          *(float *)(*(int *)(param_1 + 0x84) + 0x14 +
                    (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(((float10)(float)(fVar8 * (float10)fVar2) - fVar9 * (float10)fVar3) -
                      (float10)_DAT_005a34a0);
          local_58 = local_8 + local_4c;
          local_5c = local_c + local_48;
          if (local_58 == 0x40) {
            local_58 = 0x3f;
          }
          if (local_5c == 0x40) {
            local_5c = 0x3f;
          }
          if ((((-1 < local_58) && (local_58 < 0x40)) && (-1 < local_5c)) && (local_5c < 0x40)) {
            *(undefined4 *)
             (*(int *)(param_1 + 0x84) + 4 +
             (local_4c + local_48 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) = 0x3d4ccccd;
          }
        }
      }
    }
    else {
      iVar5 = *(int *)(DAT_0067cf38 + 0x40) + 0xcc;
      for (local_14 = 0; local_14 < *(int *)(param_1 + 0x94) + 1; local_14 = local_14 + 1) {
        for (local_1c = 0; local_1c < *(int *)(param_1 + 0x94) + 1; local_1c = local_1c + 1) {
          *(float *)(*(int *)(param_1 + 0x84) +
                    (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(local_8 + local_1c) + (float)(local_8 + local_1c);
          *(float *)(*(int *)(param_1 + 0x84) + 8 +
                    (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(local_c + local_14) + (float)(local_c + local_14);
          fVar2 = (param_2 -
                  *(float *)(*(int *)(param_1 + 0x84) +
                            (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18)) /
                  ((float)*(int *)(param_1 + 0x94) + (float)*(int *)(param_1 + 0x94));
          fVar3 = -(param_3 -
                   *(float *)(*(int *)(param_1 + 0x84) + 8 +
                             (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18)) /
                  ((float)*(int *)(param_1 + 0x94) + (float)*(int *)(param_1 + 0x94));
          fVar8 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x98));
          fVar9 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x98));
          *(float *)(*(int *)(param_1 + 0x84) + 0x10 +
                    (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(((float10)(float)(-fVar8 * (float10)fVar2) - fVar9 * (float10)fVar3) -
                      (float10)_DAT_005a34a0);
          fVar8 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x98));
          fVar9 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x98));
          *(float *)(*(int *)(param_1 + 0x84) + 0x14 +
                    (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
               (float)(((float10)(float)(fVar8 * (float10)fVar2) - fVar9 * (float10)fVar3) -
                      (float10)_DAT_005a34a0);
          local_28 = local_8 + *(int *)(iVar1 + 0x20a20) * -0x40 + local_1c;
          local_2c = local_c + *(int *)(iVar1 + 0x20a24) * -0x40 + local_14;
          if (local_28 == 0x40) {
            local_28 = 0x3f;
          }
          if (local_2c == 0x40) {
            local_2c = 0x3f;
          }
          if (((local_28 < 0) || (0x3f < local_28)) || ((local_2c < 0 || (0x3f < local_2c)))) {
            local_40 = 0;
            if (*(int *)(iVar1 + 0xac) == 0) {
              if (*(int *)(iVar1 + 0xb0) == 0) {
                if (*(int *)(iVar1 + 0xb4) == 0) {
                  if (*(int *)(iVar1 + 0xb8) != 0) {
                    local_40 = *(int *)(iVar1 + 0xb8);
                  }
                }
                else {
                  local_40 = *(int *)(iVar1 + 0xb4);
                }
              }
              else {
                local_40 = *(int *)(iVar1 + 0xb0);
              }
            }
            else {
              local_40 = *(int *)(iVar1 + 0xac);
            }
            if (local_40 != 0) {
              iVar6 = local_8 + *(int *)(local_40 + 0x20a20) * -0x40 + local_1c;
              iVar7 = local_c + *(int *)(local_40 + 0x20a24) * -0x40 + local_14;
              if (((-1 < iVar6) && (iVar6 < 0x40)) && ((-1 < iVar7 && (iVar7 < 0x40)))) {
                *(float *)(*(int *)(param_1 + 0x84) + 4 +
                          (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
                     (float)(int)*(char *)(local_40 + 0xcc + (iVar6 + iVar7 * 0x40) * 0xc) *
                     _DAT_005a4214 + _DAT_005a439c;
              }
            }
          }
          else {
            iVar6 = __ftol();
            iVar7 = __ftol();
            local_3c = (int)*(char *)(iVar1 + 0xc0cc + iVar7 + iVar6 * 0x80);
            if (10 < local_3c) {
              if (*(char *)(iVar1 + 0xc0cc + iVar7 + (iVar6 + 1) * 0x80) < '\n') {
                local_3c = (int)*(char *)(iVar1 + 0xc0cc + iVar7 + (iVar6 + 1) * 0x80);
              }
              else if (*(char *)(iVar1 + 0xc0cc + iVar7 + (iVar6 + -1) * 0x80) < '\n') {
                local_3c = (int)*(char *)(iVar1 + 0xc0cc + iVar7 + (iVar6 + -1) * 0x80);
              }
              else if (*(char *)(iVar1 + iVar6 * 0x80 + 0xc0cd + iVar7) < '\n') {
                local_3c = (int)*(char *)(iVar1 + iVar6 * 0x80 + 0xc0cd + iVar7);
              }
              else if (*(char *)(iVar1 + iVar6 * 0x80 + 0xc0cb + iVar7) < '\n') {
                local_3c = (int)*(char *)(iVar1 + iVar6 * 0x80 + 0xc0cb + iVar7);
              }
            }
            if (_DAT_005a4d40 <=
                (float)local_3c * _DAT_005a4214 -
                (float)(int)*(char *)(iVar5 + (local_28 + local_2c * 0x40) * 0xc) * _DAT_005a4214) {
              *(float *)(*(int *)(param_1 + 0x84) + 4 +
                        (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
                   (float)local_3c * _DAT_005a4214 + _DAT_005a439c;
            }
            else {
              *(float *)(*(int *)(param_1 + 0x84) + 4 +
                        (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
                   (float)(int)*(char *)(iVar5 + (local_28 + local_2c * 0x40) * 0xc) * _DAT_005a4214
                   + _DAT_005a439c;
            }
          }
          if ((*(int *)(DAT_0067cf38 + 0x24) == 0x7531) && (*(int *)(param_1 + 0x6c) == 1)) {
            if (*(float *)(*(int *)(param_1 + 0x84) + 4 +
                          (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) <=
                _DAT_005a4104) {
              *(float *)(*(int *)(param_1 + 0x84) + 4 +
                        (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) =
                   *(float *)(*(int *)(param_1 + 0x84) + 4 +
                             (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) +
                   _DAT_005a4214;
            }
            else {
              *(undefined4 *)
               (*(int *)(param_1 + 0x84) + 4 +
               (local_1c + local_14 * (*(int *)(param_1 + 0x94) + 1)) * 0x18) = 0x3f000000;
            }
          }
        }
      }
    }
    uVar4 = 1;
  }
  return uVar4;
}

