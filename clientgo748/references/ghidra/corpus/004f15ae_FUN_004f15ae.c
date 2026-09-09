// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f15ae | Name: FUN_004f15ae


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004f15ae(int *param_1)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
  undefined4 *puVar4;
  uint uVar5;
  uint uVar6;
  float10 fVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int local_74;
  int local_70;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  float local_2c;
  undefined4 local_28;
  float local_24;
  int local_1c;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1666;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  (**(code **)(*param_1 + 0x4c))();
  local_14 = 10000;
  if (param_1[0x58] == 2) {
    local_14 = 0x9c4;
  }
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  if (local_14 < (uint)(iVar1 - param_1[0x59])) {
    iVar2 = _rand();
    uVar5 = iVar2 % 5;
    uVar6 = uVar5 & 0x80000001;
    if ((int)uVar6 < 0) {
      uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
    }
    if (uVar6 == 1) {
      if (param_1[0x58] < 2) {
        if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(2), iVar2 != 0)) {
          FUN_00429a6d(2);
          iVar2 = FUN_0042afd0();
          if (iVar2 == 0) {
            uVar9 = 0;
            uVar8 = 0;
            FUN_00429a6d(2);
            FUN_0042ad2b(uVar8,uVar9);
          }
        }
        for (local_1c = 0; local_1c < (int)uVar5; local_1c = local_1c + 1) {
          FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
          local_2c = (float)(int)(local_1c + -3 + uVar5) * _DAT_005a4160 + local_2c;
          local_24 = (float)(int)(local_1c + -3 + uVar5) * _DAT_005a34a0 + local_24;
          pvVar3 = operator_new(0xd0);
          local_8 = 0;
          if (pvVar3 == (void *)0x0) {
            local_70 = 0;
          }
          else {
            local_70 = FUN_004cf150(local_2c,local_28,local_24,param_1[0x5b],param_1[0x58]);
          }
          local_8 = 0xffffffff;
          if (local_70 != 0) {
            FUN_0054ac09(local_70);
          }
        }
      }
      else if (param_1[0x58] == 2) {
        FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
        pvVar3 = operator_new(0x90);
        local_8 = 1;
        if (pvVar3 == (void *)0x0) {
          local_74 = 0;
        }
        else {
          uVar8 = 1000;
          puVar4 = (undefined4 *)FUN_00430f50(0,0,0);
          local_74 = FUN_004d4de0(local_3c,local_38,local_34,0,6,0x3a03126f,0xaaaaaaaa,0,0x78,
                                  0x3f800000,1,*puVar4,puVar4[1],puVar4[2],uVar8);
        }
        local_8 = 0xffffffff;
        if (local_74 != 0) {
          FUN_0054ac09(local_74);
        }
        if (*(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54) == 0) {
          param_1[0x59] = iVar1;
          ExceptionList = local_10;
          return 1;
        }
        fVar7 = (float10)FUN_004d0150(param_1 + 10);
        if ((((float)fVar7 < _DAT_005a3834) && (DAT_005ccf98 != 0)) &&
           (iVar2 = FUN_00429a6d(7), iVar2 != 0)) {
          uVar9 = 0;
          uVar8 = 0;
          FUN_00429a6d(7);
          FUN_0042ad2b(uVar8,uVar9);
        }
      }
    }
    param_1[0x59] = iVar1;
  }
  ExceptionList = local_10;
  return 1;
}

