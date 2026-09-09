// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004eeb0d | Name: FUN_004eeb0d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004eeb0d(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  void *pvVar6;
  float10 fVar7;
  int *local_94;
  float local_70;
  int local_34;
  undefined1 local_30 [16];
  uint local_20;
  int local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a15ae;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  param_1[0x24] = (int)((float)(uint)(iVar1 - param_1[0x20]) / (float)param_1[0x22]);
  if (_DAT_005a3660 < (float)param_1[0x24]) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  else {
    iVar2 = FUN_004b14fd(0x2bf,1,180000);
    if (iVar2 == 0) {
      ExceptionList = local_10;
      return 0;
    }
    (**(code **)(**(int **)(iVar2 + 0x318) + 0x34))(*(undefined4 *)(iVar2 + 0x318),local_30);
    (**(code **)(**(int **)(iVar2 + 0x318) + 0x2c))(*(undefined4 *)(iVar2 + 0x318),0,0,&local_18,0);
    fVar7 = (float10)FUN_00423b00((float)param_1[0x24] * _DAT_005a4148);
    if (fVar7 <= (float10)_DAT_005a3534) {
      fVar7 = (float10)FUN_00423b00((float)param_1[0x24] * _DAT_005a4148);
      fVar7 = -fVar7;
    }
    else {
      fVar7 = (float10)FUN_00423b00((float)param_1[0x24] * _DAT_005a4148);
    }
    local_70 = (float)fVar7;
    local_14 = local_70;
    for (local_34 = 0; local_34 < (int)(local_20 / 0x18); local_34 = local_34 + 1) {
      __ftol();
      iVar3 = __ftol();
      iVar4 = __ftol();
      uVar5 = __ftol();
      *(uint *)(local_18 + 0xc + local_34 * 0x18) = iVar3 << 0x10 | iVar4 << 8 | uVar5;
    }
    (**(code **)(**(int **)(iVar2 + 0x318) + 0x30))(*(undefined4 *)(iVar2 + 0x318));
    *(undefined4 *)(iVar2 + 0x298) = 0x3a;
    if (100 < (uint)(iVar1 - param_1[0x21])) {
      pvVar6 = operator_new(0x124);
      local_8 = 0;
      if (pvVar6 == (void *)0x0) {
        local_94 = (int *)0x0;
      }
      else {
        local_94 = (int *)FUN_004cb900(0x5e,700,0x3fc00000,0x3fc00000,0x3fc00000,0,0);
      }
      local_8 = 0xffffffff;
      if (local_94 != (int *)0x0) {
        local_94[0x1b] = 1;
        (**(code **)(*local_94 + 0x3c))(param_1[0x26]);
        FUN_004310a0(param_1 + 0x1d);
        local_94[0x1e] = (int)((float)param_1[0x24] + (float)param_1[0x24] + (float)local_94[0x1e]);
        FUN_0054ac09(local_94);
      }
      param_1[0x21] = iVar1;
    }
    if (param_1[0x25] == 1) {
      *(float *)(iVar2 + 0x330) = (float)param_1[0x24] * _DAT_005a4160 + _DAT_005a4158;
      *(undefined4 *)(iVar2 + 0x32c) = *(undefined4 *)(iVar2 + 0x330);
    }
    else {
      *(float *)(iVar2 + 0x330) = local_14 * _DAT_005a34a0 + _DAT_005a34a0;
      *(undefined4 *)(iVar2 + 0x32c) = *(undefined4 *)(iVar2 + 0x330);
    }
    param_1[0x23] = (int)((float)param_1[0x24] * _DAT_005a4148);
  }
  ExceptionList = local_10;
  return 1;
}

