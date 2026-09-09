// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00527848 | Name: FUN_00527848


void __fastcall FUN_00527848(int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_3c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a222f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((*(int *)(DAT_0067cf38 + 0x26e1c) == 1) &&
     (ExceptionList = &local_10, param_1 == *(int *)(DAT_0067cf38 + 0x4c))) {
    ExceptionList = &local_10;
    *(undefined1 *)(param_1 + 0x79c) = 1;
    *(undefined4 *)(param_1 + 0x7a0) = 0x28;
    FUN_0051dba4(0x18,1);
    _memset((void *)(param_1 + 0x1a2),0,0x20);
    if (*(int *)(param_1 + 0x198) != 0) {
      FUN_004c0746(1,0x28);
      FUN_004bd906(3);
    }
  }
  if (*(int *)(param_1 + 0x19c) != 0) {
    if (*(int *)(param_1 + 0x19c) != 0) {
      FUN_00493d20(1);
    }
    *(undefined4 *)(param_1 + 0x19c) = 0;
  }
  iVar1 = DAT_0067cf38;
  if (*(char *)(param_1 + 0x79c) < '\x01') {
    if (((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) &&
       (*(int *)(DAT_0067cf38 + 0x4c) == param_1)) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x27918) + 0x60))(0);
      FUN_0044eca3();
      FUN_004fa8a1(*(undefined4 *)(iVar1 + 0x26e7c));
    }
  }
  else {
    if (*(int *)(param_1 + 0x19c) == 0) {
      pvVar2 = operator_new(0x358);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_3c = 0;
      }
      else {
        local_3c = FUN_004bd420(param_1 + 0x1a2,param_1 + 0x1c2,*(undefined4 *)(param_1 + 0x7a0),0,0
                                ,1,0);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x19c) = local_3c;
      if (*(int *)(param_1 + 0x19c) == 0) {
        ExceptionList = local_10;
        return;
      }
      *(int *)(*(int *)(param_1 + 0x19c) + 0x2f4) = param_1;
      *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x2e4) = 0x28;
      if (*(int *)(param_1 + 0x24c) == 0x28) {
        *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x24) = *(undefined4 *)(param_1 + 0x7a4);
        *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x28) = *(undefined4 *)(param_1 + 0x7a4);
        *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x2c) = *(undefined4 *)(param_1 + 0x7a4);
      }
      else {
        *(float *)(*(int *)(param_1 + 0x19c) + 0x24) =
             *(float *)(param_1 + 0x38) * *(float *)(param_1 + 0x7a4);
        *(float *)(*(int *)(param_1 + 0x19c) + 0x28) =
             *(float *)(param_1 + 0x38) * *(float *)(param_1 + 0x7a4);
        *(float *)(*(int *)(param_1 + 0x19c) + 0x2c) =
             *(float *)(param_1 + 0x38) * *(float *)(param_1 + 0x7a4);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x30) = 0;
      if ((*(int *)(param_1 + 0x7a0) == 0x14) && (*(short *)(param_1 + 0x1a2) == 7)) {
        FUN_004c0746(4,*(undefined4 *)(param_1 + 0x7a0));
      }
      else if (*(int *)(param_1 + 0x7a0) == 0x14) {
        FUN_004c0746(3,*(undefined4 *)(param_1 + 0x7a0));
      }
      else {
        FUN_004c0746(2,*(undefined4 *)(param_1 + 0x7a0));
      }
    }
    if (*(int *)(param_1 + 0x19c) != 0) {
      FUN_004be7b1();
    }
  }
  ExceptionList = local_10;
  return;
}

