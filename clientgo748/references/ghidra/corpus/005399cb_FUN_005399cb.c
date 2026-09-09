// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005399cb | Name: FUN_005399cb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall FUN_005399cb(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  int iVar2;
  float10 fVar3;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  float local_6c;
  int local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  int local_58;
  undefined4 local_54;
  undefined1 local_50 [12];
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_28;
  float local_24;
  undefined4 local_20;
  undefined1 local_1c [12];
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_24 = 0.0;
  __ftol();
  local_38 = __ftol();
  __ftol();
  local_54 = __ftol();
  local_58 = __ftol();
  local_68 = __ftol();
  if ((((local_58 < 0) || (local_68 < 0)) || (0x40 < local_58)) || (0x40 < local_68)) {
    fVar3 = (float10)_DAT_005a44ac;
  }
  else {
    FUN_00430f50(0,0xc61c4000,0);
    FUN_00430f50(0,0xbf800000,0);
    FUN_00430f50(param_2,0x42c80000,param_3);
    FUN_00430f50(0,0,0);
    FUN_00430f50(0,0,0);
    FUN_00430f50(0,0,0);
    FUN_00430f50(0,0,0);
    if (((local_58 < 0x3f) && (local_68 < 0x3f)) && ((-1 < local_58 && (-1 < local_68)))) {
      puVar1 = (undefined4 *)
               FUN_00430f50((float)local_58 + (float)local_58 + *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + local_68 * 0x40) * 0xc) * _DAT_005a4214
                            ,(float)local_68 + (float)local_68 + *(float *)(param_1 + 0x20a2c));
      local_78 = *puVar1;
      local_74 = puVar1[1];
      local_70 = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)(local_58 + 1) + (float)(local_58 + 1) +
                            *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + 1 + local_68 * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)local_68 + (float)local_68 + *(float *)(param_1 + 0x20a2c));
      local_10 = *puVar1;
      local_c = puVar1[1];
      local_8 = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)local_58 + (float)local_58 + *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + (local_68 + 1) * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)(local_68 + 1) + (float)(local_68 + 1) +
                            *(float *)(param_1 + 0x20a2c));
      local_44 = *puVar1;
      local_40 = puVar1[1];
      local_3c = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)(local_58 + 1) + (float)(local_58 + 1) +
                            *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + 1 + (local_68 + 1) * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)(local_68 + 1) + (float)(local_68 + 1) +
                            *(float *)(param_1 + 0x20a2c));
      local_64 = *puVar1;
      local_60 = puVar1[1];
      local_5c = puVar1[2];
    }
    else if (local_58 == 0x3f) {
      puVar1 = (undefined4 *)
               FUN_00430f50(*(float *)(param_1 + 0x20a28) + 126.0,
                            (float)(int)*(char *)(param_1 + 0xcc + (local_68 * 0x40 + 0x3f) * 0xc) *
                            _DAT_005a4214,
                            (float)local_68 + (float)local_68 + *(float *)(param_1 + 0x20a2c));
      local_78 = *puVar1;
      local_74 = puVar1[1];
      local_70 = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)(local_58 + 1) + (float)(local_58 + 1) +
                            *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + local_68 * 0x40) * 0xc) * _DAT_005a4214
                            ,(float)local_68 + (float)local_68 + *(float *)(param_1 + 0x20a2c));
      local_10 = *puVar1;
      local_c = puVar1[1];
      local_8 = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)local_58 + (float)local_58 + *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + (local_68 + 1) * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)(local_68 + 1) + (float)(local_68 + 1) +
                            *(float *)(param_1 + 0x20a2c));
      local_44 = *puVar1;
      local_40 = puVar1[1];
      local_3c = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)(local_58 + 1) + (float)(local_58 + 1) +
                            *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + (local_68 + 1) * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)(local_68 + 1) + (float)(local_68 + 1) +
                            *(float *)(param_1 + 0x20a2c));
      local_64 = *puVar1;
      local_60 = puVar1[1];
      local_5c = puVar1[2];
    }
    else if (local_68 == 0x3f) {
      puVar1 = (undefined4 *)
               FUN_00430f50((float)local_58 + (float)local_58 + *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc + (local_58 + 0xfc0) * 0xc) *
                            _DAT_005a4214,*(float *)(param_1 + 0x20a2c) + 126.0);
      local_78 = *puVar1;
      local_74 = puVar1[1];
      local_70 = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)(local_58 + 1) + (float)(local_58 + 1) +
                            *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + 1 + local_68 * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)local_68 + (float)local_68 + *(float *)(param_1 + 0x20a2c));
      local_10 = *puVar1;
      local_c = puVar1[1];
      local_8 = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)local_58 + (float)local_58 + *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + local_68 * 0x40) * 0xc) * _DAT_005a4214
                            ,(float)(local_68 + 1) + (float)(local_68 + 1) +
                             *(float *)(param_1 + 0x20a2c));
      local_44 = *puVar1;
      local_40 = puVar1[1];
      local_3c = puVar1[2];
      puVar1 = (undefined4 *)
               FUN_00430f50((float)(local_58 + 1) + (float)(local_58 + 1) +
                            *(float *)(param_1 + 0x20a28),
                            (float)(int)*(char *)(param_1 + 0xcc +
                                                 (local_58 + 1 + local_68 * 0x40) * 0xc) *
                            _DAT_005a4214,
                            (float)(local_68 + 1) + (float)(local_68 + 1) +
                            *(float *)(param_1 + 0x20a2c));
      local_64 = *puVar1;
      local_60 = puVar1[1];
      local_5c = puVar1[2];
    }
    local_20 = 0;
    local_28 = 0;
    local_6c = 0.0;
    iVar2 = FUN_005649c7(&local_78,&local_10,&local_44,local_1c,local_50,&local_20,&local_28,
                         &local_6c);
    if (iVar2 == 1) {
      fVar3 = (float10)_DAT_005a40f8 - (float10)local_6c;
    }
    else {
      iVar2 = FUN_005649c7(&local_64,&local_44,&local_10,local_1c,local_50,&local_20,&local_28,
                           &local_6c);
      if (iVar2 == 1) {
        fVar3 = (float10)_DAT_005a40f8 - (float10)local_6c;
      }
      else if (((local_58 < 0) || (local_68 < 0)) || ((0x3f < local_58 || (0x3f < local_68)))) {
        fVar3 = (float10)_DAT_005a44ac;
      }
      else {
        if ((((local_58 < 0) || (0x3e < local_58)) || (local_68 < 0)) || (0x3e < local_68)) {
          if (local_58 == 0x3f) {
            local_24 = (float)(int)*(char *)(param_1 + 0xcc + ((local_68 + 1) * 0x40 + 0x3f) * 0xc)
                       + (float)(int)*(char *)(param_1 + 0xcc + ((local_68 + 1) * 0x40 + 0x3f) * 0xc
                                              ) +
                         (float)(int)*(char *)(param_1 + 0xcc + (local_68 * 0x40 + 0x3f) * 0xc) +
                         (float)(int)*(char *)(param_1 + 0xcc + (local_68 * 0x40 + 0x3f) * 0xc) +
                         local_24;
          }
          else if (local_68 == 0x3f) {
            local_24 = (float)(int)*(char *)(param_1 + 0xcc + (local_58 + 0xfc1) * 0xc) +
                       (float)(int)*(char *)(param_1 + 0xcc + (local_58 + 0xfc0) * 0xc) +
                       (float)(int)*(char *)(param_1 + 0xcc + (local_58 + 0xfc1) * 0xc) +
                       (float)(int)*(char *)(param_1 + 0xcc + (local_58 + 0xfc0) * 0xc) + local_24;
          }
        }
        else {
          local_24 = (float)(int)*(char *)(param_1 + 0xcc +
                                          (local_58 + 1 + (local_68 + 1) * 0x40) * 0xc) +
                     (float)(int)*(char *)(param_1 + 0xcc + (local_58 + (local_68 + 1) * 0x40) * 0xc
                                          ) +
                     (float)(int)*(char *)(param_1 + 0xcc + (local_58 + 1 + local_68 * 0x40) * 0xc)
                     + (float)(int)*(char *)(param_1 + 0xcc + (local_58 + local_68 * 0x40) * 0xc) +
                       local_24;
        }
        fVar3 = ((float10)local_24 * (float10)_DAT_005a4214) / (float10)_DAT_005a3834;
      }
    }
  }
  return fVar3;
}

