// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052e85d | Name: FUN_0052e85d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0052e85d(int param_1,int param_2)

{
  void *pvVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  int *local_58;
  int local_54;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2480;
  local_10 = ExceptionList;
  if (*(int *)(param_2 + 0xc) == 1) {
    ExceptionList = &local_10;
    for (local_1c = -1; local_1c < 2; local_1c = local_1c + 1) {
      pvVar1 = operator_new(0xcc);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_54 = 0;
      }
      else {
        local_54 = FUN_004d2230(0x1fa,0xffffffff,0,0);
      }
      local_8 = 0xffffffff;
      if (local_54 != 0) {
        *(undefined4 *)(local_54 + 0x6c) = 1;
        *(undefined1 *)(local_54 + 0xac) = 1;
        *(undefined4 *)(local_54 + 0xb8) = 3000;
        *(undefined4 *)(local_54 + 0xb0) = 8000;
        *(undefined4 *)(local_54 + 0x98) = 0x3f800000;
        *(undefined4 *)(local_54 + 0x9c) = 0x40200000;
        uVar2 = FUN_00430f50((float)local_1c * _DAT_005a4158 + *(float *)(param_1 + 0x28),
                             *(undefined4 *)(param_1 + 0x30),
                             (float)local_1c * _DAT_005a4158 + *(float *)(param_1 + 0x2c));
        FUN_004310a0(uVar2);
        FUN_0054ac09(local_54);
      }
    }
    pvVar1 = operator_new(0xc0);
    local_8 = 1;
    if (pvVar1 == (void *)0x0) {
      local_58 = (int *)0x0;
    }
    else {
      local_58 = (int *)FUN_004df250(3,0x76,0x3f800000);
    }
    local_8 = 0xffffffff;
    if (local_58 != (int *)0x0) {
      local_58[0x28] = 8000;
      (**(code **)(*local_58 + 0x44))(0xffffffff);
      local_58[0x1b] = 1;
      puVar3 = (undefined4 *)
               FUN_00430f20(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
      (**(code **)(*local_58 + 0x3c))(*puVar3,puVar3[1]);
      FUN_0054ac09(local_58);
    }
    if (DAT_005ccf98 != 0) {
      iVar4 = FUN_00429a6d(1);
      if (iVar4 != 0) {
        uVar5 = 0;
        uVar2 = 0;
        FUN_00429a6d(1);
        FUN_0042ad2b(uVar2,uVar5);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

