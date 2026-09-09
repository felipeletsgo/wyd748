// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00410a91 | Name: FUN_00410a91


void __thiscall FUN_00410a91(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_16c;
  undefined1 local_158 [128];
  int local_d8;
  int local_d4;
  undefined4 local_d0;
  undefined1 local_cc [128];
  int local_4c;
  int local_48;
  int *local_44;
  undefined1 local_40 [4];
  undefined2 local_3c;
  undefined2 local_3a;
  undefined2 local_34;
  short local_32;
  short local_30;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  short local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ee93;
  local_10 = ExceptionList;
  if (param_1[0x100] == 1) {
    ExceptionList = &local_10;
    local_14 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
    if (local_14 != 0) {
      local_18 = (short)*(undefined4 *)(local_14 + 0x1d0) +
                 (short)*(undefined4 *)(local_14 + 0x1d4) * 9;
      local_20 = FUN_0054cd07(*(undefined4 *)(local_14 + 0x670),0x21);
      if ((7 < local_20) || (local_20 < 0)) {
        local_20 = 0;
      }
      local_1c = DAT_0067cf38;
      local_44 = *(int **)(DAT_0067cf38 + 0x2873c);
      (**(code **)(*local_44 + 0x98))
                (&local_28,*(undefined4 *)(&DAT_005bec0c + local_20 * 8),
                 *(undefined4 *)(&DAT_005bec10 + local_20 * 8));
      _memset(local_40,0,0x18);
      local_3a = *(undefined2 *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x20);
      local_3c = 0x379;
      local_34 = (undefined2)param_1[0x101];
      local_32 = local_18;
      local_30 = (short)local_24 * 9 + (short)local_28;
      if ((local_28 < 0) || (local_24 < 0)) {
        local_d0 = *(undefined4 *)(local_1c + 0x27ae8);
        pvVar2 = operator_new(0xe50);
        local_8 = 0;
        if (pvVar2 == (void *)0x0) {
          local_16c = 0;
        }
        else {
          local_16c = FUN_00407203(&DAT_00a341f8,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1
                                   ,0);
        }
        local_8 = 0xffffffff;
        local_d4 = local_16c;
        if (local_16c != 0) {
          FUN_00408d33(local_16c);
        }
        if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x21), iVar1 != 0)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x21);
          FUN_0042ad2b(uVar3,uVar4);
        }
      }
      else if ((**(short **)(local_14 + 0x670) < 0xc80) || (0xce3 < **(short **)(local_14 + 0x670)))
      {
        if ((**(short **)(local_14 + 0x670) == 0xd03) || (**(short **)(local_14 + 0x670) != 0x1033))
        {
          FUN_0055f2dd(local_40,0x18);
        }
        else {
          _memset((void *)(local_1c + 0x28b6c),0,0x24);
          FUN_0058f220(local_1c + 0x28b6c,local_40,0x18);
          *(undefined2 *)(local_1c + 0x28b70) = 0x3ce;
          if (*(int *)(local_1c + 0x28b44) != 0) {
            iVar1 = FUN_0040c0f0();
            if (iVar1 != 0) {
              FUN_004481c5(0);
            }
            iVar1 = FUN_0040c0f0();
            if (iVar1 != 0) {
              (**(code **)(**(int **)(local_1c + 0x58) + 0x60))(0);
            }
            (**(code **)(**(int **)(local_1c + 0x28b44) + 0x60))(1);
            (**(code **)(**(int **)(local_1c + 0x28b68) + 0x90))(&DAT_005ccf1c);
            (**(code **)(**(int **)(local_1c + 0x28b60) + 0x90))(&DAT_005ccf20);
            (**(code **)(**(int **)(local_1c + 0x28b64) + 0x90))(&DAT_005ccf24);
            (**(code **)(**(int **)(local_1c + 0x28) + 0x40))(*(undefined4 *)(local_1c + 0x28b68));
          }
        }
      }
      else {
        iVar1 = FUN_0040c0f0();
        if (iVar1 != 0) {
          (**(code **)(**(int **)(local_1c + 0x58) + 0x60))(0);
        }
        _memset((void *)(local_1c + 0x26e54),0,0x20);
        FUN_0058f220(local_1c + 0x26e54,local_40,0x18);
        *(undefined2 *)(local_1c + 0x26e58) = 0x3b8;
        if ((0 < **(short **)(local_14 + 0x670)) && (**(short **)(local_14 + 0x670) < 0x1964)) {
          local_48 = *(int *)(&DAT_00d449d0 + **(short **)(local_14 + 0x670) * 0x8c);
        }
        if (DAT_005b892c == 2) {
          FUN_0058f078(local_cc,&DAT_00a3c578,
                       local_48 + (local_48 * *(int *)(DAT_013b71e8 + 0xf0c)) / 100);
        }
        else {
          FUN_0058f078(local_cc,&DAT_005b12c8,
                       local_48 + (local_48 * *(int *)(DAT_013b71e8 + 0xf0c)) / 100);
        }
        for (local_4c = 0; local_4c < 6; local_4c = local_4c + 1) {
          (**(code **)(**(int **)(local_1c + 0x278b8 + local_4c * 4) + 0x80))(&DAT_005ccf18,0);
        }
        (**(code **)(**(int **)(local_1c + 0x278d0) + 0x80))(local_cc,0);
        (**(code **)(**(int **)(local_1c + 0x278b4) + 0x60))(1);
        (**(code **)(**(int **)(local_1c + 0x285d4) + 0x60))(0);
      }
    }
  }
  else if ((param_1[0x100] == 4) &&
          (ExceptionList = &local_10, local_d8 = (**(code **)(*param_1 + 0xb4))(param_2,param_3),
          local_d8 != 0)) {
    FUN_0058f078(local_158,&DAT_00a358f8,&DAT_00d44950 + **(short **)(local_d8 + 0x670) * 0x8c);
    iVar1 = DAT_0067cf38;
    (**(code **)(**(int **)(DAT_0067cf38 + 0x8c) + 0x8c))(local_158,4,0);
    (**(code **)(**(int **)(iVar1 + 0x8c) + 0x60))(1);
    *(int *)(*(int *)(iVar1 + 0x8c) + 0x1e8) =
         (int)**(short **)(local_d8 + 0x670) << 0x10 | param_1[0x101];
  }
  ExceptionList = local_10;
  return;
}

