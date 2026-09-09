// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004eced4 | Name: FUN_004eced4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004eced4(int *param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  int *local_80;
  undefined1 local_54 [12];
  undefined1 local_48 [12];
  undefined1 local_3c [12];
  undefined1 local_30 [12];
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  int *local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a147e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  local_14 = (float)(uint)(iVar1 - param_1[0x26]) / (float)param_1[0x27];
  if ((_DAT_005a34a0 < local_14) && (param_1[7] != 0)) {
    param_1[7] = 0;
  }
  if (_DAT_005a3660 < local_14) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  else if (100 < (uint)(iVar1 - param_1[0x28])) {
    if (param_1[7] == 0) {
      if ((float)_DAT_005a45c8 <= local_14) {
        FUN_004310a0(param_1 + 0x23);
      }
      else {
        uVar3 = 0x40000000;
        puVar4 = local_48;
        FUN_004c8c60(local_3c,local_14);
        uVar3 = FUN_004c8c60(puVar4,uVar3);
        puVar4 = local_54;
        FUN_004c8c60(local_30,_DAT_005a3660 - (local_14 + local_14));
        uVar3 = FUN_00493d50(puVar4,uVar3);
        FUN_004310a0(uVar3);
      }
    }
    else {
      param_1[0x1d] = *(int *)(param_1[7] + 0x28);
      param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
      param_1[0x1e] = *(int *)(param_1[7] + 0x30);
    }
    if (DAT_0092e660 == 0) {
      local_20 = _rand();
      local_1c = 2;
      local_24 = 0x3d4ccccd;
      if (param_1[0x29] == 2) {
        local_24 = 0x3ca3d70a;
      }
      pvVar2 = operator_new(0x180);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_80 = (int *)0x0;
      }
      else {
        local_80 = (int *)FUN_004c8f70(local_1c,2000,local_24,local_24,local_24,0x38d1b717,1,0x50);
      }
      local_8 = 0xffffffff;
      local_18 = local_80;
      if (local_80 == (int *)0x0) {
        ExceptionList = local_10;
        return 1;
      }
      local_80[0x1b] = 1;
      uVar3 = FUN_00430f50((float)(local_20 % 5) * _DAT_005a4214 + (float)param_1[0x1d],
                           param_1[0x1e],
                           (float)(local_20 % 5) * _DAT_005a4214 + (float)param_1[0x1f]);
      uVar3 = FUN_004310a0(uVar3);
      FUN_004310a0(uVar3);
      if (param_1[0x29] == 0) {
        (**(code **)(*local_18 + 0x3c))(0xaaaaaa00);
      }
      else if (param_1[0x29] == 1) {
        (**(code **)(*local_18 + 0x3c))(0xaa5599aa);
      }
      else if (param_1[0x29] == 2) {
        (**(code **)(*local_18 + 0x3c))(0xffffeeff);
      }
      local_18[0x52] = local_20 % 3 + 1;
      local_18[0x50] = 0x3fc00000;
      local_18[0x4f] = (int)((float)param_1[0x29] * _DAT_005a4154 + _DAT_005a4158);
      local_18[0x51] = 0x40400000;
      FUN_0054ac09(local_18);
    }
    param_1[0x28] = iVar1;
  }
  ExceptionList = local_10;
  return 1;
}

