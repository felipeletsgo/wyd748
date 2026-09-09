// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c37c4 | Name: FUN_004c37c4


undefined4 __fastcall FUN_004c37c4(int param_1)

{
  void *pvVar1;
  int local_34;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a06e4;
  local_10 = ExceptionList;
  if (*(int *)(*(int *)(param_1 + 0xc0) + 0x2f4) != 0) {
    ExceptionList = &local_10;
    for (local_18 = 0; local_18 < 2; local_18 = local_18 + 1) {
      if (((*(int *)(param_1 + 0xc0) != 0) && (**(int **)(param_1 + 0xc0) < 0x13)) &&
         (*(int *)(param_1 + 8) ==
          *(int *)(&DAT_005baa88 + **(int **)(param_1 + 0xc0) * 8 + local_18 * 4))) {
        pvVar1 = operator_new(0x291c);
        local_8 = 0;
        if (pvVar1 == (void *)0x0) {
          local_34 = 0;
        }
        else {
          local_34 = FUN_004d97d0();
        }
        local_8 = 0xffffffff;
        if (local_34 != 0) {
          *(undefined4 *)(local_34 + 0x84) = *(undefined4 *)(param_1 + 0xc0);
          if (*(int *)(*(int *)(param_1 + 0xc0) + 0x2f8 + local_18 * 4) != 0) {
            (**(code **)(*DAT_013b71e8 + 100))
                      (*(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2f8 + local_18 * 4));
            *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2f8 + local_18 * 4) = 0;
          }
          *(int *)(*(int *)(param_1 + 0xc0) + 0x2f8 + local_18 * 4) = local_34;
          local_14 = FUN_004c15fc(*(undefined4 *)(param_1 + 8));
          *(undefined4 *)(local_34 + 0x1c4c) = 0;
          local_1c = 0;
          while ((local_1c < 0x30 && (local_14 != 0))) {
            *(undefined4 *)(local_34 + 0x1b8c + local_1c * 4) = *(undefined4 *)(local_14 + 4);
            local_14 = FUN_004c15fc(*(undefined4 *)(local_14 + 4));
            *(int *)(local_34 + 0x1c4c) = *(int *)(local_34 + 0x1c4c) + 1;
            if (*(int *)(local_34 + 0x1b8c + local_1c * 4) == 0) break;
            local_1c = local_1c + 1;
          }
          if ((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x2c) != 0)) {
            FUN_0054ac09(local_34);
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

