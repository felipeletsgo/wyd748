// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047591f | Name: FUN_0047591f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0047591f(int param_1,int param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  void *pvVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  uint uVar8;
  float10 fVar9;
  int *local_b8;
  int *local_b4;
  int *local_a4;
  int *local_64;
  float local_44;
  undefined1 local_38 [12];
  undefined2 local_2c;
  undefined2 local_2a;
  short local_28;
  short local_26 [4];
  undefined1 local_1e;
  undefined1 local_1d;
  undefined1 local_1c;
  undefined1 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f7b7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = (int *)(**(code **)(*DAT_013b71e8 + 0x38))(param_2 + 0x3a99);
  if (local_14 == (int *)0x0) {
    _memset(local_38,0,0x20);
    local_28 = (short)param_2 + 0x3a99;
    local_26[0] = (&DAT_00e22c04)[param_2 * 4];
    local_2c = (&DAT_00e22c00)[param_2 * 4];
    local_2a = (&DAT_00e22c02)[param_2 * 4];
    local_1e = *(undefined1 *)(&DAT_00e22c06 + param_2 * 4);
    local_1c = 0x10;
    local_1d = 2;
    local_18 = local_38;
    iVar3 = FUN_0054cd07(local_26,0x22);
    if (iVar3 < 1) {
      if (local_14 == (int *)0x0) {
        if (*(short *)(&DAT_00d44990 + *(short *)(local_18 + 0x12) * 0x8c) == 0x647) {
          pvVar4 = operator_new(0x300);
          local_8 = 1;
          if (pvVar4 == (void *)0x0) {
            local_b4 = (int *)0x0;
          }
          else {
            local_b4 = (int *)FUN_004f0c50();
          }
          local_64 = local_b4;
          if (local_b4 != (int *)0x0) {
            local_b4[9] = 0x647;
          }
        }
        else {
          pvVar4 = operator_new(0x2e0);
          local_8 = 2;
          if (pvVar4 == (void *)0x0) {
            local_b8 = (int *)0x0;
          }
          else {
            local_b8 = (int *)FUN_004f3a9b();
          }
          local_64 = local_b8;
        }
        local_8 = 0xffffffff;
      }
      else {
        local_64 = local_14;
      }
      if (local_64 != (int *)0x0) {
        (**(code **)(*local_64 + 0x78))
                  (*(undefined4 *)(local_18 + 0x12),*(undefined4 *)(local_18 + 0x16));
        local_64[8] = (uint)*(ushort *)(local_18 + 0x10);
        local_64[0x10] = 0;
        (**(code **)(*local_64 + 0x38))();
        (**(code **)(*local_64 + 0x40))
                  (0,((float)(byte)local_18[0x1a] * _DAT_005a4148) / _DAT_005a365c,0);
        fVar1 = (float)*(ushort *)(local_18 + 0xc) + _DAT_005a34a0;
        fVar2 = (float)*(ushort *)(local_18 + 0xe) + _DAT_005a34a0;
        puVar5 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
        iVar3 = FUN_0049c70f(*puVar5,puVar5[1]);
        (**(code **)(*local_64 + 0x3c))(fVar1,(float)iVar3 * _DAT_005a4214 + _DAT_005a4214,fVar2);
        if (local_14 == (int *)0x0) {
          FUN_0054ac09(local_64);
        }
        if (local_18[0x1d] == '\x01') {
          iVar3 = FUN_0054cd07(local_18 + 0x12,0x26);
          if (iVar3 == 2) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x2c), iVar3 != 0)) {
              uVar7 = 0;
              uVar6 = 0;
              FUN_00429a6d(0x2c);
              FUN_0042ad2b(uVar6,uVar7);
            }
          }
          else if ((((*(short *)(local_18 + 0x12) == 0x19c) ||
                    (*(short *)(local_18 + 0x12) == 0x19d)) ||
                   (*(short *)(local_18 + 0x12) == 0x102d)) ||
                  ((*(short *)(local_18 + 0x12) == 0x1a3 || (*(short *)(local_18 + 0x12) == 0x1a4)))
                  ) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x30), iVar3 != 0)) {
              uVar7 = 0;
              uVar6 = 0;
              FUN_00429a6d(0x30);
              FUN_0042ad2b(uVar6,uVar7);
            }
          }
          else if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x2d), iVar3 != 0)) {
            uVar7 = 0;
            uVar6 = 0;
            FUN_00429a6d(0x2d);
            FUN_0042ad2b(uVar6,uVar7);
          }
        }
      }
    }
    else if ((-1 < local_26[0]) && (local_26[0] < 0x1965)) {
      pvVar4 = operator_new(0x314);
      local_8 = 0;
      if (pvVar4 == (void *)0x0) {
        local_a4 = (int *)0x0;
      }
      else {
        local_a4 = (int *)FUN_004f1960();
      }
      local_8 = 0xffffffff;
      if (local_a4 != (int *)0x0) {
        (**(code **)(*local_a4 + 0x78))
                  (*(undefined4 *)(local_18 + 0x12),*(undefined4 *)(local_18 + 0x16));
        (**(code **)(*local_a4 + 0x7c))
                  (*(undefined4 *)(local_18 + 0x12),*(undefined4 *)(local_18 + 0x16));
        local_a4[8] = (uint)*(ushort *)(local_18 + 0x10);
        local_a4[0x10] = 0;
        (**(code **)(*local_a4 + 0x38))();
        (**(code **)(*local_a4 + 0x40))
                  (0,((float)(byte)local_18[0x1a] * _DAT_005a4148) / _DAT_005a365c,0);
        fVar1 = (float)*(ushort *)(local_18 + 0xc) + _DAT_005a34a0;
        fVar2 = (float)*(ushort *)(local_18 + 0xe) + _DAT_005a34a0;
        puVar5 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
        fVar9 = (float10)FUN_005399cb(*puVar5,puVar5[1]);
        local_44 = (float)fVar9;
        if (local_44 < _DAT_005a4398) {
          uVar8 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
          if ((int)uVar8 < 0) {
            uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
          }
          if (*(int *)(param_1 + 0x68 + uVar8 * 4) == 0) {
            FUN_0049a73a(0);
            uVar8 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
            if ((int)uVar8 < 0) {
              uVar8 = (uVar8 - 1 | 0xfffffffe) + 1;
            }
            if (*(int *)(param_1 + 0x68 + uVar8 * 4) != 0) {
              puVar5 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
              fVar9 = (float10)FUN_005399cb(*puVar5,puVar5[1]);
              local_44 = (float)fVar9;
            }
          }
          else {
            puVar5 = (undefined4 *)FUN_00430f20(fVar1,fVar2);
            fVar9 = (float10)FUN_005399cb(*puVar5,puVar5[1]);
            local_44 = (float)fVar9;
          }
        }
        (**(code **)(*local_a4 + 0x3c))(fVar1,local_44,fVar2);
        uVar6 = FUN_0054cd07(local_18 + 0x12,0x22);
        uVar7 = __ftol((int)(char)local_18[0x1c]);
        FUN_005554cc(uVar6,1,local_18[0x1b],*(undefined2 *)(local_18 + 0xc),
                     *(undefined2 *)(local_18 + 0xe),param_1 + 0x94,uVar7);
        (**(code **)(*local_a4 + 0x80))(local_18[0x1b]);
        if (local_14 == (int *)0x0) {
          FUN_0054ac09(local_a4);
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}

