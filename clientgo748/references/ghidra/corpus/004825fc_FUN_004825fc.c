// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004825fc | Name: FUN_004825fc


undefined4 __thiscall FUN_004825fc(int param_1,int param_2)

{
  void *pvVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  int local_30;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fa36;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined1 *)(param_2 + 0x6f) = 0;
  *(undefined1 *)(param_2 + 0xcf) = 0;
  *(undefined1 *)(param_2 + 0x12f) = 0;
  *(undefined1 *)(param_2 + 399) = 0;
  if (*(short *)(param_2 + 0xc) == 10) {
    for (local_18 = 0; local_18 < 4; local_18 = local_18 + 1) {
      FUN_0058ee20(param_1 + 0x28814 + local_18 * 0x60,param_2 + 0x10 + local_18 * 0x60);
    }
    *(undefined4 *)(param_1 + 0x28994) = 0;
  }
  else if (*(int *)(param_1 + 0x285f4) != 0) {
    if (DAT_005b892c == 2) {
      if ((*(short *)(param_2 + 0xc) == 0) && (*(int *)(param_1 + 0x285f8) != 0)) {
        (**(code **)(**(int **)(param_1 + 0x285f8) + 0x80))(&DAT_00a3c378,0);
      }
      else if ((*(short *)(param_2 + 0xc) == 1) && (*(int *)(param_1 + 0x285f8) != 0)) {
        (**(code **)(**(int **)(param_1 + 0x285f8) + 0x80))(&DAT_00a3c2f8,0);
      }
    }
    for (local_1c = 0; local_1c < 4; local_1c = local_1c + 1) {
      pvVar1 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_30 = 0;
      }
      else {
        local_30 = FUN_00407203(param_2 + 0x10 + local_1c * 0x60,0xffccaaff,0,0,0x43960000,
                                0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      if (local_30 != 0) {
        FUN_00408d33(local_30);
      }
      (**(code **)(**(int **)(param_1 + 0x285fc + local_1c * 4) + 0x80))
                (param_2 + 0x10 + local_1c * 0x60,0);
    }
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x2860c) = uVar2;
    (**(code **)(**(int **)(param_1 + 0x285f4) + 0x60))(1);
    if (DAT_005ccf98 != 0) {
      iVar3 = FUN_00429a6d(0x21);
      if (iVar3 != 0) {
        uVar4 = 0;
        uVar2 = 0;
        FUN_00429a6d(0x21);
        FUN_0042ad2b(uVar2,uVar4);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

