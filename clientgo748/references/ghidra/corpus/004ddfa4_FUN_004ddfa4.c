// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ddfa4 | Name: FUN_004ddfa4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004ddfa4(int param_1,int param_2)

{
  float fVar1;
  int iVar2;
  void *pvVar3;
  float10 fVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_58;
  undefined4 local_34;
  undefined4 local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  float local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0d36;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x28) != 0) {
    local_18 = (float)(uint)(param_2 - *(int *)(param_1 + 0x88)) * _DAT_005a4214;
    ExceptionList = &local_10;
    for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
      if (*(float *)(param_1 + 0xb8 + local_14 * 0xc) <= _DAT_005a3534) {
        iVar2 = _rand();
        if (iVar2 % 5 == 0) {
          pvVar3 = operator_new(0x124);
          local_8 = 0;
          if (pvVar3 == (void *)0x0) {
            local_58 = 0;
          }
          else {
            local_58 = FUN_004cb900(10,700,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3a51b717,0);
          }
          local_8 = 0xffffffff;
          local_20 = local_58;
          if (local_58 != 0) {
            *(undefined4 *)(local_58 + 0x6c) = 1;
            *(undefined4 *)(local_58 + 0x74) = *(undefined4 *)(param_1 + 0xb4 + local_14 * 0xc);
            *(undefined4 *)(local_58 + 0x7c) = *(undefined4 *)(param_1 + 0xbc + local_14 * 0xc);
            FUN_00430f20(*(undefined4 *)(local_58 + 0x74),*(undefined4 *)(local_58 + 0x7c));
            iVar2 = FUN_0049c70f(local_34,local_30);
            local_2c = (float)iVar2;
            if (local_2c == 127.0) {
              local_2c = 0.0;
            }
            FUN_00430f20(*(undefined4 *)(local_20 + 0x74),*(undefined4 *)(local_20 + 0x7c));
            iVar2 = FUN_0049cf38(local_28,local_24,0xc2c80000,&local_2c);
            if (iVar2 == 1) {
              local_2c = local_2c + _DAT_005a4150;
            }
            *(float *)(local_20 + 0x78) = local_2c * _DAT_005a4214 + _DAT_005a4150;
            FUN_0054ac09(local_20);
            fVar4 = (float10)FUN_004d0150(&local_34);
            if ((((float)fVar4 < _DAT_005a3834) && (DAT_005ccf98 != 0)) &&
               (iVar2 = FUN_00429a6d(8), iVar2 != 0)) {
              uVar6 = 0;
              uVar5 = 0;
              FUN_00429a6d(8);
              FUN_0042ad2b(uVar5,uVar6);
            }
          }
        }
        local_1c = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
        if (local_1c != 0) {
          *(float *)(param_1 + 0xb8 + local_14 * 0xc) = *(float *)(local_1c + 0x30) + _DAT_005a3670;
          fVar1 = *(float *)(param_1 + 0x80) + _DAT_005a3660;
          iVar2 = _rand();
          *(float *)(param_1 + 0xb4 + local_14 * 0xc) = fVar1 - (float)(iVar2 % 5) / _DAT_005a3834;
          iVar2 = _rand();
          *(float *)(param_1 + 0xbc + local_14 * 0xc) =
               *(float *)(param_1 + 0x84) - (float)(iVar2 % 5) / _DAT_005a3834;
        }
      }
      else {
        *(float *)(param_1 + 0xb8 + local_14 * 0xc) =
             *(float *)(param_1 + 0xb8 + local_14 * 0xc) -
             local_18 * *(float *)(param_1 + 0x8c + local_14 * 4) * _DAT_005a34a0;
      }
    }
    *(int *)(param_1 + 0x88) = param_2;
  }
  ExceptionList = local_10;
  return 1;
}

