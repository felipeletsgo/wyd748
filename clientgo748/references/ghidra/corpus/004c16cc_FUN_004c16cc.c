// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c16cc | Name: FUN_004c16cc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c16cc(int *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  undefined4 *puVar5;
  undefined1 local_264 [64];
  undefined1 local_224 [64];
  undefined4 local_1e4;
  undefined4 local_1e0;
  undefined4 local_1dc;
  undefined4 local_1d8;
  undefined1 local_1d4 [64];
  undefined4 local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  int local_150;
  undefined1 local_14c [64];
  undefined1 local_10c [64];
  int local_cc;
  float local_c8;
  undefined1 local_c4 [64];
  undefined1 local_84 [64];
  int local_44 [16];
  
  if (param_1[0x24] != 0) {
    piVar3 = param_1 + 0x12;
    piVar4 = local_44;
    for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *piVar4 = *piVar3;
      piVar3 = piVar3 + 1;
      piVar4 = piVar4 + 1;
    }
    if ((*(int *)param_1[0x25] < 0x13) &&
       (*param_1 == *(int *)(&DAT_005baa88 + *(int *)param_1[0x25] * 8))) {
      FUN_00430f10();
      FUN_00430f10();
      if (*(char *)(param_1[0x25] + 0x304) == '\x01') {
        thunk_FUN_0056239d(local_84,-*(float *)(param_1[0x25] + 0x300),0x3c23d70a,0);
        thunk_FUN_00562821(local_c4,0,0,0x40490fdb);
      }
      else {
        local_c8 = 0.0;
        if ((0x6a4 < *(ushort *)(param_1[0x25] + 0x264)) &&
           (*(ushort *)(param_1[0x25] + 0x264) < 0x709)) {
          local_c8 = -0.05;
        }
        thunk_FUN_0056239d(local_84,-*(float *)(param_1[0x25] + 0x300),_DAT_005a45d4 + local_c8,0);
        thunk_FUN_00562821(local_c4,0x40490fdb,0,0);
      }
      thunk_FUN_00561a5b(local_c4,local_84,local_c4);
      thunk_FUN_00561a5b(local_44,local_c4,param_1 + 0x12);
      if (*(int *)(param_1[0x25] + 0x2f8) != 0) {
        FUN_004c5b60(local_44);
        piVar3 = param_1 + 0x12;
        piVar4 = (int *)(*(int *)(param_1[0x25] + 0x2f8) + 0x2c);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *piVar4 = *piVar3;
          piVar3 = piVar3 + 1;
          piVar4 = piVar4 + 1;
        }
        puVar1 = (undefined4 *)FUN_004c5b90(local_224,local_c4);
        puVar5 = (undefined4 *)(*(int *)(param_1[0x25] + 0x2f8) + 0x2890);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar5 = *puVar1;
          puVar1 = puVar1 + 1;
          puVar5 = puVar5 + 1;
        }
        puVar1 = (undefined4 *)(*(int *)(param_1[0x25] + 0xb4) + 8);
        puVar5 = (undefined4 *)(*(int *)(param_1[0x25] + 0x2f8) + 0x2850);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar5 = *puVar1;
          puVar1 = puVar1 + 1;
          puVar5 = puVar5 + 1;
        }
        *(int *)(*(int *)(param_1[0x25] + 0x2f8) + 0x28fc) = *param_1;
        local_cc = *(int *)(param_1[0x25] + 0x2f4);
        if (local_cc != 0) {
          *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f8) + 0x28d4) =
               *(undefined4 *)(local_cc + 0x164);
        }
      }
    }
    if ((*(int *)param_1[0x25] < 0x13) &&
       (*param_1 == *(int *)(&DAT_005baa8c + *(int *)param_1[0x25] * 8))) {
      FUN_00430f10();
      FUN_00430f10();
      if (*(char *)(param_1[0x25] + 0x305) == '\x01') {
        thunk_FUN_0056239d(local_10c,-*(float *)(param_1[0x25] + 0x300),0x3c23d70a,0);
        thunk_FUN_00562821(local_14c,0x40490fdb,0,0);
      }
      else {
        if (*(int *)param_1[0x25] == 6) {
          thunk_FUN_0056239d(local_10c,-*(float *)(param_1[0x25] + 0x300) * _DAT_005a34a0,0xbc23d70a
                             ,0);
        }
        else if (*(int *)param_1[0x25] == 9) {
          thunk_FUN_0056239d(local_10c,-*(float *)(param_1[0x25] + 0x300) * _DAT_005a46d8,0x3cf5c28f
                             ,0);
        }
        else {
          thunk_FUN_0056239d(local_10c,-*(float *)(param_1[0x25] + 0x300),0xbc23d70a,0);
        }
        if (*(int *)param_1[0x25] == 1) {
          thunk_FUN_00562821(local_14c,0xbe860a92,0xbe32b8c3,0x40490fdb);
        }
        else if (*(int *)param_1[0x25] == 6) {
          thunk_FUN_00562821(local_14c,0xbeb2b8c3,0,0x40490fdb);
        }
        else {
          thunk_FUN_00562821(local_14c,0,0,0x40490fdb);
        }
      }
      thunk_FUN_00561a5b(local_14c,local_10c,local_14c);
      thunk_FUN_00561a5b(local_44,local_14c,param_1 + 0x12);
      if (*(int *)(param_1[0x25] + 0x2fc) != 0) {
        FUN_004c5b60(local_44);
        piVar3 = param_1 + 0x12;
        piVar4 = (int *)(*(int *)(param_1[0x25] + 0x2fc) + 0x2c);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *piVar4 = *piVar3;
          piVar3 = piVar3 + 1;
          piVar4 = piVar4 + 1;
        }
        puVar1 = (undefined4 *)FUN_004c5b90(local_264,local_14c);
        puVar5 = (undefined4 *)(*(int *)(param_1[0x25] + 0x2fc) + 0x2890);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar5 = *puVar1;
          puVar1 = puVar1 + 1;
          puVar5 = puVar5 + 1;
        }
        puVar1 = (undefined4 *)(*(int *)(param_1[0x25] + 0xb4) + 8);
        puVar5 = (undefined4 *)(*(int *)(param_1[0x25] + 0x2fc) + 0x2850);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar5 = *puVar1;
          puVar1 = puVar1 + 1;
          puVar5 = puVar5 + 1;
        }
        *(int *)(*(int *)(param_1[0x25] + 0x2fc) + 0x28fc) = *param_1;
        local_150 = *(int *)(param_1[0x25] + 0x2f4);
        if (local_150 != 0) {
          *(undefined4 *)(*(int *)(param_1[0x25] + 0x2fc) + 0x28d4) =
               *(undefined4 *)(local_150 + 0x160);
        }
      }
    }
    if (((*(int *)(param_1[0x25] + 0x2f4) == 0) ||
        ((((*(int *)param_1[0x25] != 0 && (*(int *)param_1[0x25] != 1)) &&
          (*(int *)param_1[0x25] != 2)) && (*(int *)param_1[0x25] != 4)))) ||
       ((*(char *)(*(int *)(param_1[0x25] + 0x2f4) + 0x787) != '\x01' ||
        ((*(int *)(param_1[0x24] + 8) != *(int *)(&DAT_005baa88 + *(int *)param_1[0x25] * 8) &&
         (*(int *)(param_1[0x24] + 8) != *(int *)(&DAT_005baa8c + *(int *)param_1[0x25] * 8))))))) {
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_44);
      (*(code *)**(undefined4 **)param_1[0x24])(0);
    }
    else {
      if (*(char *)(param_1[0x25] + 0x307) == '\x01') {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,9);
      }
      local_1e4 = 0x3f000000;
      local_1e0 = 0x3f000000;
      local_1dc = 0x3f000000;
      local_1d8 = 0x3f000000;
      _memset(&local_194,0,0x44);
      local_174 = local_1e4;
      local_170 = local_1e0;
      local_16c = local_1dc;
      local_168 = local_1d8;
      local_194 = local_1e4;
      local_190 = local_1e0;
      local_18c = local_1dc;
      local_188 = local_1d8;
      local_164 = local_1e4;
      local_160 = local_1e0;
      local_15c = local_1dc;
      local_158 = local_1d8;
      local_184 = local_1e4;
      local_180 = local_1e0;
      local_17c = local_1dc;
      local_178 = local_1d8;
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_194);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_44);
      (*(code *)**(undefined4 **)param_1[0x24])(1);
      local_1e4 = 0x3f000000;
      local_1e0 = 0x3e4ccccd;
      local_1dc = 0;
      local_1d8 = 0;
      local_174 = 0x3f000000;
      local_170 = 0x3e4ccccd;
      local_16c = 0;
      local_168 = 0;
      local_194 = 0x3f000000;
      local_190 = 0x3e4ccccd;
      local_18c = 0;
      local_188 = 0;
      local_164 = 0x3f000000;
      local_160 = 0x3e4ccccd;
      local_15c = 0;
      local_158 = 0;
      local_184 = 0x3f000000;
      local_180 = 0x3e4ccccd;
      local_17c = 0;
      local_178 = 0;
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_194);
      FUN_00430f10();
      thunk_FUN_0056230d(local_1d4,
                         *(float *)(*(int *)(param_1[0x25] + 0x2f4) + 0x38) + _DAT_005a439c,
                         *(float *)(*(int *)(param_1[0x25] + 0x2f4) + 0x38) + _DAT_005a439c,
                         *(float *)(*(int *)(param_1[0x25] + 0x2f4) + 0x38) + _DAT_005a439c);
      thunk_FUN_00561a5b(local_1d4,local_1d4,local_44);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_1d4);
      (*(code *)**(undefined4 **)param_1[0x24])(1);
      if (*(char *)(param_1[0x25] + 0x307) == '\x01') {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
    }
  }
  if (param_1[0x22] != 0) {
    FUN_004c16cc();
  }
  if (param_1[0x23] != 0) {
    FUN_004c16cc();
  }
  return 1;
}

