// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dd68d | Name: FUN_004dd68d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004dd68d(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  int local_68;
  undefined4 local_4c;
  undefined4 local_48;
  float local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  int local_30;
  byte local_2c;
  undefined3 uStack_2b;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0d0e;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x28) != 0) {
    local_28 = (float)(uint)(param_2 - *(int *)(param_1 + 0x80)) * _DAT_005a4214;
    ExceptionList = &local_10;
    FUN_00430f20(0,0);
    local_14 = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
    if (local_14 == 0) {
      local_24 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x78) + _DAT_005a4bd8;
      local_20 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x7c) + _DAT_005a4bd8;
    }
    else {
      local_24 = *(float *)(local_14 + 0x28);
      local_20 = *(float *)(local_14 + 0x2c);
    }
    local_1c = 0.0;
    if (local_14 != 0) {
      local_1c = *(float *)(local_14 + 0x30);
    }
    for (local_18 = 0; local_18 < 0x32; local_18 = local_18 + 1) {
      if (*(float *)(param_1 + 0x150 + local_18 * 0xc) <= local_1c) {
        iVar1 = _rand();
        if (iVar1 % 5 == 0) {
          iVar1 = __ftol();
          local_34 = (int)(iVar1 + (iVar1 >> 0x1f & 3U)) >> 2;
          iVar1 = __ftol();
          local_30 = (int)(iVar1 + (iVar1 >> 0x1f & 3U)) >> 2;
          _local_2c = CONCAT31(uStack_2b,(&DAT_00934178)[local_34 + local_30 * 0x400]);
          if (((&DAT_00934178)[local_34 + local_30 * 0x400] & 8) == 0) {
            pvVar2 = operator_new(0x124);
            local_8 = 0;
            if (pvVar2 == (void *)0x0) {
              local_68 = 0;
            }
            else {
              local_68 = FUN_004cb900(10,700,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x39d1b717,0);
            }
            local_8 = 0xffffffff;
            local_38 = local_68;
            if (local_68 != 0) {
              *(undefined4 *)(local_68 + 0x6c) = 1;
              *(float *)(local_68 + 0x74) = local_24 + *(float *)(param_1 + 0x14c + local_18 * 0xc);
              *(float *)(local_68 + 0x7c) = local_20 + *(float *)(param_1 + 0x154 + local_18 * 0xc);
              FUN_00430f20(*(undefined4 *)(local_68 + 0x74),*(undefined4 *)(local_68 + 0x7c));
              iVar1 = FUN_0049c70f(local_4c,local_48);
              local_44 = (float)iVar1;
              if (local_44 == 127.0) {
                local_44 = 0.0;
              }
              FUN_00430f20(*(undefined4 *)(local_38 + 0x74),*(undefined4 *)(local_38 + 0x7c));
              iVar1 = FUN_0049cf38(local_40,local_3c,0xc2c80000,&local_44);
              if (iVar1 == 1) {
                local_44 = local_44 + _DAT_005a4150;
              }
              *(float *)(local_38 + 0x78) = local_44 * _DAT_005a4214 + _DAT_005a4150;
              FUN_0054ac09(local_38);
            }
          }
        }
        if (local_14 != 0) {
          *(float *)(param_1 + 0x150 + local_18 * 0xc) = *(float *)(local_14 + 0x30) + _DAT_005a3670
          ;
        }
        iVar1 = _rand();
        *(float *)(param_1 + 0x14c + local_18 * 0xc) =
             _DAT_005a3794 - (float)(iVar1 % 0x18) / _DAT_005a365c;
        iVar1 = _rand();
        *(float *)(param_1 + 0x154 + local_18 * 0xc) =
             _DAT_005a3794 - (float)(iVar1 % 0x18) / _DAT_005a365c;
      }
      else {
        *(float *)(param_1 + 0x150 + local_18 * 0xc) =
             *(float *)(param_1 + 0x150 + local_18 * 0xc) -
             local_28 * *(float *)(param_1 + 0x84 + local_18 * 4);
      }
    }
    *(int *)(param_1 + 0x80) = param_2;
  }
  ExceptionList = local_10;
  return 1;
}

