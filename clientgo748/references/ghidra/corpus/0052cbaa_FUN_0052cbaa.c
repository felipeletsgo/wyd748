// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052cbaa | Name: FUN_0052cbaa


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0052cbaa(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int local_f8;
  int local_f4;
  int local_f0;
  undefined1 local_d0 [32];
  int *local_b0;
  int *local_ac;
  int local_a8;
  int local_a4;
  undefined4 local_a0;
  undefined1 local_9c [128];
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a23d9;
  local_10 = ExceptionList;
  local_18 = param_2;
  ExceptionList = &local_10;
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    *(undefined2 *)(param_1 + 0x450) = *(undefined2 *)(local_18 + 0xc);
    (**(code **)(**(int **)(param_1 + 0x434) + 0x88))((int)*(short *)(local_18 + 0xc));
    FUN_004ff37b((int)*(short *)(param_1 + 0x450));
    FUN_004ff365();
    local_a8 = 0;
    local_a0 = 0xffffff00;
    iVar1 = FUN_00430d79(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3660,
                         *(undefined4 *)(param_1 + 0x2c),&local_14,&local_1c);
    if (iVar1 != 0) {
      if (*(short *)(local_18 + 0xe) < 0) {
        FUN_0058f078(local_9c,&DAT_005bb840,-(int)*(short *)(local_18 + 0xe));
        if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
          local_a0 = 0xffff0000;
          pvVar2 = operator_new(0x2a0);
          local_8 = 0;
          if (pvVar2 == (void *)0x0) {
            local_f0 = 0;
          }
          else {
            uVar9 = 4;
            uVar8 = 0;
            uVar7 = 0x5dc;
            uVar6 = 1;
            uVar5 = 500;
            uVar4 = 0x40000000;
            uVar3 = local_a0;
            iVar1 = __ftol(local_a0,0x40000000,500,1,0x5dc,0,4);
            local_f0 = FUN_00422930(local_9c,local_14,local_1c + iVar1,uVar3,uVar4,uVar5,uVar6,uVar7
                                    ,uVar8,uVar9);
          }
          local_a8 = local_f0;
        }
        else {
          pvVar2 = operator_new(0x2a0);
          local_8 = 1;
          if (pvVar2 == (void *)0x0) {
            local_f4 = 0;
          }
          else {
            uVar9 = 3;
            uVar8 = 0;
            uVar7 = 0x5dc;
            uVar6 = 1;
            uVar5 = 500;
            uVar4 = 0x40000000;
            uVar3 = local_a0;
            iVar1 = __ftol(local_a0,0x40000000,500,1,0x5dc,0,3);
            local_f4 = FUN_00422930(local_9c,local_14,local_1c + iVar1,uVar3,uVar4,uVar5,uVar6,uVar7
                                    ,uVar8,uVar9);
          }
          local_a8 = local_f4;
        }
        local_8 = 0xffffffff;
      }
      else {
        local_a0 = 0xff5555ff;
        if (*(short *)(local_18 + 0xe) != 0) {
          FUN_0058f078(local_9c,&DAT_005bb844,(int)*(short *)(local_18 + 0xe));
          pvVar2 = operator_new(0x2a0);
          local_8 = 2;
          if (pvVar2 == (void *)0x0) {
            local_f8 = 0;
          }
          else {
            uVar9 = 2;
            uVar8 = 0;
            uVar7 = 0x5dc;
            uVar6 = 1;
            uVar5 = 500;
            uVar4 = 0x40000000;
            uVar3 = local_a0;
            iVar1 = __ftol(local_a0,0x40000000,500,1,0x5dc,0,2);
            local_f8 = FUN_00422930(local_9c,local_14,local_1c + iVar1,uVar3,uVar4,uVar5,uVar6,uVar7
                                    ,uVar8,uVar9);
          }
          local_8 = 0xffffffff;
          local_a8 = local_f8;
        }
      }
    }
    local_a4 = DAT_0067cf38;
    FUN_00430d79(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3660,
                 *(undefined4 *)(param_1 + 0x2c),&local_14,&local_1c);
    if (local_a8 != 0) {
      FUN_0054ac09(local_a8);
    }
    if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
      FUN_0058f220(DAT_013b71e8 + 0x72c,param_1 + 0x444,0x1c);
      local_ac = (int *)(**(code **)(**(int **)(local_a4 + 0x28) + 0x48))(0x491);
      (**(code **)(*local_ac + 0x88))((int)*(short *)(param_1 + 0x450));
      local_b0 = (int *)(**(code **)(**(int **)(local_a4 + 0x28) + 0x48))(0x402);
      if (local_b0 != (int *)0x0) {
        FUN_0058f078(local_d0,&DAT_005bb84c,(int)*(short *)(param_1 + 0x450));
        (**(code **)(*local_b0 + 0x80))(local_d0,0);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

