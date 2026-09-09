// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e7adc | Name: FUN_004e7adc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004e7adc(undefined4 *param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_7c;
  int local_78;
  int *local_74;
  int local_70;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_005a122e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a507c;
  local_8 = 0;
  if (param_1[0x27] != 0) {
    if ((undefined4 *)param_1[0x27] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x27])(1);
    }
    param_1[0x27] = 0;
  }
  if (param_1[0x28] != 0) {
    if ((undefined4 *)param_1[0x28] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x28])(1);
    }
    param_1[0x28] = 0;
  }
  if (DAT_0092e660 != 1) {
    if (param_1[0x29] == 0) {
      pvVar1 = operator_new(0x180);
      local_8._0_1_ = 1;
      if (pvVar1 == (void *)0x0) {
        local_70 = 0;
      }
      else {
        local_70 = FUN_004c8f70(0x47,0x378,0x3fc00000,0x3fc00000,0x3fc00000,0,8,0x6f);
      }
      local_8._0_1_ = 0;
      if (local_70 != 0) {
        *(undefined4 *)(local_70 + 0x6c) = 1;
        *(undefined4 *)(local_70 + 0x104) = 0;
        FUN_004310a0(param_1 + 0x24);
        *(float *)(local_70 + 0x78) = *(float *)(local_70 + 0x78) - _DAT_005a34a0;
        FUN_0054ac09(local_70);
      }
      pvVar1 = operator_new(0xc0);
      local_8._0_1_ = 2;
      if (pvVar1 == (void *)0x0) {
        local_74 = (int *)0x0;
      }
      else {
        local_74 = (int *)FUN_004df250(4,7,0x3f800000);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_74 != (int *)0x0) {
        (**(code **)(*local_74 + 0x44))(param_1[0x2f]);
        local_74[0x1b] = 1;
        puVar2 = (undefined4 *)FUN_00430f20(param_1[0x24],param_1[0x26]);
        (**(code **)(*local_74 + 0x3c))(*puVar2,puVar2[1]);
        local_74[0x28] = 0x5dc;
        FUN_0054ac09(local_74);
      }
    }
    else if (param_1[0x29] == 1) {
      pvVar1 = operator_new(0x9c);
      local_8._0_1_ = 3;
      if (pvVar1 == (void *)0x0) {
        local_78 = 0;
      }
      else {
        local_78 = FUN_004e2360(param_1[0x24],param_1[0x25],param_1[0x26],0,0x3f800000,0xb4,
                                0x44444444);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_78 != 0) {
        FUN_0054ac09(local_78);
      }
    }
    else if (param_1[0x29] == 2) {
      pvVar1 = operator_new(0x9c);
      local_8._0_1_ = 4;
      if (pvVar1 == (void *)0x0) {
        local_7c = 0;
      }
      else {
        local_7c = FUN_004e2360(param_1[0x24],param_1[0x25],param_1[0x26],2,0x3f800000,0xb4,0x4488);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_7c != 0) {
        FUN_0054ac09(local_7c);
      }
    }
    if ((((param_1[0x29] != 3) && (param_1[0x29] != 4)) && (DAT_005ccf98 != 0)) &&
       (iVar3 = FUN_00429a6d(0x9a), iVar3 != 0)) {
      uVar5 = 0;
      uVar4 = 0;
      FUN_00429a6d(0x9a);
      FUN_0042ad2b(uVar4,uVar5);
    }
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

