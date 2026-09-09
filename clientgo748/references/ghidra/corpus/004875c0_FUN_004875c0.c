// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004875c0 | Name: FUN_004875c0


undefined4 __thiscall FUN_004875c0(int param_1,int param_2)

{
  int *piVar1;
  void *pvVar2;
  void *pvVar3;
  int iVar4;
  uint uVar5;
  int local_70;
  int local_68;
  int local_30;
  int local_28;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fb28;
  local_10 = ExceptionList;
  if (*(int *)(param_2 + 0xc) == 1) {
    piVar1 = *(int **)(param_1 + 0x28730);
    ExceptionList = &local_10;
    (**(code **)(*piVar1 + 0xbc))();
    for (local_18 = 0; local_18 < 0x1b; local_18 = local_18 + 1) {
      pvVar2 = operator_new(8);
      if ((pvVar2 != (void *)0x0) &&
         (FUN_0058f220(pvVar2,param_2 + 0x10 + local_18 * 8,8),
         0 < *(short *)(param_2 + 0x10 + local_18 * 8))) {
        pvVar3 = operator_new(0x678);
        local_8 = 0;
        if (pvVar3 == (void *)0x0) {
          local_68 = 0;
        }
        else {
          local_68 = FUN_0040d13e(0,pvVar2,0,0);
        }
        local_8 = 0xffffffff;
        if (local_68 != 0) {
          (**(code **)(*piVar1 + 0x8c))(local_68,local_18 % 9,(local_18 / 9) * 3);
          local_28 = FUN_0054e93b(pvVar2);
          if ((0x919 < **(short **)(local_68 + 0x670)) && (**(short **)(local_68 + 0x670) < 0x956))
          {
            local_28 = 0;
          }
          if (0 < local_28) {
            FUN_0058f078(local_68 + 0x39a,&PTR_DAT_005b88a4,local_28);
            FUN_00421b8f(local_68 + 0x39a,*(undefined4 *)(local_68 + 900),0);
          }
        }
      }
    }
    *(undefined4 *)(DAT_013b71e8 + 0xf0c) = *(undefined4 *)(param_2 + 0xe8);
    FUN_004481c5(1);
  }
  else if (*(int *)(param_2 + 0xc) == 3) {
    piVar1 = *(int **)(param_1 + 0x28734);
    ExceptionList = &local_10;
    (**(code **)(*piVar1 + 0xbc))();
    if (DAT_005b892c == 2) {
      switch((*(short *)(param_2 + 0x10) + -5000) / 0x18) {
      case 0:
        (**(code **)(**(int **)(param_1 + 0x285e4) + 0x80))(&DAT_00a376f8,0);
        (**(code **)(**(int **)(param_1 + 0x285e8) + 0x80))(&DAT_00a37778,0);
        (**(code **)(**(int **)(param_1 + 0x285ec) + 0x80))(&DAT_00a377f8,0);
        break;
      case 1:
        (**(code **)(**(int **)(param_1 + 0x285e4) + 0x80))(&DAT_00a37878,0);
        (**(code **)(**(int **)(param_1 + 0x285e8) + 0x80))(&DAT_00a378f8,0);
        (**(code **)(**(int **)(param_1 + 0x285ec) + 0x80))(&DAT_00a37978,0);
        break;
      case 2:
        (**(code **)(**(int **)(param_1 + 0x285e4) + 0x80))(&DAT_00a379f8,0);
        (**(code **)(**(int **)(param_1 + 0x285e8) + 0x80))(&DAT_00a37a78,0);
        (**(code **)(**(int **)(param_1 + 0x285ec) + 0x80))(&DAT_00a37af8,0);
        break;
      case 3:
        (**(code **)(**(int **)(param_1 + 0x285e4) + 0x80))(&DAT_00a383f8,0);
        (**(code **)(**(int **)(param_1 + 0x285e8) + 0x80))(&DAT_00a38478,0);
        (**(code **)(**(int **)(param_1 + 0x285ec) + 0x80))(&DAT_00a384f8,0);
        break;
      default:
        (**(code **)(**(int **)(param_1 + 0x285e4) + 0x80))(&DAT_00a376f8,0);
        (**(code **)(**(int **)(param_1 + 0x285e8) + 0x80))(&DAT_00a37778,0);
        (**(code **)(**(int **)(param_1 + 0x285ec) + 0x80))(&DAT_00a377f8,0);
      }
    }
    for (local_30 = 0; local_30 < 0x1b; local_30 = local_30 + 1) {
      pvVar2 = operator_new(8);
      if ((pvVar2 != (void *)0x0) &&
         (FUN_0058f220(pvVar2,param_2 + 0x10 + local_30 * 8,8),
         0 < *(short *)(param_2 + 0x10 + local_30 * 8))) {
        pvVar3 = operator_new(0x678);
        local_8 = 1;
        if (pvVar3 == (void *)0x0) {
          local_70 = 0;
        }
        else {
          local_70 = FUN_0040d13e(0,pvVar2,0,0);
        }
        local_8 = 0xffffffff;
        if (DAT_005b892c == 2) {
          if (local_70 != 0) {
            iVar4 = local_30 - local_30 / 9;
            uVar5 = local_30 % 9 & 0x80000003;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xfffffffc) + 1;
            }
            (**(code **)(*piVar1 + 0x8c))
                      (local_70,uVar5,((int)(iVar4 + (iVar4 >> 0x1f & 3U)) >> 2) + local_30 / 9);
          }
        }
        else if (local_70 != 0) {
          (**(code **)(*piVar1 + 0x8c))(local_70,local_30 % 9,(local_30 / 9) * 3);
        }
      }
    }
    iVar4 = FUN_0040c0f0();
    if (iVar4 == 0) {
      FUN_0044c15c();
    }
  }
  ExceptionList = local_10;
  return 1;
}

