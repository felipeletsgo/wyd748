// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004856c3 | Name: FUN_004856c3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004856c3(int param_1,int param_2)

{
  float fVar1;
  float fVar2;
  int *piVar3;
  int iVar4;
  void *pvVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  uint uVar9;
  float10 fVar10;
  int *local_94;
  int *local_90;
  int *local_80;
  int *local_40;
  int *local_28;
  float local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fadb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  piVar3 = (int *)(**(code **)(*DAT_013b71e8 + 0x38))(*(undefined2 *)(param_2 + 0x10));
  iVar4 = FUN_0054cd07(param_2 + 0x12,0x22);
  if (iVar4 < 1) {
    local_40 = piVar3;
    if (piVar3 == (int *)0x0) {
      if (*(short *)(&DAT_00d44990 + *(short *)(param_2 + 0x12) * 0x8c) == 0x647) {
        pvVar5 = operator_new(0x300);
        local_8 = 1;
        if (pvVar5 == (void *)0x0) {
          local_90 = (int *)0x0;
        }
        else {
          local_90 = (int *)FUN_004f0c50();
        }
        local_40 = local_90;
        if (local_90 != (int *)0x0) {
          local_90[9] = 0x647;
        }
        *(undefined1 *)(param_2 + 0x1a) = 1;
      }
      else {
        pvVar5 = operator_new(0x2e0);
        local_8 = 2;
        if (pvVar5 == (void *)0x0) {
          local_94 = (int *)0x0;
        }
        else {
          local_94 = (int *)FUN_004f3a9b();
        }
        local_40 = local_94;
      }
      local_8 = 0xffffffff;
    }
    if (local_40 != (int *)0x0) {
      (**(code **)(*local_40 + 0x78))
                (*(undefined4 *)(param_2 + 0x12),*(undefined4 *)(param_2 + 0x16));
      local_40[8] = (uint)*(ushort *)(param_2 + 0x10);
      local_40[0x10] = 0;
      (**(code **)(*local_40 + 0x38))();
      (**(code **)(*local_40 + 0x40))
                (0,((float)*(byte *)(param_2 + 0x1a) * _DAT_005a4148) / _DAT_005a365c,0);
      fVar1 = (float)*(ushort *)(param_2 + 0xc) + _DAT_005a34a0;
      fVar2 = (float)*(ushort *)(param_2 + 0xe) + _DAT_005a34a0;
      puVar6 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
      iVar4 = FUN_0049c70f(*puVar6,puVar6[1]);
      (**(code **)(*local_40 + 0x3c))(fVar1,(float)iVar4 * _DAT_005a4214 + _DAT_005a4214,fVar2);
      if (piVar3 == (int *)0x0) {
        FUN_0054ac09(local_40);
      }
      if (*(char *)(param_2 + 0x1d) == '\x01') {
        iVar4 = FUN_0054cd07(param_2 + 0x12,0x26);
        if (iVar4 == 2) {
          if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x2c), iVar4 != 0)) {
            uVar8 = 0;
            uVar7 = 0;
            FUN_00429a6d(0x2c);
            FUN_0042ad2b(uVar7,uVar8);
          }
        }
        else if ((((*(short *)(param_2 + 0x12) == 0x19c) || (*(short *)(param_2 + 0x12) == 0x19d))
                 || (*(short *)(param_2 + 0x12) == 0x102d)) ||
                ((*(short *)(param_2 + 0x12) == 0x1a3 || (*(short *)(param_2 + 0x12) == 0x1a4)))) {
          if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x30), iVar4 != 0)) {
            uVar8 = 0;
            uVar7 = 0;
            FUN_00429a6d(0x30);
            FUN_0042ad2b(uVar7,uVar8);
          }
        }
        else if (*(short *)(param_2 + 0x12) == 0x2eb) {
          if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x132), iVar4 != 0)) {
            uVar8 = 0;
            uVar7 = 0;
            FUN_00429a6d(0x132);
            FUN_0042ad2b(uVar7,uVar8);
          }
        }
        else if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x2d), iVar4 != 0)) {
          uVar8 = 0;
          uVar7 = 0;
          FUN_00429a6d(0x2d);
          FUN_0042ad2b(uVar7,uVar8);
        }
      }
    }
  }
  else {
    local_28 = piVar3;
    if (piVar3 == (int *)0x0) {
      pvVar5 = operator_new(0x314);
      local_8 = 0;
      if (pvVar5 == (void *)0x0) {
        local_80 = (int *)0x0;
      }
      else {
        local_80 = (int *)FUN_004f1960();
      }
      local_8 = 0xffffffff;
      local_28 = local_80;
    }
    if (local_28 != (int *)0x0) {
      (**(code **)(*local_28 + 0x78))
                (*(undefined4 *)(param_2 + 0x12),*(undefined4 *)(param_2 + 0x16));
      (**(code **)(*local_28 + 0x7c))
                (*(undefined4 *)(param_2 + 0x12),*(undefined4 *)(param_2 + 0x16));
      local_28[8] = (uint)*(ushort *)(param_2 + 0x10);
      local_28[0x10] = 0;
      (**(code **)(*local_28 + 0x38))();
      (**(code **)(*local_28 + 0x40))
                (0,((float)*(byte *)(param_2 + 0x1a) * _DAT_005a4148) / _DAT_005a365c,0);
      fVar1 = (float)*(ushort *)(param_2 + 0xc) + _DAT_005a34a0;
      fVar2 = (float)*(ushort *)(param_2 + 0xe) + _DAT_005a34a0;
      puVar6 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
      fVar10 = (float10)FUN_005399cb(*puVar6,puVar6[1]);
      local_1c = (float)fVar10;
      if (local_1c < _DAT_005a4398) {
        uVar9 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
        if ((int)uVar9 < 0) {
          uVar9 = (uVar9 - 1 | 0xfffffffe) + 1;
        }
        if (*(int *)(param_1 + 0x68 + uVar9 * 4) == 0) {
          FUN_0049a73a(0);
          uVar9 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
          if ((int)uVar9 < 0) {
            uVar9 = (uVar9 - 1 | 0xfffffffe) + 1;
          }
          if (*(int *)(param_1 + 0x68 + uVar9 * 4) != 0) {
            puVar6 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
            fVar10 = (float10)FUN_005399cb(*puVar6,puVar6[1]);
            local_1c = (float)fVar10;
          }
        }
        else {
          puVar6 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
          fVar10 = (float10)FUN_005399cb(*puVar6,puVar6[1]);
          local_1c = (float)fVar10;
        }
      }
      (**(code **)(*local_28 + 0x3c))(fVar1,local_1c,fVar2);
      *(undefined2 *)(local_28 + 0xc0) = 1;
      uVar7 = FUN_0054cd07(param_2 + 0x12,0x22);
      uVar8 = __ftol((int)*(char *)(param_2 + 0x1c));
      FUN_005554cc(uVar7,1,*(undefined1 *)(param_2 + 0x1b),*(undefined2 *)(param_2 + 0xc),
                   *(undefined2 *)(param_2 + 0xe),param_1 + 0x94,uVar8);
      (**(code **)(*local_28 + 0x80))(*(undefined1 *)(param_2 + 0x1b));
      if (piVar3 == (int *)0x0) {
        FUN_0054ac09(local_28);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

