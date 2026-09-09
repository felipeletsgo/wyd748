// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00441823 | Name: FUN_00441823


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00441823(int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  int local_138;
  int local_134;
  int local_110;
  int local_10c;
  int local_108;
  int local_104;
  int local_d0;
  undefined4 local_cc;
  int *local_c8;
  undefined1 local_a0 [128];
  undefined4 local_20;
  int local_1c;
  int local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f57c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x27d38) != 0) {
    ExceptionList = &local_10;
    pvVar2 = operator_new(0xf60);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_c8 = (int *)0x0;
    }
    else {
      local_c8 = (int *)FUN_00407d62(&DAT_005b7a5c,&DAT_00a39978,&DAT_00a399f8,&DAT_00a39a78,
                                     &DAT_00a39af8,&DAT_00a39b78,0x5588ffff,1);
    }
    local_8 = 0xffffffff;
    local_14 = local_c8;
    (**(code **)(*local_c8 + 0x44))(0x338);
    (**(code **)(*local_14 + 0x70))(0x40800000,0x41e00000);
    FUN_0054ac09(local_14);
    pvVar2 = operator_new(0xb8);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_cc = 0;
    }
    else {
      local_cc = FUN_0040833b(0x40800000,0x43a00000,0x43fa0000,0x41800000,1,10,1,1,10,1,10);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x27d68) = local_cc;
    (**(code **)(**(int **)(param_1 + 0x27d68) + 0x44))(0x33a);
    if (*(int *)(param_1 + 0x28) == 0) {
      local_d0 = 0;
    }
    else {
      local_d0 = *(int *)(param_1 + 0x28) + 0x24;
    }
    (**(code **)(**(int **)(param_1 + 0x27d68) + 0x50))(local_d0);
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x27d68));
    (**(code **)(**(int **)(param_1 + 0x27d38) + 0x60))(0);
  }
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x360);
  *(undefined4 *)(param_1 + 0x27ca8) = uVar3;
  if (*(int *)(param_1 + 0x27ca8) != 0) {
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x361);
    *(undefined4 *)(param_1 + 0x27cec) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x363);
    *(undefined4 *)(param_1 + 0x27cac) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x364);
    *(undefined4 *)(param_1 + 0x27cbc) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x365);
    *(undefined4 *)(param_1 + 0x27cb0) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x366);
    *(undefined4 *)(param_1 + 0x27cc0) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x367);
    *(undefined4 *)(param_1 + 0x27cb4) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x368);
    *(undefined4 *)(param_1 + 0x27cc4) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x369);
    *(undefined4 *)(param_1 + 0x27cb8) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x36a);
    *(undefined4 *)(param_1 + 0x27cc8) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x36b);
    *(undefined4 *)(param_1 + 0x27cf0) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x36e);
    *(undefined4 *)(param_1 + 0x27cf4) = uVar3;
    *(undefined1 *)(*(int *)(param_1 + 0x27cf4) + 0x1ec) = 1;
    *(undefined1 *)(*(int *)(param_1 + 0x27cf4) + 0x1ed) = 1;
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0x27cf4) + 0x1f0) + 0x70))(0xc1d00000,0xc1880000);
      (**(code **)(**(int **)(*(int *)(param_1 + 0x27cf0) + 0x1f0) + 0x70))(0xc1d00000,0xc1880000);
      *(undefined1 *)(*(int *)(param_1 + 0x27cf4) + 0x1ec) = 0;
      (**(code **)(**(int **)(param_1 + 0x27cf0) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a4344,
                 ((float)*(uint *)(DAT_005ccf9c + 0x2a508) -
                 *(float *)(*(int *)(param_1 + 0x27cf4) + 0x58)) - _DAT_005a4348);
      *(undefined1 *)(*(int *)(param_1 + 0x27cf0) + 0x1ed) = 1;
      (**(code **)(**(int **)(param_1 + 0x27cf4) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a4340,
                 ((float)*(uint *)(DAT_005ccf9c + 0x2a508) -
                 *(float *)(*(int *)(param_1 + 0x27cf4) + 0x58)) - _DAT_005a4348);
    }
    FUN_00442dd0(*(undefined4 *)(param_1 + 0x27cbc),s_UI_interface_txt_005b7a60);
    FUN_00442dd0(*(undefined4 *)(param_1 + 0x27cc0),s_UI_command_txt_005b7a74);
    FUN_00442dd0(*(undefined4 *)(param_1 + 0x27cc4),s_UI_etc_txt_005b7a84);
    if (*(int *)(param_1 + 0x27cc0) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27cc0) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27cc4) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27cc4) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27cc8) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27cc8) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27cf0) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27cf0) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27cf4) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27cf4) + 0x60))(0);
    }
    (**(code **)(**(int **)(param_1 + 0x27ca8) + 0x60))(0);
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(param_1 + 0x27ca8) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(*(int *)(param_1 + 0x27ca8) + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(*(int *)(param_1 + 0x27ca8) + 0x58) * _DAT_005a34a0);
      uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x17b3);
      *(undefined4 *)(param_1 + 0x27cd0) = uVar3;
      for (local_18 = 0; local_18 < 3; local_18 = local_18 + 1) {
        uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_18 + 0x17b0);
        *(undefined4 *)(param_1 + 0x27cd4 + local_18 * 4) = uVar3;
        uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_18 + 0x17ba);
        *(undefined4 *)(param_1 + 0x27ce0 + local_18 * 4) = uVar3;
        if (0 < local_18) {
          (**(code **)(**(int **)(param_1 + 0x27cd4 + local_18 * 4) + 0x60))(0);
        }
      }
      FUN_00442dd0(*(undefined4 *)(param_1 + 0x27ce0),s_UI_interface1_txt_005b7a90);
      FUN_00442dd0(*(undefined4 *)(param_1 + 0x27ce4),s_UI_interface2_txt_005b7aa4);
      FUN_00442dd0(*(undefined4 *)(param_1 + 0x27ce8),s_UI_interface3_txt_005b7ab8);
      *(undefined4 *)(param_1 + 0x27ccc) = 0;
    }
  }
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x140);
  *(undefined4 *)(param_1 + 0x27cf8) = uVar3;
  if (*(int *)(param_1 + 0x27cf8) != 0) {
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x149);
    *(undefined4 *)(param_1 + 0x27d00) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x141);
    *(undefined4 *)(param_1 + 0x27d10) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x142);
    *(undefined4 *)(param_1 + 0x27d20) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x14a);
    *(undefined4 *)(param_1 + 0x27d04) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x145);
    *(undefined4 *)(param_1 + 0x27d14) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x146);
    *(undefined4 *)(param_1 + 0x27d24) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x14b);
    *(undefined4 *)(param_1 + 0x27d08) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x147);
    *(undefined4 *)(param_1 + 0x27d18) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x148);
    *(undefined4 *)(param_1 + 0x27d28) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x14d);
    *(undefined4 *)(param_1 + 0x27d0c) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x14f);
    *(undefined4 *)(param_1 + 0x27d1c) = uVar3;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x14e);
    *(undefined4 *)(param_1 + 0x27d2c) = uVar3;
    if (*(int *)(param_1 + 0x27d10) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d10) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d14) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d14) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d18) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d18) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d1c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d1c) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d20) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d20) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d24) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d24) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d28) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d28) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27d2c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d2c) + 0x60))(0);
    }
    (**(code **)(**(int **)(param_1 + 0x27cf8) + 0x60))(0);
    if (*(int *)(param_1 + 0x28) == 0) {
      local_104 = 0;
    }
    else {
      local_104 = *(int *)(param_1 + 0x28) + 0x24;
    }
    (**(code **)(**(int **)(param_1 + 0x27d10) + 0x50))(local_104);
    if (*(int *)(param_1 + 0x28) == 0) {
      local_108 = 0;
    }
    else {
      local_108 = *(int *)(param_1 + 0x28) + 0x24;
    }
    (**(code **)(**(int **)(param_1 + 0x27d14) + 0x50))(local_108);
    if (*(int *)(param_1 + 0x28) == 0) {
      local_10c = 0;
    }
    else {
      local_10c = *(int *)(param_1 + 0x28) + 0x24;
    }
    (**(code **)(**(int **)(param_1 + 0x27d18) + 0x50))(local_10c);
    if (*(int *)(param_1 + 0x27d1c) != 0) {
      if (*(int *)(param_1 + 0x28) == 0) {
        local_110 = 0;
      }
      else {
        local_110 = *(int *)(param_1 + 0x28) + 0x24;
      }
      (**(code **)(**(int **)(param_1 + 0x27d1c) + 0x50))(local_110);
    }
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(param_1 + 0x27cf8) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(*(int *)(param_1 + 0x27cf8) + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(*(int *)(param_1 + 0x27cf8) + 0x58) * _DAT_005a34a0);
    }
    else {
      *(undefined4 *)(*(int *)(param_1 + 0x27cf8) + 0x3c) = 1;
      *(undefined4 *)(*(int *)(param_1 + 0x27d20) + 0x3c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x27d24) + 0x3c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x27d28) + 0x3c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x27d2c) + 0x3c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x27d10) + 0x3c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x27d14) + 0x3c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x27d18) + 0x3c) = 0;
      if (*(int *)(param_1 + 0x27d1c) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x27d1c) + 0x3c) = 0;
      }
    }
    _memset((void *)(param_1 + 0x276c0),0,500);
    FUN_0049ec64(param_1 + 0x276c0,s_UI_QuestSubjects_txt_005b7acc,0x61);
    FUN_0049ec64(param_1 + 0x276c0,s_UI_QuestSubjects2_txt_005b7ae4,0x62);
    FUN_0049ec64(param_1 + 0x276c0,s_UI_QuestSubjects3_txt_005b7afc,99);
    FUN_0049ec64(param_1 + 0x276c0,s_UI_QuestSubjects4_txt_005b7b14,0x65);
    FUN_0049ec64(param_1 + 0x276c0,s_UI_QuestMessage_txt_005b7b2c,100);
    uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x14c);
    *(undefined4 *)(param_1 + 0x27d30) = uVar3;
    *(undefined1 *)(*(int *)(param_1 + 0x27d30) + 0x1ec) = 1;
    *(undefined1 *)(*(int *)(param_1 + 0x27d30) + 0x1ed) = 1;
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0x27d30) + 0x1f0) + 0x70))(0xc1200000,0xc1400000);
      *(undefined1 *)(*(int *)(param_1 + 0x27d30) + 0x1ed) = 1;
      (**(code **)(**(int **)(param_1 + 0x27d30) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                 *(float *)(*(int *)(param_1 + 0x27d30) + 0x54),
                 ((float)*(uint *)(DAT_005ccf9c + 0x2a508) -
                 *(float *)(*(int *)(param_1 + 0x27d30) + 0x58)) - _DAT_005a4348);
    }
    iVar1 = DAT_013b71e8;
    local_1c = DAT_013b71e8 + 0x6ec;
    if (*(short *)(DAT_013b71e8 + 0x72c) == 0) {
      FUN_0049e77b(*(undefined4 *)(param_1 + 0x27d10),s_UI_QuestSubjects_txt_005b7b40,
                   *(short *)(DAT_013b71e8 + 0x72c) + 1,(int)*(short *)(DAT_013b71e8 + 0x748) % 10);
      FUN_0049e77b(*(undefined4 *)(param_1 + 0x27d14),s_UI_QuestSubjects2_txt_005b7b58,
                   *(short *)(iVar1 + 0x72c) + 1,(int)*(short *)(iVar1 + 0x748) % 10);
      FUN_0049e77b(*(undefined4 *)(param_1 + 0x27d18),s_UI_QuestSubjects3_txt_005b7b70,
                   *(short *)(iVar1 + 0x72c) + 1,(int)*(short *)(iVar1 + 0x748) % 10);
      FUN_0049e77b(*(undefined4 *)(param_1 + 0x27d1c),s_UI_QuestSubjects4_txt_005b7b88,
                   *(short *)(iVar1 + 0x72c) + 1,(int)*(short *)(iVar1 + 0x748) % 10);
      (**(code **)(**(int **)(param_1 + 0x27d30) + 0x60))(1);
      local_20 = 0xffaaaaff;
      if (*(char *)(param_1 + 0x276c0 + (int)*(short *)(iVar1 + 0x72c)) == 'd') {
        local_20 = FUN_0049eada(local_a0,s_UI_QuestMessage_txt_005b7ba0,
                                *(short *)(iVar1 + 0x72c) + 1,(int)*(short *)(iVar1 + 0x748) % 10);
      }
      else {
        FUN_0058f078(local_a0,&DAT_00a3daf8);
      }
      FUN_00403df2(local_a0,3000);
      pvVar2 = operator_new(0xe50);
      local_8 = 2;
      if (pvVar2 == (void *)0x0) {
        local_134 = 0;
      }
      else {
        local_134 = FUN_00407203(local_a0,local_20,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      if ((local_134 != 0) && (*(int *)(param_1 + 0x27ae8) != 0)) {
        FUN_00408d33(local_134);
      }
    }
    else if (*(int *)(param_1 + 0x27d30) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d30) + 0x60))(0);
    }
  }
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3100);
  *(undefined4 *)(param_1 + 0x28c10) = uVar3;
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3101);
  *(undefined4 *)(param_1 + 0x28c14) = uVar3;
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3105);
  *(undefined4 *)(param_1 + 0x28c18) = uVar3;
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3106);
  *(undefined4 *)(param_1 + 0x28c1c) = uVar3;
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3107);
  *(undefined4 *)(param_1 + 0x28c20) = uVar3;
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3108);
  *(undefined4 *)(param_1 + 0x28c24) = uVar3;
  if (*(int *)(param_1 + 0x28c14) != 0) {
    if (*(int *)(param_1 + 0x28) == 0) {
      local_138 = 0;
    }
    else {
      local_138 = *(int *)(param_1 + 0x28) + 0x24;
    }
    (**(code **)(**(int **)(param_1 + 0x28c14) + 0x50))(local_138);
  }
  if (*(int *)(param_1 + 0x28c10) != 0) {
    (**(code **)(**(int **)(param_1 + 0x28c10) + 0x60))(0);
  }
  ExceptionList = local_10;
  return;
}

