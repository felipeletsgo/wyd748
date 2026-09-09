// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c21e8 | Name: FUN_004c21e8


void __thiscall FUN_004c21e8(int *param_1,undefined4 param_2)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  undefined1 local_1ac [12];
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined4 local_198;
  undefined1 local_190 [12];
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined1 local_174 [12];
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined1 local_158 [12];
  undefined4 local_14c;
  undefined4 local_148;
  undefined4 local_144;
  undefined1 local_13c [12];
  undefined4 local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined1 local_120 [12];
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  undefined1 local_104 [12];
  undefined4 local_f8;
  undefined4 local_f4;
  undefined4 local_f0;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined1 local_cc [12];
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined1 local_5c [12];
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  FUN_004c5b60(param_2);
  piVar5 = param_1 + 2;
  thunk_FUN_00561a5b(param_1 + 0x12,piVar5,param_2);
  if ((param_1[0x25] == 0) || (*(int *)(param_1[0x25] + 0x2f4) == 0)) {
    if (param_1[0x23] != 0) {
      FUN_004c21e8(param_1 + 0x12);
    }
    if (param_1[0x22] != 0) {
      FUN_004c21e8(param_2);
    }
    return;
  }
  local_8 = -1;
  if (*(int *)param_1[0x25] == 0x14) {
    if (*param_1 == 8) {
      local_8 = 0;
    }
    else if (*param_1 == 7) {
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      puVar1 = (undefined4 *)FUN_00430f50(0x3d4ccccd,0x3ebd70a4,0x3dcccccd);
      local_14 = *puVar1;
      local_10 = puVar1[1];
      local_c = puVar1[2];
      puVar1 = (undefined4 *)FUN_00430f50(0x3d4ccccd,0x3ebd70a4,0xbdcccccd);
      local_20 = *puVar1;
      local_1c = puVar1[1];
      local_18 = puVar1[2];
      thunk_FUN_0056144f(&local_30,&local_14,param_1 + 0x12);
      thunk_FUN_0056144f(&local_40,&local_20,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xc4) = local_30;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 200) = local_2c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xcc) = local_28;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xd0) = local_40;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xd4) = local_3c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xd8) = local_38;
    }
    else if (*param_1 == 9) {
      local_8 = 1;
    }
    else if (*param_1 == 0xd) {
      local_8 = 2;
    }
    else if (*param_1 == 0x10) {
      local_8 = 3;
    }
    else if (*param_1 == 0xb) {
      local_8 = 4;
    }
    else if (*param_1 == 0x12) {
      local_8 = 5;
    }
    else if (*param_1 == 0x18) {
      local_8 = 6;
    }
    else if (*param_1 == 0x1f) {
      local_8 = 7;
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      FUN_00430f10();
      FUN_00430f50(0,0,0);
      thunk_FUN_0056144f(&local_50,local_5c,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_50;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_4c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_48;
    }
    if ((*(short *)(param_1[0x25] + 0x248) == 7) && (*param_1 == 3)) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    else if ((*(short *)(param_1[0x25] + 0x248) != 7) && (*param_1 == 4)) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if ((*(int *)param_1[0x25] == 0) || (*(int *)param_1[0x25] == 1)) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    if (*param_1 == 8) {
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      puVar1 = (undefined4 *)FUN_00430f50(0x3e23d70a,0x3da3d70a,0x3d0f5c29);
      local_84 = *puVar1;
      local_80 = puVar1[1];
      local_7c = puVar1[2];
      puVar1 = (undefined4 *)FUN_00430f50(0x3e23d70a,0x3da3d70a,0xbd0f5c29);
      local_90 = *puVar1;
      local_8c = puVar1[1];
      local_88 = puVar1[2];
      thunk_FUN_0056144f(&local_a0,&local_84,param_1 + 0x12);
      thunk_FUN_0056144f(&local_b0,&local_90,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xc4) = local_a0;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 200) = local_9c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xcc) = local_98;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xd0) = local_b0;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xd4) = local_ac;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xd8) = local_a8;
      puVar1 = (undefined4 *)FUN_00430f50(0x3ecccccd,0x3e4ccccd,0);
      local_78 = *puVar1;
      local_74 = puVar1[1];
      local_70 = puVar1[2];
      local_8 = 0;
    }
    else if (*param_1 == 7) {
      local_8 = 1;
    }
    else if (*param_1 == *(int *)(&DAT_005baa88 + *(int *)param_1[0x25] * 8)) {
      local_8 = 6;
    }
    else if (*param_1 == *(int *)(&DAT_005baa8c + *(int *)param_1[0x25] * 8)) {
      local_8 = 7;
    }
    else if (*param_1 == 0x1d) {
      local_8 = 2;
    }
    else if (*param_1 == 0x22) {
      local_8 = 3;
    }
    if (*param_1 == 6) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_6c,&local_78,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_6c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_68;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_64;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 3) {
    if (*param_1 == 9) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 8) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    if (*param_1 == 0x21) {
      local_8 = 0;
    }
    if (*param_1 == 0x22) {
      local_8 = 1;
    }
    if (*param_1 == 0x23) {
      local_8 = 2;
    }
    if (*param_1 == 0x28) {
      local_8 = 3;
    }
    if (*param_1 == 0x29) {
      local_8 = 4;
    }
    if (*param_1 == 0x2a) {
      local_8 = 5;
    }
    if (*param_1 == 5) {
      local_8 = 6;
    }
    if (*param_1 == 0x10) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_c0,local_cc,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_c0;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_bc;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_b8;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (((*(int *)param_1[0x25] == 0x1d) || (*(int *)param_1[0x25] == 2)) ||
     (*(int *)param_1[0x25] == 4)) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    if (*param_1 == 8) {
      local_8 = 0;
      if (*(int *)param_1[0x25] == 2) {
        puVar1 = (undefined4 *)FUN_00430f50(0,0x3e4ccccd,0);
        local_e8 = *puVar1;
        local_e4 = puVar1[1];
        local_e0 = puVar1[2];
      }
      if (*(int *)param_1[0x25] == 4) {
        puVar1 = (undefined4 *)FUN_00430f50(0xbf000000,0,0);
        local_e8 = *puVar1;
        local_e4 = puVar1[1];
        local_e0 = puVar1[2];
      }
    }
    if (*param_1 == 7) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    if ((*(int *)param_1[0x25] == 2) && (*param_1 == 0x1b)) {
      local_8 = 1;
    }
    if ((*(int *)param_1[0x25] == 2) && (*param_1 == 0x20)) {
      local_8 = 2;
    }
    if ((*(int *)param_1[0x25] == 4) && (*param_1 == 0x19)) {
      local_8 = 1;
    }
    if ((*(int *)param_1[0x25] == 4) && (*param_1 == 0x1f)) {
      local_8 = 2;
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_dc,&local_e8,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_dc;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_d8;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_d4;
    }
    if ((*(int *)param_1[0x25] == 0x1d) && (*param_1 == 5)) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 0x1a) {
    FUN_00430f10();
    FUN_00430f50(0x3e800000,0x3e19999a,0);
    if (*param_1 == 0xe) {
      local_8 = 0;
    }
    if (*param_1 == 0x16) {
      local_8 = 1;
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_f8,local_104,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_f8;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_f4;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_f0;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 7) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    if (*param_1 == 6) {
      local_8 = 0;
    }
    if (*param_1 == 7) {
      local_8 = 1;
    }
    if (*param_1 == 0xc) {
      local_8 = 2;
    }
    if (*param_1 == 0x12) {
      local_8 = 3;
    }
    if (*param_1 == 0x17) {
      local_8 = 4;
    }
    if (*param_1 == 0x1c) {
      local_8 = 5;
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_114,local_120,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_114;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_110;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_10c;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if ((*(int *)param_1[0x25] == 0x19) || (*(int *)param_1[0x25] == 0x1c)) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    local_8 = 0;
    if (((*param_1 == 6) && (*(int *)param_1[0x25] == 0x19)) ||
       ((*param_1 == 10 && (*(int *)param_1[0x25] == 0x1c)))) {
      thunk_FUN_0056144f(&local_130,local_13c,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_130;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_12c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_128;
    }
    if (*(int *)param_1[0x25] == 0x19) {
      if (*param_1 == 3) {
        piVar3 = param_1 + 0x12;
        piVar4 = (int *)(param_1[0x25] + 0x74);
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *piVar4 = *piVar3;
          piVar3 = piVar3 + 1;
          piVar4 = piVar4 + 1;
        }
      }
    }
    else if ((*(int *)param_1[0x25] == 0x1c) && (*param_1 == 3)) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 0x16) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    local_8 = 0;
    if (*param_1 == 0x19) {
      thunk_FUN_0056144f(&local_14c,local_158,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_14c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_148;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_144;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 6) {
    FUN_00430f10();
    FUN_00430f50(0,0x3d4ccccd,0x3d4ccccd);
    local_8 = 0;
    if (*param_1 == 0x12) {
      thunk_FUN_0056144f(&local_168,local_174,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_168;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_164;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_160;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 0x1e) {
    if (*param_1 == 4) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 0x26) {
    FUN_00430f10();
    FUN_00430f50(0,0,0);
    if (*param_1 == 0x21) {
      local_8 = 0;
    }
    if (*param_1 == 0x10) {
      local_8 = 1;
    }
    if (*param_1 == 0xb) {
      local_8 = 2;
    }
    if (*param_1 == 0x15) {
      local_8 = 3;
    }
    if (*param_1 == 0x1a) {
      local_8 = 4;
    }
    if (*param_1 == 5) {
      local_8 = 5;
    }
    if (*param_1 == 2) {
      local_8 = 6;
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_184,local_190,param_1 + 0x12);
      if (*(int *)(*(int *)(param_1[0x25] + 0x2f4) + 0x5c) == 0x26) {
        *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_184;
        *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_180;
        *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_17c;
      }
      else {
        *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xdc + local_8 * 0xc) = local_184;
        *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xe0 + local_8 * 0xc) = local_180;
        *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0xe4 + local_8 * 0xc) = local_17c;
      }
    }
    if (*param_1 == 3) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] == 0x29) {
    FUN_00430f10();
    FUN_00430f50(0,0,0xbecccccd);
    if (*param_1 == 4) {
      local_8 = 0;
    }
    if (*param_1 == 9) {
      local_8 = 1;
    }
    if (*param_1 == 0xc) {
      local_8 = 2;
    }
    if ((-1 < local_8) && (local_8 < 10)) {
      thunk_FUN_0056144f(&local_1a0,local_1ac,param_1 + 0x12);
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 100 + local_8 * 0xc) = local_1a0;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x68 + local_8 * 0xc) = local_19c;
      *(undefined4 *)(*(int *)(param_1[0x25] + 0x2f4) + 0x6c + local_8 * 0xc) = local_198;
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*(int *)param_1[0x25] != 0x27) {
    if (*(int *)param_1[0x25] != 0x28) {
      FUN_013d4000(piVar5);
      return;
    }
    if (*param_1 == 0xf) {
      piVar3 = param_1 + 0x12;
      piVar4 = (int *)(param_1[0x25] + 0x74);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *piVar4 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar4 = piVar4 + 1;
      }
    }
    FUN_004c3642(piVar5);
    return;
  }
  if (*param_1 == 7) {
    piVar3 = param_1 + 0x12;
    piVar4 = (int *)(param_1[0x25] + 0x74);
    for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *piVar4 = *piVar3;
      piVar3 = piVar3 + 1;
      piVar4 = piVar4 + 1;
    }
  }
  FUN_004c3642(piVar5);
  return;
}

