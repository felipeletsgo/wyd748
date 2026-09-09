// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052b97d | Name: FUN_0052b97d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0052b97d(int param_1,int param_2)

{
  short sVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  bool bVar5;
  int iVar6;
  void *pvVar7;
  int local_14c;
  undefined1 local_120 [32];
  int local_100;
  int local_fc;
  int local_f8;
  short local_f4;
  int local_f0;
  undefined4 local_ec;
  int local_e8;
  undefined4 local_e4;
  int local_e0;
  int local_dc;
  undefined4 local_d8;
  int local_d4;
  undefined1 local_d0 [128];
  int local_50;
  int local_4c;
  undefined1 local_48 [32];
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a23a5;
  local_10 = ExceptionList;
  local_20 = param_2;
  if ((*(uint *)(param_1 + 0x20) < 1000) && ((*(byte *)(param_2 + 0x12) & 1) == 1)) {
    ExceptionList = &local_10;
    *(undefined1 *)(param_1 + 0x795) = 1;
  }
  else {
    ExceptionList = &local_10;
    *(undefined1 *)(param_1 + 0x795) = 0;
  }
  local_18 = 0;
  iVar6 = FUN_00494dcf();
  if (iVar6 == 30000) {
    local_18 = DAT_0067cf38;
  }
  if (local_18 != 0) {
    local_1c = *(int *)(local_18 + 0x2872c);
    if (local_1c != 0) {
      for (local_24 = 0; local_24 < *(int *)(local_1c + 0x210); local_24 = local_24 + 1) {
        local_28 = *(int *)(local_1c + 0x214 + local_24 * 4);
        if (*(int *)(local_28 + 0xe50) == *(int *)(param_1 + 0x20)) {
          FUN_0058f078(local_48,&DAT_005bb80c,*(short *)(local_20 + 0xc) + 1);
          (**(code **)(**(int **)(local_28 + 0xe60) + 0x80))(local_48,0);
          if (*(short *)(local_20 + 0x14) < *(short *)(local_20 + 0x18)) {
            *(undefined2 *)(local_20 + 0x18) = *(undefined2 *)(local_20 + 0x14);
          }
          (**(code **)(**(int **)(local_28 + 0xe68) + 0x8c))((int)*(short *)(local_20 + 0x14));
          (**(code **)(**(int **)(local_28 + 0xe68) + 0x88))((int)*(short *)(local_20 + 0x18));
          break;
        }
      }
    }
    iVar6 = __ftol();
    if (((iVar6 >> 7 < 0x11) || (iVar6 = __ftol(), 0x13 < iVar6 >> 7)) ||
       (iVar6 = __ftol(), iVar6 >> 7 < 0x1e)) {
      bVar5 = false;
    }
    else {
      bVar5 = true;
    }
    if (bVar5) {
      iVar6 = __ftol();
      if ((iVar6 >> 7 == 0x12) && (iVar6 = __ftol(), iVar6 >> 7 == 0x1e)) {
        bVar5 = true;
      }
      else {
        bVar5 = false;
      }
      if (bVar5) {
        if (*(short *)(*(int *)(local_18 + 0x4c) + 0x680) != *(short *)(local_20 + 0x4a)) {
          *(undefined2 *)(local_20 + 0x4a) = 0;
        }
      }
      else if (param_1 != *(int *)(DAT_0067cf38 + 0x4c)) {
        *(undefined2 *)(local_20 + 0x4a) = 0;
      }
    }
    iVar6 = __ftol();
    if ((iVar6 >> 7 == 0x11) && (iVar6 = __ftol(), iVar6 >> 7 == 0x1c)) {
      bVar5 = true;
    }
    else {
      bVar5 = false;
    }
    if ((bVar5) && (param_1 != *(int *)(DAT_0067cf38 + 0x4c))) {
      *(undefined2 *)(local_20 + 0x4a) = 0;
    }
    FUN_0058f220(param_1 + 0x444,local_20 + 0xc,0x1c);
    if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
      if ((((*(short *)(DAT_013b71e8 + 0x72c) < *(short *)(local_20 + 0xc)) &&
           (iVar6 = FUN_004ff388(), iVar6 == 2)) &&
          (local_4c = DAT_013b71e8 + 0x6ec,
          *(char *)(local_18 + 0x276c1 + (int)*(short *)(DAT_013b71e8 + 0x72c)) != '\0')) &&
         (((5 < _DAT_005d04dc % 10 && (0x15d < *(short *)(local_20 + 0xc))) ||
          ((_DAT_005d04dc % 10 < 6 && (*(short *)(local_20 + 0xc) < 0x15e)))))) {
        (**(code **)(**(int **)(local_18 + 0x27d30) + 0x60))(1);
        local_50 = -0x555501;
        if (*(char *)(local_18 + 0x276c1 + (int)*(short *)(local_4c + 0x40)) == 'd') {
          local_50 = FUN_0049eada(local_d0,s_UI_QuestMessage_txt_005bb810,
                                  *(short *)(local_4c + 0x40) + 2,
                                  (int)*(short *)(local_4c + 0x5c) % 10);
        }
        else {
          FUN_0058f078(local_d0,&DAT_00a3daf8);
        }
        if (local_50 != -0x1000000) {
          FUN_00403df2(local_d0,3000);
          pvVar7 = operator_new(0xe50);
          local_8 = 0;
          if (pvVar7 == (void *)0x0) {
            local_14c = 0;
          }
          else {
            local_14c = FUN_00407203(local_d0,local_50,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
          }
          local_8 = 0xffffffff;
          local_d4 = local_14c;
          if (local_14c != 0) {
            FUN_00408d33(local_14c);
          }
        }
      }
      FUN_0058f220(DAT_013b71e8 + 0x72c,local_20 + 0xc,0x1c);
      *(undefined2 *)(param_1 + 0x682) = *(undefined2 *)(DAT_013b71e8 + 0x7a8);
      *(undefined1 *)(DAT_013b71e8 + 0x9d9) = *(undefined1 *)(local_20 + 0x56);
    }
    sVar1 = *(short *)(local_20 + 0x4a);
    local_14 = CONCAT22(local_14._2_2_,sVar1);
    *(short *)(param_1 + 0x680) = sVar1;
    if (sVar1 == 0) {
      *(undefined4 *)(param_1 + 0x878) = 1;
      (**(code **)(**(int **)(param_1 + 0x7e4) + 0x60))(0);
    }
    else {
      local_d8 = FUN_00552e21((int)*(short *)(param_1 + 0x682));
      *(undefined4 *)(param_1 + 0x878) = 0;
      *(undefined4 *)(param_1 + 0x7e8) = local_d8;
      *(uint *)(param_1 + 0x7f0) = local_14 & 0xfff;
      *(undefined4 *)(param_1 + 0x7e8) = local_d8;
      *(int *)(param_1 + 0x7ec) = (int)(local_14 & 0xffff) >> 0xc;
      *(undefined2 *)(param_1 + 0x880) = *(undefined2 *)(param_1 + 0x682);
      local_dc = DAT_0067cf38;
      if ((DAT_0067cf38 != 0) && (iVar6 = FUN_0040c0f0(), iVar6 == 0)) {
        FUN_0047dee5(param_1 + 0x7e4);
      }
    }
    FUN_005277a7((float)(int)*(short *)(param_1 + 0x45a));
    if (*(int *)(param_1 + 0x24c) == 0x28) {
      *(float *)(param_1 + 0x38) = *(float *)(param_1 + 0x38) * _DAT_005a55f4;
    }
    if ((param_1 == *(int *)(DAT_0067cf38 + 0x4c)) && (local_18 != 0)) {
      local_ec = 0;
      local_e4 = 0;
      for (local_e8 = 0; local_e8 < 0x10; local_e8 = local_e8 + 1) {
        if (*(short *)(param_1 + 0x684 + local_e8 * 2) != 0) {
          local_ec = 1;
          break;
        }
      }
      for (local_e8 = 0; local_e8 < 0x10; local_e8 = local_e8 + 1) {
        if (*(short *)(local_20 + 0x2a + local_e8 * 2) != 0) {
          local_e4 = 1;
          break;
        }
      }
      local_e0 = (**(code **)(*DAT_0092e654 + 8))();
      for (local_f0 = 0; local_f0 < 0x10; local_f0 = local_f0 + 1) {
        local_f4 = (((*(ushort *)(param_1 + 0x684 + local_f0 * 2) & 0xff) - 1) * 8 + 4) -
                   (short)((ulonglong)(uint)(local_e0 - *(int *)(local_18 + 0x27480 + local_f0 * 4))
                          / 1000);
        if (((int)((int)local_f4 + ((int)local_f4 >> 0x1f & 7U)) >> 3 !=
             (*(ushort *)(local_20 + 0x2a + local_f0 * 2) & 0xff)) ||
           ((int)(uint)*(ushort *)(param_1 + 0x684 + local_f0 * 2) >> 8 !=
            (int)(uint)*(ushort *)(local_20 + 0x2a + local_f0 * 2) >> 8)) {
          FUN_0058f220(param_1 + 0x684 + local_f0 * 2,local_20 + 0x2a + local_f0 * 2,2);
          *(int *)(local_18 + 0x27480 + local_f0 * 4) = local_e0;
        }
      }
    }
    else {
      FUN_0058f220(param_1 + 0x684,local_20 + 0x2a,0x20);
    }
    if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
      *(undefined1 *)(DAT_013b71e8 + 0x9d2) = *(undefined1 *)(local_20 + 0x28);
      *(undefined1 *)(DAT_013b71e8 + 0x9d3) = *(undefined1 *)(local_20 + 0x29);
    }
    FUN_0052433d();
    FUN_004fff0b(*(undefined2 *)(local_20 + 0x4c));
    if ((param_1 == *(int *)(DAT_0067cf38 + 0x4c)) && (local_18 != 0)) {
      if (*(char *)(param_1 + 0x246) == '\0') {
        FUN_004fa8a1(*(undefined4 *)(local_18 + 0x26e7c));
      }
    }
    else if (*(char *)(param_1 + 0x246) == '\0') {
      FUN_004fa8a1(0);
    }
    if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
      local_f8 = DAT_013b71e8 + 0x6ec;
      FUN_0058f220(DAT_013b71e8 + 0x9dc,local_20 + 0x4e,4);
      *(undefined2 *)(DAT_013b71e8 + 0x2c4 + *(char *)(DAT_013b71e8 + 0xef4) * 2) =
           (undefined2)local_14;
      if (((local_14 & 0xffff) == 0) && (*(short *)(local_f8 + 0xbc) == 0)) {
        *(undefined2 *)(DAT_013b71e8 + 0xf10) = 0xffff;
      }
      FUN_0058f220(local_f8 + 0x40,local_20 + 0xc,0x1c);
      if (*(char *)(param_1 + 0x79c) == '\x01') {
        local_100 = FUN_0054cd07(local_f8 + 0xcc,0x50);
        (**(code **)(**(int **)(param_1 + 0x438) + 0x88))(local_100);
        local_fc = DAT_0067cf38;
        if ((DAT_005b892c == 2) && (DAT_0067cf38 != 0)) {
          (**(code **)(**(int **)(DAT_0067cf38 + 0x27ad4) + 0x88))(local_100);
          if (local_100 < 0) {
            local_100 = 0;
          }
          FUN_0058f078(local_120,&DAT_005bb824,local_100);
          (**(code **)(**(int **)(local_fc + 0x27ac0) + 0x80))(local_120,0);
        }
      }
      if (local_18 != 0) {
        FUN_00480c25();
        *(undefined2 *)(local_18 + 0x26e80) = *(undefined2 *)(local_20 + 0x52);
        *(undefined2 *)(local_18 + 0x26e82) = *(undefined2 *)(local_20 + 0x54);
      }
      if (local_18 != 0) {
        FUN_004431e4(0);
      }
    }
    if (*(char *)(param_1 + 0x79c) != '\0') {
      iVar6 = *(int *)(param_1 + 0x19c);
      uVar2 = *(undefined4 *)(iVar6 + 0x18);
      uVar3 = *(undefined4 *)(iVar6 + 0x1c);
      uVar4 = *(undefined4 *)(iVar6 + 0x20);
      FUN_00527848();
      FUN_004be157(uVar2,uVar3,uVar4);
    }
    *(undefined1 *)(param_1 + 0x7e2) = *(undefined1 *)(local_20 + 0x5a);
  }
  ExceptionList = local_10;
  return 1;
}

