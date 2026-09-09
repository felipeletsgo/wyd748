// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d2e12 | Name: FUN_004d2e12


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d2e12(int param_1)

{
  float fVar1;
  undefined4 *puVar2;
  float10 fVar3;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  float local_60;
  uint local_5c;
  float local_58;
  int local_54;
  int local_50;
  uint local_4c;
  int local_48;
  float local_44;
  int local_40;
  int local_3c;
  uint local_38;
  int local_34;
  uint local_30;
  int local_2c;
  undefined1 local_28 [16];
  uint local_18;
  uint local_10;
  int local_c;
  int *local_8;
  
  if ((((((*(int *)(param_1 + 0x18) != -1) && (*(int *)(param_1 + 0x18) != DAT_005bdef0)) &&
        (*(int *)(param_1 + 0x18) != DAT_005bdef4)) &&
       ((*(int *)(param_1 + 0x18) != DAT_005bdef8 && (*(int *)(param_1 + 0x18) != DAT_005bdefc))))
      && ((*(int *)(param_1 + 0x18) != DAT_005bdf00 &&
          ((*(int *)(param_1 + 0x18) != DAT_005bdf04 && (*(int *)(param_1 + 0x18) != DAT_005bdf08)))
          ))) &&
     ((*(int *)(param_1 + 0x18) != DAT_005bdf0c && (*(int *)(param_1 + 0x18) != DAT_005bdf10)))) {
    if (*(int *)(param_1 + 0x84) != 0x1fa) {
      return 0;
    }
    if (*(int *)(DAT_0067cf38 + 0x24) != 0x7531) {
      return 0;
    }
  }
  if ((*(int *)(param_1 + 0x28) == 1) &&
     (local_8 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0x84),1,180000), local_8 != (int *)0x0
     )) {
    if (*(int *)(param_1 + 0x6c) == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
    }
    if (*(int *)(param_1 + 0x6c) == 3) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,2,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,3,0);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,6,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,2,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,3,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    if (*(int *)(param_1 + 0x6c) == 3) {
      _memset(&local_a4,0,0x44);
      FUN_004d3c10(*(undefined4 *)(param_1 + 0x88));
      puVar2 = (undefined4 *)FUN_00430f10();
      local_84 = *puVar2;
      local_80 = puVar2[1];
      local_7c = puVar2[2];
      local_78 = puVar2[3];
      FUN_004d3c10(*(undefined4 *)(param_1 + 0x88));
      puVar2 = (undefined4 *)FUN_00430f10();
      local_a4 = *puVar2;
      local_a0 = puVar2[1];
      local_9c = puVar2[2];
      local_98 = puVar2[3];
      FUN_004d3c10(*(undefined4 *)(param_1 + 0x88));
      puVar2 = (undefined4 *)FUN_00430f10();
      local_74 = *puVar2;
      local_70 = puVar2[1];
      local_6c = puVar2[2];
      local_68 = puVar2[3];
      FUN_004d3c10(*(undefined4 *)(param_1 + 0x88));
      puVar2 = (undefined4 *)FUN_00430f10();
      local_94 = *puVar2;
      local_90 = puVar2[1];
      local_8c = puVar2[2];
      local_88 = puVar2[3];
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_a4);
    }
    else {
      if (local_8[0xc6] == 0) {
        return 0;
      }
      (**(code **)(*(int *)local_8[0xc6] + 0x34))(local_8[0xc6],local_28);
      (**(code **)(*(int *)local_8[0xc6] + 0x2c))(local_8[0xc6],0,0,&local_c,0);
      local_30 = local_18 / 0x18;
      local_10 = *(uint *)(param_1 + 0x88);
      if (*(char *)(param_1 + 0xac) == '\x01') {
        fVar1 = *(float *)(param_1 + 0xa8) * _DAT_005a4148;
        fVar3 = (float10)FUN_00423b00(fVar1 + fVar1);
        local_44 = (float)(fVar3 * (float10)_DAT_005a4158 + (float10)_DAT_005a415c);
        local_34 = __ftol();
        local_3c = __ftol();
        local_40 = __ftol();
        local_38 = __ftol();
        local_10 = local_34 << 0x18 | local_3c << 0x10 | local_40 << 8 | local_38;
      }
      if (*(int *)(param_1 + 0xa0) == 4) {
        fVar3 = (float10)FUN_00423b00(*(float *)(param_1 + 0xa8) * _DAT_005a4148);
        local_58 = (float)fVar3;
        local_48 = __ftol();
        local_50 = __ftol();
        local_54 = __ftol();
        local_4c = __ftol();
        local_10 = local_48 << 0x18 | local_50 << 0x10 | local_54 << 8 | local_4c;
      }
      for (local_2c = 0; local_2c < (int)local_30; local_2c = local_2c + 1) {
        *(uint *)(local_c + 0xc + local_2c * 0x18) = local_10;
        if (*(char *)(param_1 + 0xad) == '\x01') {
          *(float *)(local_c + 0x10 + local_2c * 0x18) =
               *(float *)(param_1 + 0xa8) * _DAT_005a41ec +
               *(float *)(local_c + 0x10 + local_2c * 0x18);
        }
        if (*(char *)(param_1 + 0xad) == '\x02') {
          local_5c = (**(code **)(*DAT_0092e654 + 8))();
          local_60 = (float)(local_5c % 100) / _DAT_005a40f8;
          *(float *)(local_c + 0x14 + local_2c * 0x18) =
               local_60 * _DAT_005a4214 + *(float *)(local_c + 0x14 + local_2c * 0x18);
        }
      }
      (**(code **)(*(int *)local_8[0xc6] + 0x30))(local_8[0xc6]);
    }
    local_8[0xcb] = *(int *)(param_1 + 0x98);
    local_8[0xcc] = *(int *)(param_1 + 0x9c);
    if (*(int *)(param_1 + 0xa0) == 1) {
      local_8[0xa6] = *(int *)(param_1 + 0xa4);
    }
    else if (*(int *)(param_1 + 0xa0) == 2) {
      local_8[0xcc] = (int)(*(float *)(param_1 + 0x98) * *(float *)(param_1 + 0xa8) * _DAT_005a40f0)
      ;
      *(float *)(param_1 + 0x8c) = *(float *)(param_1 + 0xa8) * _DAT_005a4148;
    }
    else if (*(int *)(param_1 + 0xa0) == 3) {
      *(float *)(param_1 + 0x8c) = *(float *)(param_1 + 0xa8) * _DAT_005a4148;
    }
    else if (*(int *)(param_1 + 0xa0) == 4) {
      local_8[0xa6] = *(int *)(param_1 + 0xa4);
      if (_DAT_005a4158 <= *(float *)(param_1 + 0xa8)) {
        fVar3 = (float10)FUN_00423b00((*(float *)(param_1 + 0xa8) - _DAT_005a4158) * _DAT_005a4148 *
                                      _DAT_005a34a0);
        local_8[0xcb] =
             (int)(float)((fVar3 + (float10)_DAT_005a430c) * (float10)*(float *)(param_1 + 0x98));
        fVar3 = (float10)FUN_00423b00((*(float *)(param_1 + 0xa8) - _DAT_005a4158) * _DAT_005a4148 *
                                      _DAT_005a34a0);
        local_8[0xcc] =
             (int)(float)((fVar3 + (float10)_DAT_005a430c) * (float10)*(float *)(param_1 + 0x9c));
      }
      else {
        local_8[0xcb] =
             (int)((*(float *)(param_1 + 0xa8) * _DAT_005a4104 + _DAT_005a34a0) *
                  *(float *)(param_1 + 0x98));
        local_8[0xcc] =
             (int)((*(float *)(param_1 + 0xa8) * _DAT_005a4104 + _DAT_005a34a0) *
                  *(float *)(param_1 + 0x9c));
      }
    }
    else if (*(int *)(param_1 + 0xa0) == 5) {
      local_8[0xa6] = *(int *)(param_1 + 0xa4);
    }
    (**(code **)(*local_8 + 8))
              (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
               *(undefined4 *)(param_1 + 0x7c),*(undefined4 *)(param_1 + 0x8c),
               *(undefined4 *)(param_1 + 0x90),*(undefined4 *)(param_1 + 0x94),0,0);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
  }
  return 1;
}

