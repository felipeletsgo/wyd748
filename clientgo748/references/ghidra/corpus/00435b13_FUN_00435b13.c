// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00435b13 | Name: FUN_00435b13


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00435b13(int *param_1)

{
  undefined1 uVar1;
  void *pvVar2;
  int iVar3;
  undefined2 extraout_var;
  int iVar4;
  undefined2 extraout_var_00;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_af8;
  int local_af4;
  int local_ad0;
  int local_acc;
  int local_ac0;
  int local_abc;
  int local_aa8;
  int local_aa4;
  int local_aa0;
  int local_a9c;
  int local_a98;
  int local_a94;
  int local_a90;
  int local_a8c;
  int local_a68;
  int local_a64;
  int local_a60;
  int local_a5c;
  int local_a58;
  int local_a54;
  int local_a50;
  int local_a4c;
  int local_a44;
  int local_a40;
  int local_a3c;
  undefined4 local_a2c;
  undefined4 local_a28;
  undefined4 local_a24;
  undefined4 local_a20;
  undefined4 local_a1c;
  undefined4 local_a18;
  undefined4 local_a14;
  undefined4 local_a10;
  undefined4 local_a0c;
  undefined4 local_a08;
  undefined4 local_a04;
  undefined4 local_a00;
  undefined4 local_9fc;
  undefined4 local_9f8;
  undefined4 local_9f4;
  undefined4 local_9f0;
  undefined4 local_9ec;
  int local_9e8;
  int local_9e4;
  int local_9e0;
  int local_834;
  int local_828;
  int local_81c;
  int local_800;
  int local_7fc;
  int local_7e8;
  int local_7e4;
  int local_7d0;
  int local_7cc;
  int local_7b8;
  int local_7b4;
  int local_7b0;
  int local_7ac;
  int local_7a8;
  int local_7a4;
  int local_7a0;
  int local_79c;
  int local_798;
  int local_794;
  int local_790;
  int local_78c;
  int local_788;
  int local_784;
  int local_780;
  int local_77c;
  int local_778;
  int local_774;
  int local_770;
  int local_76c;
  int local_510;
  int local_50c;
  undefined1 local_500 [4];
  undefined2 local_4fc;
  undefined2 local_4fa;
  undefined1 local_4f4 [16];
  undefined1 local_4e4 [100];
  int local_480;
  int local_47c;
  int *local_478;
  int local_474;
  int local_470;
  void *local_46c;
  undefined4 local_468;
  undefined4 local_464;
  void *local_460;
  undefined4 local_45c;
  void *local_458;
  void *local_454;
  undefined4 local_450;
  undefined4 local_44c;
  void *local_448;
  undefined4 local_444;
  void *local_440;
  void *local_43c;
  undefined4 local_438;
  undefined4 local_434;
  void *local_430;
  undefined4 local_42c;
  void *local_428;
  void *local_424;
  undefined4 local_420;
  undefined4 local_41c;
  void *local_418;
  void *local_414;
  undefined4 local_410;
  undefined4 local_40c;
  void *local_408;
  void *local_404;
  undefined4 local_400;
  void *local_3fc;
  undefined4 local_3f8;
  void *local_3f4;
  undefined4 local_3f0;
  void *local_3ec;
  undefined4 local_3e8;
  int local_3e4;
  int local_3e0;
  int local_3dc;
  int local_3d8;
  int local_3d4;
  int local_3d0;
  int local_3cc;
  int local_3c8;
  int local_3c4;
  int local_3c0;
  int *local_3bc;
  int *local_3b8;
  int *local_3b4;
  int *local_3b0;
  int local_3ac;
  int local_3a8;
  int *local_3a4;
  int local_3a0;
  int local_39c;
  int local_398;
  int local_394;
  int local_390;
  int local_38c;
  int local_388;
  int local_384;
  int local_380;
  int local_37c;
  int local_378;
  int local_374;
  int local_370;
  int local_36c;
  int local_368;
  int local_364;
  int local_360;
  int local_35c;
  undefined4 local_358;
  int local_354;
  int local_350;
  int local_34c;
  int local_348;
  int *local_344;
  int local_340;
  int *local_33c;
  int local_338;
  int local_334;
  int local_330;
  int local_32c;
  int local_328;
  int local_324;
  int local_320;
  int local_31c;
  int local_318;
  int local_314;
  int local_310;
  int local_30c;
  int local_308;
  int *local_304;
  int local_300;
  int local_2fc;
  int local_2f8;
  int local_2f4;
  int local_2f0;
  int local_2ec;
  int local_2e8;
  int local_2e4;
  int local_2e0;
  int *local_2dc;
  int local_2d8;
  int local_2d4;
  int local_2d0;
  int local_2cc;
  int local_2c8;
  int local_2c4;
  int local_2c0;
  int local_2bc;
  int *local_2b8;
  undefined1 local_2b4 [128];
  int local_234;
  undefined1 local_230 [128];
  byte local_1b0;
  undefined2 local_1ac;
  int local_1a8;
  undefined4 local_1a4;
  int local_1a0;
  int local_19c;
  undefined2 local_198;
  int local_194;
  int *local_190;
  undefined4 local_18c;
  int local_188;
  int local_184;
  int local_180;
  float local_17c;
  undefined1 local_178 [128];
  int local_f8;
  int local_f4;
  int local_f0;
  int local_ec;
  int local_e8;
  undefined4 local_e4;
  int local_e0;
  int local_dc;
  int local_d8;
  int local_d4;
  int local_d0;
  int local_cc;
  undefined1 local_c8 [128];
  int local_48;
  int local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f548;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00431159(s_>>_Init_Field_Scene__Start_005b78b8);
  if (DAT_005b892c == 2) {
    (**(code **)(*param_1 + 0x3c))(s_UI_FieldScene2_txt_005b78d8);
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e8) = 0;
  }
  else {
    (**(code **)(*param_1 + 0x3c))(s_UI_FieldScene_txt_005b78ec);
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e8) = 0x58;
  }
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0x1e4);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_76c = 0;
    }
    else {
      local_76c = FUN_00401466(0xfffffff7,
                               ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a4338) /
                               _DAT_005a365c,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a433c,0x44034000,
                               0x42580000,0x44ffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0xa184] = local_76c;
    if (param_1[0xa184] != 0) {
      (**(code **)(*(int *)param_1[0xa184] + 0x44))(0x380);
      *(undefined4 *)(param_1[0xa184] + 0x3c) = 0;
      (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0xa184]);
    }
    pvVar2 = operator_new(0xe4c);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_770 = 0;
    }
    else {
      local_770 = FUN_00401d03(0xfffffffe,s_Question_005b7900,0xffffffff,0x41200000,0x40800000,
                               0x43ff0000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    param_1[0xa185] = local_770;
    if (param_1[0xa185] != 0) {
      FUN_0054ac09(param_1[0xa185]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 2;
    if (pvVar2 == (void *)0x0) {
      local_774 = 0;
    }
    else {
      local_774 = FUN_00402f01(0xfffffffe,0x40a00000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer1_005b790c);
    }
    local_8 = 0xffffffff;
    param_1[0xa186] = local_774;
    if (param_1[0xa186] != 0) {
      (**(code **)(*(int *)param_1[0xa186] + 0x44))(0x381);
      if (param_1[10] == 0) {
        local_778 = 0;
      }
      else {
        local_778 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa186] + 0x50))(local_778);
      FUN_0054ac09(param_1[0xa186]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 3;
    if (pvVar2 == (void *)0x0) {
      local_77c = 0;
    }
    else {
      local_77c = FUN_00402f01(0xfffffffe,0x43070000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer2_005b7914);
    }
    local_8 = 0xffffffff;
    param_1[0xa187] = local_77c;
    if (param_1[0xa187] != 0) {
      (**(code **)(*(int *)param_1[0xa187] + 0x44))(0x382);
      if (param_1[10] == 0) {
        local_780 = 0;
      }
      else {
        local_780 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa187] + 0x50))(local_780);
      FUN_0054ac09(param_1[0xa187]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 4;
    if (pvVar2 == (void *)0x0) {
      local_784 = 0;
    }
    else {
      local_784 = FUN_00402f01(0xfffffffe,0x43848000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer3_005b791c);
    }
    local_8 = 0xffffffff;
    param_1[0xa188] = local_784;
    if (param_1[0xa188] != 0) {
      (**(code **)(*(int *)param_1[0xa188] + 0x44))(899);
      if (param_1[10] == 0) {
        local_788 = 0;
      }
      else {
        local_788 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa188] + 0x50))(local_788);
      FUN_0054ac09(param_1[0xa188]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 5;
    if (pvVar2 == (void *)0x0) {
      local_78c = 0;
    }
    else {
      local_78c = FUN_00402f01(0xfffffffe,0x43c58000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer4_005b7924);
    }
    local_8 = 0xffffffff;
    param_1[0xa189] = local_78c;
    if (param_1[0xa189] != 0) {
      (**(code **)(*(int *)param_1[0xa189] + 0x44))(900);
      if (param_1[10] == 0) {
        local_790 = 0;
      }
      else {
        local_790 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa189] + 0x50))(local_790);
      FUN_0054ac09(param_1[0xa189]);
    }
  }
  else {
    pvVar2 = operator_new(0x1e4);
    local_8 = 6;
    if (pvVar2 == (void *)0x0) {
      local_794 = 0;
    }
    else {
      local_794 = FUN_00401466(9,0x43130000,0x43cf8000,0x44034000,0x42580000,0x77777777,4);
    }
    local_8 = 0xffffffff;
    param_1[0xa184] = local_794;
    if (param_1[0xa184] != 0) {
      (**(code **)(*(int *)param_1[0xa184] + 0x44))(0x380);
      *(undefined4 *)(param_1[0xa184] + 0x3c) = 0;
      (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0xa184]);
    }
    pvVar2 = operator_new(0xe4c);
    local_8 = 7;
    if (pvVar2 == (void *)0x0) {
      local_798 = 0;
    }
    else {
      local_798 = FUN_00401d03(0xfffffffe,s_Question_005b792c,0xffffffff,0x41200000,0x40800000,
                               0x43ff0000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    param_1[0xa185] = local_798;
    if (param_1[0xa185] != 0) {
      FUN_0054ac09(param_1[0xa185]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 8;
    if (pvVar2 == (void *)0x0) {
      local_79c = 0;
    }
    else {
      local_79c = FUN_00402f01(0xfffffffe,0x40a00000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer1_005b7938);
    }
    local_8 = 0xffffffff;
    param_1[0xa186] = local_79c;
    if (param_1[0xa186] != 0) {
      (**(code **)(*(int *)param_1[0xa186] + 0x44))(0x381);
      if (param_1[10] == 0) {
        local_7a0 = 0;
      }
      else {
        local_7a0 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa186] + 0x50))(local_7a0);
      FUN_0054ac09(param_1[0xa186]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 9;
    if (pvVar2 == (void *)0x0) {
      local_7a4 = 0;
    }
    else {
      local_7a4 = FUN_00402f01(0xfffffffe,0x43070000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer2_005b7940);
    }
    local_8 = 0xffffffff;
    param_1[0xa187] = local_7a4;
    if (param_1[0xa187] != 0) {
      (**(code **)(*(int *)param_1[0xa187] + 0x44))(0x382);
      if (param_1[10] == 0) {
        local_7a8 = 0;
      }
      else {
        local_7a8 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa187] + 0x50))(local_7a8);
      FUN_0054ac09(param_1[0xa187]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 10;
    if (pvVar2 == (void *)0x0) {
      local_7ac = 0;
    }
    else {
      local_7ac = FUN_00402f01(0xfffffffe,0x43848000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer3_005b7948);
    }
    local_8 = 0xffffffff;
    param_1[0xa188] = local_7ac;
    if (param_1[0xa188] != 0) {
      (**(code **)(*(int *)param_1[0xa188] + 0x44))(899);
      if (param_1[10] == 0) {
        local_7b0 = 0;
      }
      else {
        local_7b0 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa188] + 0x50))(local_7b0);
      FUN_0054ac09(param_1[0xa188]);
    }
    pvVar2 = operator_new(0x208);
    local_8 = 0xb;
    if (pvVar2 == (void *)0x0) {
      local_7b4 = 0;
    }
    else {
      local_7b4 = FUN_00402f01(0xfffffffe,0x43c58000,0x41e00000,0x42fa0000,0x41a00000,0x77777777,1,
                               s_Answer4_005b7950);
    }
    local_8 = 0xffffffff;
    param_1[0xa189] = local_7b4;
    if (param_1[0xa189] != 0) {
      (**(code **)(*(int *)param_1[0xa189] + 0x44))(900);
      if (param_1[10] == 0) {
        local_7b8 = 0;
      }
      else {
        local_7b8 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[0xa189] + 0x50))(local_7b8);
      FUN_0054ac09(param_1[0xa189]);
    }
  }
  if (param_1[0xa184] != 0) {
    (**(code **)(*(int *)param_1[0xa184] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13a);
  param_1[0x9f29] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13b);
  param_1[0x9f25] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13c);
  param_1[0x9f26] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1501);
  param_1[0x9eba] = iVar3;
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13e);
    param_1[0x9f27] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13f);
    param_1[0x9f28] = iVar3;
    (**(code **)(*(int *)param_1[0x9f27] + 0x60))(1);
    *(undefined4 *)(param_1[0x9f27] + 0x1f8) = 1;
    (**(code **)(*(int *)param_1[0x9f28] + 0x60))(1);
    *(undefined4 *)(param_1[0x9f28] + 0x1f8) = 1;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x163f);
    param_1[0x9ebb] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1640);
    param_1[0x9ebc] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1641);
    param_1[0x9ebd] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1642);
    param_1[0x9ebe] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1643);
    param_1[0x9ebf] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1644);
    param_1[0x9ec0] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3223);
    param_1[0x9ec1] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3224);
    param_1[0x9ec2] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3225);
    param_1[0x9ec3] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3226);
    param_1[0x9ec4] = iVar3;
    if (param_1[0x9ebd] != 0) {
      *(undefined4 *)(param_1[0x9ebd] + 0x1e8) = 1;
    }
    if (param_1[0x9ebe] != 0) {
      *(undefined4 *)(param_1[0x9ebe] + 0x1e8) = 1;
    }
    if (param_1[0x9ebf] != 0) {
      *(undefined4 *)(param_1[0x9ebf] + 0x1e8) = 1;
    }
    if (param_1[0x9ec0] != 0) {
      *(undefined4 *)(param_1[0x9ec0] + 0x1e8) = 1;
    }
    if (param_1[0x9ec1] != 0) {
      *(undefined4 *)(param_1[0x9ec1] + 0x1e8) = 1;
    }
    if (param_1[0x9ec2] != 0) {
      *(undefined4 *)(param_1[0x9ec2] + 0x1e8) = 1;
    }
    if (param_1[0x9ec3] != 0) {
      *(undefined4 *)(param_1[0x9ec3] + 0x1e8) = 1;
    }
    if (param_1[0x9ec4] != 0) {
      *(undefined4 *)(param_1[0x9ec4] + 0x1e8) = 1;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x285);
  param_1[0xa1ca] = iVar3;
  FUN_0058f078(local_2b4,&PTR_DAT_005b7958,&DAT_00a38ff8);
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0xe4c);
    local_8 = 0xc;
    if (pvVar2 == (void *)0x0) {
      local_7cc = 0;
    }
    else {
      local_7cc = FUN_00401d03(0xfffffffe,local_2b4,0xffffffbb,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                               _DAT_005a4330,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a4334,0x42700000,
                               0x41900000,1,0x88770000,1,0);
    }
    param_1[0x9e45] = local_7cc;
  }
  else {
    pvVar2 = operator_new(0xe4c);
    local_8 = 0xd;
    if (pvVar2 == (void *)0x0) {
      local_7d0 = 0;
    }
    else {
      local_7d0 = FUN_00401d03(0xfffffffe,local_2b4,0xffffffbb,0x43200000,0x43e90000,0x431e0000,
                               0x41900000,1,0x88770000,1,0);
    }
    param_1[0x9e45] = local_7d0;
  }
  local_8 = 0xffffffff;
  if (param_1[0x9e45] != 0) {
    (**(code **)(*(int *)param_1[0x9e45] + 0x60))(0);
    (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x9e45]);
  }
  FUN_0058f078(local_2b4,&PTR_DAT_005b795c,&DAT_00a3b578);
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0xe4c);
    local_8 = 0xe;
    if (pvVar2 == (void *)0x0) {
      local_7e4 = 0;
    }
    else {
      local_7e4 = FUN_00401d03(0xfffffffe,local_2b4,0xffffffff,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                               _DAT_005a432c,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a4334,0x42a00000,
                               0x41900000,1,0x88000077,1,0);
    }
    param_1[0x9e47] = local_7e4;
  }
  else {
    pvVar2 = operator_new(0xe4c);
    local_8 = 0xf;
    if (pvVar2 == (void *)0x0) {
      local_7e8 = 0;
    }
    else {
      local_7e8 = FUN_00401d03(0xfffffffe,local_2b4,0xffffffff,0x43a00000,0x43e90000,0x431e0000,
                               0x41900000,1,0x88000077,1,0);
    }
    param_1[0x9e47] = local_7e8;
  }
  local_8 = 0xffffffff;
  if (param_1[0x9e47] != 0) {
    (**(code **)(*(int *)param_1[0x9e47] + 0x60))(0);
    (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x9e47]);
  }
  FUN_0058f078(local_2b4,&PTR_DAT_005b7960,&DAT_00a39778);
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0xe4c);
    local_8 = 0x10;
    if (pvVar2 == (void *)0x0) {
      local_7fc = 0;
    }
    else {
      local_7fc = FUN_00401d03(0xfffffffe,local_2b4,0xffffffff,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 +
                               _DAT_005a3970,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a4334,0x42700000,
                               0x41900000,1,0x88007700,1,0);
    }
    param_1[0x9e46] = local_7fc;
  }
  else {
    pvVar2 = operator_new(0xe4c);
    local_8 = 0x11;
    if (pvVar2 == (void *)0x0) {
      local_800 = 0;
    }
    else {
      local_800 = FUN_00401d03(0xfffffffe,local_2b4,0xffffffff,0x43f00000,0x43e90000,0x431e0000,
                               0x41900000,1,0x88007700,1,0);
    }
    param_1[0x9e46] = local_800;
  }
  local_8 = 0xffffffff;
  if (param_1[0x9e46] != 0) {
    (**(code **)(*(int *)param_1[0x9e46] + 0x60))(0);
    (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x9e46]);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x23f);
  param_1[0x9ec9] = iVar3;
  if (param_1[0x9ec9] != 0) {
    (**(code **)(*(int *)param_1[0x9ec9] + 0x60))(0);
  }
  FUN_00447469(param_1[0x9eb9]);
  _memset((void *)(DAT_013b71e8 + 0xc60),0,0x9c);
  _memset((void *)(DAT_013b71e8 + 0xcfc),0,0x54);
  _memset((void *)(DAT_013b71e8 + 0xd50),0,0x54);
  _memset((void *)(DAT_013b71e8 + 0xda4),0,0x54);
  _memset((void *)(DAT_013b71e8 + 0xdf8),0,0x54);
  _memset((void *)(DAT_013b71e8 + 0xe4c),0,0x54);
  _memset((void *)(DAT_013b71e8 + 0xea0),0,0x54);
  *(undefined2 *)(DAT_013b71e8 + 0xd02) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  *(undefined2 *)(DAT_013b71e8 + 0xd00) = 0x3a6;
  *(undefined2 *)(DAT_013b71e8 + 0xd56) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  *(undefined2 *)(DAT_013b71e8 + 0xd54) = 0x3b5;
  *(undefined2 *)(DAT_013b71e8 + 0xdaa) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  *(undefined2 *)(DAT_013b71e8 + 0xda8) = 0x3ba;
  *(undefined2 *)(DAT_013b71e8 + 0xdfe) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  *(undefined2 *)(DAT_013b71e8 + 0xdfc) = 0x3c0;
  *(undefined2 *)(DAT_013b71e8 + 0xe52) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  *(undefined2 *)(DAT_013b71e8 + 0xe50) = 0x2c3;
  *(undefined2 *)(DAT_013b71e8 + 0xea6) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  *(undefined2 *)(DAT_013b71e8 + 0xea4) = 0x2c3;
  for (local_31c = 0; local_31c < 0xf; local_31c = local_31c + 1) {
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xce4) = 0xff;
  }
  for (local_31c = 0; local_31c < 8; local_31c = local_31c + 1) {
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xd48) = 0xff;
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xd9c) = 0xff;
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xdf0) = 0xff;
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xe44) = 0xff;
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xe98) = 0xff;
    *(undefined1 *)(DAT_013b71e8 + local_31c + 0xeec) = 0xff;
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x303);
  param_1[0x9ec7] = iVar3;
  if (param_1[0x9ec7] != 0) {
    (**(code **)(*(int *)param_1[0x9ec7] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x270);
  param_1[0x9ec8] = iVar3;
  if (param_1[0x9ec8] != 0) {
    *(undefined4 *)(param_1[0x9ec8] + 0x3c) = 0;
  }
  local_35c = (**(code **)(*(int *)param_1[10] + 0x48))(0x271);
  if (local_35c != 0) {
    *(undefined4 *)(local_35c + 0x3c) = 0;
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1403);
  param_1[0x9ecb] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x23e);
  param_1[0x9ecc] = iVar3;
  if (param_1[0x9ecc] != 0) {
    (**(code **)(*(int *)param_1[0x9ecc] + 0x60))(0);
  }
  if (param_1[0x9ecb] != 0) {
    (**(code **)(*(int *)param_1[0x9ecb] + 0x60))(1);
  }
  _memset(&DAT_0092eaf8,0,0x1a0);
  *(undefined2 *)(param_1 + 0x9bbb) = 0;
  *(undefined2 *)((int)param_1 + 0x26eee) = 0;
  _memset(param_1 + 0x9bbc,0,0x1e0);
  _memset(param_1 + 0x9c34,0,0x50);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9ec8] + 0x74))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504),(float)*(uint *)(DAT_005ccf9c + 0x2a508));
  }
  else {
    local_3b4 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x104);
    local_3b0 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x105);
    local_3bc = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x106);
    local_3b8 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x107);
    if (local_3b4 != (int *)0x0) {
      iVar3 = (**(code **)(*local_3b4 + 0x84))();
      *(undefined4 *)(iVar3 + 0x18) = 0xbfc90fdb;
      local_3b4[0xf] = 0;
    }
    if (local_3b0 != (int *)0x0) {
      iVar3 = (**(code **)(*local_3b0 + 0x84))();
      *(undefined4 *)(iVar3 + 0x18) = 0xbfc90fdb;
      local_3b0[0xf] = 0;
    }
    if (local_3bc != (int *)0x0) {
      iVar3 = (**(code **)(*local_3bc + 0x84))();
      *(undefined4 *)(iVar3 + 0x18) = 0xbfc90fdb;
      local_3bc[0xf] = 0;
    }
    if (local_3b8 != (int *)0x0) {
      iVar3 = (**(code **)(*local_3b8 + 0x84))();
      *(undefined4 *)(iVar3 + 0x18) = 0xbfc90fdb;
      local_3b8[0xf] = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x561);
  param_1[0x9f20] = iVar3;
  if (param_1[0x9f20] != 0) {
    (**(code **)(*(int *)param_1[0x9f20] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x103);
  param_1[0xa1e9] = iVar3;
  if (param_1[0xa1e9] != 0) {
    (**(code **)(*(int *)param_1[0xa1e9] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x10a);
  param_1[0xa1ea] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x10b);
  param_1[0xa1eb] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x10c);
  param_1[0xa1ec] = iVar3;
  *(int *)(param_1[0x9f20] + 0x1d0) = param_1[0xa1e9];
  pvVar2 = operator_new(0xe4c);
  local_8 = 0x12;
  if (pvVar2 == (void *)0x0) {
    local_81c = 0;
  }
  else {
    local_81c = FUN_00401d03(0xfffffffe,s_00___00_005b7964,0xff00ff00,
                             ((float)*(uint *)(DAT_005ccf9c + 0x2a504) / DAT_005b490c) /
                             _DAT_005a365c - _DAT_005a3a98,0x41f00000,0x43480000,0x41800000,0,
                             0x77777777,1,0);
  }
  local_8 = 0xffffffff;
  param_1[0x9e35] = local_81c;
  if (param_1[0x9e35] != 0) {
    *(undefined4 *)(param_1[0x9e35] + 0x74) = 0x40000000;
    (**(code **)(*(int *)param_1[0x9e35] + 0x60))(0);
  }
  FUN_0058f078(local_178,s__s_100_005b796c,&DAT_00a3b478);
  pvVar2 = operator_new(0xe4c);
  local_8 = 0x13;
  if (pvVar2 == (void *)0x0) {
    local_828 = 0;
  }
  else {
    local_828 = FUN_00401d03(0xfffffffe,local_178,0xffffaa00,
                             (float)*(uint *)(DAT_005ccf9c + 0x2a504) / DAT_005b490c - _DAT_005a4328
                             ,0x41f00000,0x42c80000,0x41800000,0,0x77777777,1,0);
  }
  local_8 = 0xffffffff;
  param_1[0x9e37] = local_828;
  if (param_1[0x9e37] != 0) {
    *(undefined4 *)(param_1[0x9e37] + 0x74) = 0x40000000;
    (**(code **)(*(int *)param_1[0x9e37] + 0x60))(0);
  }
  pvVar2 = operator_new(0xe4c);
  local_8 = 0x14;
  if (pvVar2 == (void *)0x0) {
    local_834 = 0;
  }
  else {
    local_834 = FUN_00401d03(0xfffffffe,&DAT_005d0420,0xddffff33,
                             (float)*(uint *)(DAT_005ccf9c + 0x2a504) / DAT_005b490c - _DAT_005a4324
                             ,_DAT_005a4104 * DAT_005b4910,0x42c80000,0x41800000,0,0x77777777,1,0);
  }
  local_8 = 0xffffffff;
  param_1[0xa393] = local_834;
  if (param_1[0xa393] != 0) {
    *(undefined4 *)(param_1[0xa393] + 0x74) = 0x3f800000;
    (**(code **)(*(int *)param_1[0xa393] + 0x60))(1);
  }
  if (param_1[0x9e35] != 0) {
    (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x9e35]);
  }
  if (param_1[0x9e37] != 0) {
    (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x9e37]);
  }
  if (param_1[0xa393] != 0) {
    (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0xa393]);
  }
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9e35] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 - _DAT_005a3704,
               _DAT_005a40f0 * DAT_005b4910 + _DAT_005a4320);
    (**(code **)(*(int *)param_1[0x9e37] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a431c,_DAT_005a40fc * DAT_005b4910
              );
    if (*(uint *)(DAT_005ccf9c + 0x2a504) < 800) {
      (**(code **)(*(int *)param_1[0xa393] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) + _DAT_005a4318,
                 _DAT_005a4104 * DAT_005b4910);
    }
    else {
      (**(code **)(*(int *)param_1[0xa393] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a4324,
                 _DAT_005a4104 * DAT_005b4910);
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x172);
  param_1[0x9e48] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x411);
  param_1[0x9e49] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1908);
  param_1[0x9e4a] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x421);
  param_1[0x9e4b] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x423);
  param_1[0x9e4d] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x42a);
  param_1[0x9e4e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x404);
  param_1[0x9e4f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x444);
  param_1[0x9e50] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x404);
  param_1[0x9e51] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x446);
  param_1[0x9e52] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x407);
  param_1[0x9e53] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x408);
  param_1[0x9e54] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x425);
  param_1[0x9e55] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x426);
  param_1[0x9e56] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x428);
  param_1[0x9e57] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x442);
  param_1[0x9e58] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x484);
  param_1[0x9e59] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x23c);
  param_1[0x9e5a] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x412);
  param_1[0x9e5b] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x413);
  param_1[0x9e5c] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x414);
  param_1[0x9e5d] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x430);
  param_1[0x9e5e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x433);
  param_1[0x9e5f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x436);
  param_1[0x9e60] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x439);
  param_1[0x9e61] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x472);
  param_1[0x9e62] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x475);
  param_1[0x9e63] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x478);
  param_1[0x9e64] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x481);
  param_1[0x9e65] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x560);
  param_1[0x9e6a] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x493);
  param_1[0x9e6e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x494);
  param_1[0x9e6b] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x495);
  param_1[0x9e6c] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x496);
  param_1[0x9e6d] = iVar3;
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9e54] + 0x60))(0);
  }
  for (local_31c = 0; local_31c < 0x18; local_31c = local_31c + 1) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_31c + 0x223);
    param_1[local_31c + 0x9e6f] = iVar3;
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x783);
  param_1[0x9e87] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x784);
  param_1[0x9e88] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x785);
  param_1[0x9e89] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x485);
  param_1[0x9e8a] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x448);
  param_1[0x9e9b] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x405);
  param_1[0x9e9c] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x450);
  param_1[0x9e9d] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x452);
  param_1[0x9e9e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x454);
  param_1[0x9e9f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x409);
  param_1[0x9ea0] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x410);
  param_1[0x9ea1] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x456);
  param_1[0x9ea2] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x458);
  param_1[0x9ea3] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x406);
  param_1[0x9ea4] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x460);
  param_1[0x9ea5] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x461);
  param_1[0x9ea6] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x462);
  param_1[0x9ea7] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x463);
  param_1[0x9ea8] = iVar3;
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1654);
    param_1[0x9e8e] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1655);
    param_1[0x9e8f] = iVar3;
    FUN_00401304();
    FUN_004013c2();
    *(float *)(param_1[0x9e8e] + 0x4c) =
         ((float)(*(uint *)(DAT_005ccf9c + 0x2a504) >> 1) -
         *(float *)(param_1[0x9e8e] + 0x54) / _DAT_005a365c) + _DAT_005a3834;
    local_3c0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3150);
    FUN_00401304();
    FUN_004013c2();
    *(float *)(local_3c0 + 0x4c) = *(float *)(param_1[0x9e8e] + 0x4c) - *(float *)(local_3c0 + 0x54)
    ;
    *(float *)(param_1[0x9eba] + 0x50) =
         (*(float *)(local_3c0 + 0x50) - *(float *)(param_1[0x9eba] + 0x58)) - _DAT_005a3794;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3228);
    param_1[0x9ec5] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x165a);
    param_1[0x9e90] = iVar3;
    if (param_1[0x9e90] != 0) {
      (**(code **)(*(int *)param_1[0x9e90] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9e90] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) - *(float *)(param_1[0x9e90] + 0x58));
      (**(code **)(**(int **)(param_1[0x9e90] + 0x1f0) + 0x70))
                (*(float *)(param_1[0x9e8e] + 0x54) * _DAT_005a34a0 - _DAT_005a3a9c,
                 *(float *)(*(int *)(param_1[0x9e90] + 0x1f0) + 0x50) - _DAT_005a3970);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x165b);
    param_1[0x9e91] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x165c);
    param_1[0x9e92] = iVar3;
    if (param_1[0x9e91] != 0) {
      FUN_00401304();
      FUN_00401367();
      FUN_004013ad();
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1661);
      param_1[0x9e93] = iVar3;
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1662);
    param_1[0x9e99] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1663);
    param_1[0x9e9a] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1664);
    param_1[0x9e94] = iVar3;
    if (param_1[0x9e94] != 0) {
      (**(code **)(*(int *)param_1[0x9e94] + 0x70))
                (0,(float)*(uint *)(DAT_005ccf9c + 0x2a508) - *(float *)(param_1[0x9e94] + 0x58));
      (**(code **)(**(int **)(param_1[0x9e94] + 0x1f0) + 0x70))
                (*(float *)(*(int *)(param_1[0x9e94] + 0x1f0) + 0x4c) + _DAT_005a4104,
                 *(float *)(param_1[0x9e91] + 0x58) * _DAT_005a34a0 - _DAT_005a4104);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x166d);
    param_1[0x9e95] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1665);
    param_1[0x9e96] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1666);
    param_1[0x9e97] = iVar3;
    if (param_1[0x9e96] != 0) {
      (**(code **)(*(int *)param_1[0x9e96] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - *(float *)(param_1[0x9e96] + 0x54),
                 ((float)*(uint *)(DAT_005ccf9c + 0x2a508) - *(float *)(param_1[0x9e8e] + 0x58)) -
                 _DAT_005a40fc);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1667);
    param_1[0x9e98] = iVar3;
    if (param_1[0x9e98] != 0) {
      (**(code **)(*(int *)param_1[0x9e98] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a3670,
                 *(undefined4 *)(param_1[0x9e96] + 0x50));
      (**(code **)(**(int **)(param_1[0x9e98] + 0x1f0) + 0x70))
                (*(float *)(*(int *)(param_1[0x9e98] + 0x1f0) + 0x4c) - _DAT_005a3c68,
                 *(float *)(param_1[0x9e96] + 0x58) * _DAT_005a34a0 - _DAT_005a4104);
    }
    (**(code **)(*param_1 + 0x58))(0x1667,0);
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x166b);
    param_1[0x9eca] = iVar3;
    if (param_1[0x9eca] != 0) {
      local_3c4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3150);
      *(undefined4 *)(param_1[0x9eca] + 0x50) = *(undefined4 *)(local_3c4 + 0x50);
      *(undefined4 *)(param_1[0x9eca] + 0x4c) = *(undefined4 *)(local_3c4 + 0x4c);
      if (DAT_013b722c == 0) {
        (**(code **)(*(int *)param_1[0x9eca] + 0x60))(0);
      }
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1670);
    param_1[0x9ec6] = iVar3;
    if (param_1[0x9ec6] != 0) {
      FUN_00401304();
      FUN_0040137c();
      FUN_004013c2();
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x124);
    param_1[0xa1ff] = iVar3;
    if (param_1[0xa1ff] != 0) {
      FUN_00401304();
      FUN_0040137c();
      *(float *)(param_1[0xa1ff] + 0x50) =
           *(float *)(param_1[0x9ec6] + 0x50) - *(float *)(param_1[0xa1ff] + 0x58);
      *(float *)(param_1[0xa1ff] + 0x4c) = *(float *)(param_1[0xa1ff] + 0x4c) - _DAT_005a365c;
      (**(code **)(*(int *)param_1[0xa1ff] + 0x60))(0);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1671);
    param_1[0xa200] = iVar3;
    if (param_1[0xa200] != 0) {
      FUN_004013f3();
      FUN_004013c2();
      *(float *)(param_1[0xa200] + 0x4c) =
           *(float *)(param_1[0x9e8e] + 0x4c) + *(float *)(param_1[0x9e8e] + 0x54);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1673);
    param_1[0xa201] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1674);
    param_1[0xa202] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1678);
    param_1[0x9e4c] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1690);
    param_1[0x9e66] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1691);
    param_1[0x9e67] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1692);
    param_1[0x9e68] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1693);
    param_1[0x9e69] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x169c);
    param_1[0x9e8b] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x169d);
    param_1[0x9e8c] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x169e);
    param_1[0x9e8d] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17a2);
    param_1[0xa179] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17a3);
    param_1[0xa17a] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17a4);
    param_1[0xa17b] = iVar3;
  }
  for (local_31c = 0; local_31c < 0x10; local_31c = local_31c + 1) {
    if (DAT_005b892c == 2) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_31c + 0x530);
      param_1[local_31c + 0xa1a4] = iVar3;
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_31c + 0x540);
      param_1[local_31c + 0xa194] = iVar3;
    }
    else {
      if (local_31c < 4) {
        iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_31c + 0x570);
        param_1[local_31c + 0xa18a] = iVar3;
        iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_31c + 0x580);
        param_1[local_31c + 0xa18e] = iVar3;
      }
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x53f - local_31c);
      param_1[local_31c + 0xa1a4] = iVar3;
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x54f - local_31c);
      param_1[local_31c + 0xa194] = iVar3;
    }
  }
  if ((DAT_005b892c == 2) && (param_1[0xa200] != 0)) {
    if (param_1[0x9e45] != 0) {
      (**(code **)(*(int *)param_1[0x9e45] + 0x70))
                (*(float *)(param_1[0xa200] + 0x4c) + _DAT_005a4310,
                 *(float *)(param_1[0xa200] + 0x50) - _DAT_005a4314);
    }
    if (param_1[0x9e47] != 0) {
      (**(code **)(*(int *)param_1[0x9e47] + 0x70))
                (*(float *)(param_1[0x9e45] + 0x4c) + *(float *)(param_1[0x9e45] + 0x54),
                 *(undefined4 *)(param_1[0x9e45] + 0x50));
    }
    if (param_1[0x9e46] != 0) {
      (**(code **)(*(int *)param_1[0x9e46] + 0x70))
                (*(float *)(param_1[0x9e47] + 0x4c) + *(float *)(param_1[0x9e47] + 0x54),
                 *(undefined4 *)(param_1[0x9e45] + 0x50));
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x72b);
  param_1[0x9ea9] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x32c);
  param_1[0x9eaa] = iVar3;
  local_304 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x26b);
  local_2dc = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x25b);
  *(undefined1 *)((int)local_304 + 0xcdd) = 1;
  *(undefined1 *)((int)local_2dc + 0xcdd) = 1;
  (**(code **)(*local_304 + 0x80))(s_0_005b7974,0);
  (**(code **)(*local_2dc + 0x80))(s_0_005b7980,0);
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x101);
  param_1[0x9ecd] = iVar3;
  local_3ac = (**(code **)(*(int *)param_1[10] + 0x48))(0x111);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9ecd] + 0x70))
              (*(float *)(param_1[0x9ecd] + 0x54) * _DAT_005a34a0 +
               (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 + _DAT_005a3670,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9ecd] + 0x58) * _DAT_005a34a0);
  }
  else {
    local_390 = (**(code **)(*(int *)param_1[10] + 0x48))(0x112);
    local_e8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x113);
    local_38 = (**(code **)(*(int *)param_1[10] + 0x48))(0x114);
  }
  if (param_1[0x9ecd] != 0) {
    (**(code **)(*(int *)param_1[0x9ecd] + 0x60))(0);
    *(undefined4 *)(local_3ac + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_390 + 0x3c) = 0;
      *(undefined4 *)(local_e8 + 0x3c) = 0;
      *(undefined4 *)(local_38 + 0x3c) = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x286);
  param_1[0x9ece] = iVar3;
  if (param_1[0x9ece] != 0) {
    (**(code **)(*(int *)param_1[0x9ece] + 0x60))(0);
    for (local_3c8 = 0; local_3c8 < 0xc; local_3c8 = local_3c8 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3c8 + 0x28d);
      param_1[local_3c8 + 0x9ecf] = iVar3;
      if (param_1[local_3c8 + 0x9ecf] != 0) {
        *(undefined4 *)(param_1[local_3c8 + 0x9ecf] + 0x400) = 9;
      }
    }
    if (DAT_005b892c == 2) {
      (**(code **)(*(int *)param_1[0x9ece] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9ece] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9ece] + 0x58) * _DAT_005a34a0);
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x550);
  param_1[0x9edb] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17de);
  param_1[0x9ee4] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1801);
  param_1[0x9eed] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1920);
  param_1[0x9efa] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1951);
  param_1[0x9f04] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1970);
  param_1[0x9f0e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1828);
  param_1[0x9ef6] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1927);
  param_1[0x9f03] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1977);
  param_1[0x9f17] = iVar3;
  if (DAT_005b892c == 2) {
    if (param_1[0x9edb] != 0) {
      (**(code **)(*(int *)param_1[0x9edb] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9edb] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9edb] + 0x58) * _DAT_005a34a0);
    }
    if (param_1[0x9ee4] != 0) {
      (**(code **)(*(int *)param_1[0x9ee4] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9ee4] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9ee4] + 0x58) * _DAT_005a34a0);
    }
    if (param_1[0x9eed] != 0) {
      (**(code **)(*(int *)param_1[0x9eed] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9eed] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9eed] + 0x58) * _DAT_005a34a0);
    }
    if (param_1[0x9efa] != 0) {
      (**(code **)(*(int *)param_1[0x9efa] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9efa] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9efa] + 0x58) * _DAT_005a34a0);
    }
    if (param_1[0x9f04] != 0) {
      (**(code **)(*(int *)param_1[0x9f04] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9f04] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9f04] + 0x58) * _DAT_005a34a0);
    }
    if (param_1[0x9f0e] != 0) {
      (**(code **)(*(int *)param_1[0x9f0e] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9f0e] + 0x54) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9f0e] + 0x58) * _DAT_005a34a0);
    }
  }
  if (param_1[0x9ef6] != 0) {
    FUN_00442dd0(param_1[0x9ef6],s_UI_mix3desc_txt_005b798c);
  }
  if (param_1[0x9f03] != 0) {
    FUN_00442dd0(param_1[0x9f03],s_UI_mix4desc_txt_005b799c);
  }
  if (param_1[0x9f17] != 0) {
    FUN_00442dd0(param_1[0x9f17],s_UI_mix6desc_txt_005b79ac);
  }
  if (param_1[0x9edb] != 0) {
    (**(code **)(*(int *)param_1[0x9edb] + 0x60))(0);
    for (local_3cc = 0; local_3cc < 8; local_3cc = local_3cc + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3cc + 0x557);
      param_1[local_3cc + 0x9edc] = iVar3;
      if (param_1[local_3cc + 0x9edc] != 0) {
        *(undefined4 *)(param_1[local_3cc + 0x9edc] + 0x400) = 0xc;
      }
    }
  }
  if (param_1[0x9ee4] != 0) {
    (**(code **)(*(int *)param_1[0x9ee4] + 0x60))(0);
    for (local_3d0 = 0; local_3d0 < 8; local_3d0 = local_3d0 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3d0 + 0x17e0);
      param_1[local_3d0 + 0x9ee5] = iVar3;
      if (param_1[local_3d0 + 0x9ee5] != 0) {
        *(undefined4 *)(param_1[local_3d0 + 0x9ee5] + 0x400) = 0xe;
      }
    }
    for (local_3d0 = 0; local_3d0 < 4; local_3d0 = local_3d0 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3d0 + 0x17f1);
      param_1[local_3d0 + 0x9f18] = iVar3;
      if (param_1[local_3d0 + 0x9f18] != 0) {
        *(undefined4 *)(param_1[local_3d0 + 0x9f18] + 0x400) = 0xe;
      }
    }
  }
  if (param_1[0x9eed] != 0) {
    (**(code **)(*(int *)param_1[0x9eed] + 0x60))(0);
    for (local_3d4 = 0; local_3d4 < 6; local_3d4 = local_3d4 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3d4 + 0x1803);
      param_1[local_3d4 + 0x9eee] = iVar3;
      if (param_1[local_3d4 + 0x9eee] != 0) {
        *(undefined4 *)(param_1[local_3d4 + 0x9eee] + 0x400) = 0x10;
      }
    }
  }
  if (param_1[0x9efa] != 0) {
    (**(code **)(*(int *)param_1[0x9efa] + 0x60))(0);
    for (local_3d8 = 0; local_3d8 < 3; local_3d8 = local_3d8 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3d8 + 0x1924);
      param_1[local_3d8 + 0x9efb] = iVar3;
      if (param_1[local_3d8 + 0x9efb] != 0) {
        *(undefined4 *)(param_1[local_3d8 + 0x9efb] + 0x400) = 0x12;
      }
    }
  }
  if (param_1[0x9f04] != 0) {
    (**(code **)(*(int *)param_1[0x9f04] + 0x60))(0);
    for (local_3dc = 0; local_3dc < 7; local_3dc = local_3dc + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3dc + 0x1953);
      param_1[local_3dc + 0x9f05] = iVar3;
      if (param_1[local_3dc + 0x9f05] != 0) {
        *(undefined4 *)(param_1[local_3dc + 0x9f05] + 0x400) = 0x14;
      }
    }
  }
  if (param_1[0x9f0e] != 0) {
    (**(code **)(*(int *)param_1[0x9f0e] + 0x60))(0);
    for (local_3e0 = 0; local_3e0 < 3; local_3e0 = local_3e0 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3e0 + 0x1974);
      param_1[local_3e0 + 0x9f0f] = iVar3;
      if (param_1[local_3e0 + 0x9f0f] != 0) {
        *(undefined4 *)(param_1[local_3e0 + 0x9f0f] + 0x400) = 0x16;
      }
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1829);
  param_1[0x9ef7] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1839);
  param_1[0x9ef8] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1840);
  param_1[0xa1df] = iVar3;
  *(undefined4 *)(param_1[0xa1df] + 0x1f8) = 0;
  *(undefined4 *)(param_1[0xa1df] + 0x400) = 1;
  if ((DAT_005b892c == 2) && (param_1[0x9ef7] != 0)) {
    (**(code **)(*(int *)param_1[0x9ef7] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9ef7] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9ef7] + 0x58) * _DAT_005a34a0);
  }
  if (param_1[0x9ef7] != 0) {
    (**(code **)(*(int *)param_1[0x9ef7] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1900);
  param_1[0x9ef9] = iVar3;
  if ((DAT_005b892c == 2) && (param_1[0x9ef9] != 0)) {
    (**(code **)(*(int *)param_1[0x9ef9] + 0x70))
              (((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9ef9] + 0x54) * _DAT_005a34a0) + _DAT_005a3670,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9ef9] + 0x58) * _DAT_005a34a0);
  }
  if (param_1[0x9ef9] != 0) {
    (**(code **)(*(int *)param_1[0x9ef9] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x102);
  param_1[0x16] = iVar3;
  *(undefined4 *)(param_1[0x16] + 0x3c) = 0;
  (**(code **)(*(int *)param_1[0x16] + 0x60))(0);
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x304);
  param_1[0x9e36] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x305);
  param_1[0x9e38] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x306);
  param_1[0x9e39] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x307);
  param_1[0x9e3a] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x308);
  param_1[0x9e3b] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x309);
  param_1[0x9e3c] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x310);
  param_1[0x9e3d] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x31a);
  param_1[0x9e3e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x31b);
  param_1[0x9e3f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x31c);
  param_1[0x9e40] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x31d);
  param_1[0x9e41] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x31e);
  param_1[0x9e42] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(799);
  param_1[0x9e43] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x278);
  param_1[0x9f1c] = iVar3;
  if (param_1[0x9f1c] != 0) {
    (**(code **)(*(int *)param_1[0x9f1c] + 0x60))(0);
    *(undefined4 *)(param_1[0x9f1c] + 0x60) = 1;
    *(int *)(param_1[10] + 0x138) = param_1[0x9f1c];
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x280);
  param_1[0x9f1d] = iVar3;
  if (param_1[0x9f1d] != 0) {
    (**(code **)(*(int *)param_1[0x9f1d] + 0x60))(0);
    *(undefined4 *)(param_1[0x9f1d] + 0x60) = 1;
    *(int *)(param_1[10] + 0x13c) = param_1[0x9f1d];
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x272);
  param_1[0x9f1e] = iVar3;
  if (param_1[0x9f1e] != 0) {
    (**(code **)(*(int *)param_1[0x9f1e] + 0x60))(0);
    *(undefined4 *)(param_1[0x9f1e] + 0x60) = 1;
    *(int *)(param_1[10] + 0x140) = param_1[0x9f1e];
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(299);
  param_1[0xa1fd] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x12a);
  param_1[0xa1fe] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x281);
  param_1[0xa1ed] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x282);
  param_1[0xa1ee] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x283);
  param_1[0xa1ef] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x26c);
  param_1[0xa1f0] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x27f);
  param_1[0xa1f1] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x26d);
  param_1[0xa1f2] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x26e);
  param_1[0xa1f3] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x26f);
  param_1[0xa1f4] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x330);
  param_1[0xa1f5] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x331);
  param_1[0xa1f6] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x35e);
  param_1[0xa1f7] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x35f);
  param_1[0xa1f8] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x390);
  param_1[0xa1f9] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x391);
  param_1[0xa1fa] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x392);
  param_1[0xa1fb] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x393);
  param_1[0xa1fc] = iVar3;
  local_340 = (**(code **)(*(int *)param_1[10] + 0x48))(0x279);
  local_358 = (**(code **)(*(int *)param_1[10] + 0x48))(0x27a);
  local_e4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x27b);
  local_18c = (**(code **)(*(int *)param_1[10] + 0x48))(0x27c);
  if (((DAT_005b892c != 2) && (local_340 != 0)) && (DAT_005b490c != 1.0)) {
    FUN_00481c77(local_340);
    FUN_00481c77(local_358);
    FUN_00481c77(local_e4);
    FUN_00481c77(local_18c);
    FUN_00481c77(param_1[0xa1ed]);
    FUN_00481c77(param_1[0xa1ee]);
    FUN_00481c77(param_1[0xa1ef]);
    FUN_00481c77(param_1[0xa1f0]);
    FUN_00481c77(param_1[0xa1f1]);
    FUN_00481c77(param_1[0xa1f2]);
    FUN_00481c77(param_1[0xa1f3]);
    FUN_00481c77(param_1[0xa1f4]);
    FUN_00481c77(param_1[0xa1f5]);
    FUN_00481c77(param_1[0xa1f6]);
    FUN_00481c77(param_1[0xa1f7]);
    FUN_00481c77(param_1[0xa1f8]);
    FUN_00481c77(param_1[0xa1f9]);
    FUN_00481c77(param_1[0xa1fa]);
    FUN_00481c77(param_1[0xa1fb]);
    FUN_00481c77(param_1[0xa1fc]);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x201);
  param_1[0x9f1f] = iVar3;
  local_2f4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x211);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9f1f] + 0x70))
              (((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1f] + 0x54) * _DAT_005a430c) - _DAT_005a3670,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1f] + 0x58) * _DAT_005a34a0);
    (**(code **)(*(int *)param_1[0x9f1c] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1c] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1c] + 0x58) * _DAT_005a34a0);
    (**(code **)(*(int *)param_1[0x9f1d] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1d] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1d] + 0x58) * _DAT_005a34a0);
    (**(code **)(*(int *)param_1[0x9f1e] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1e] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f1e] + 0x58) * _DAT_005a34a0);
  }
  else {
    local_2d8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x212);
    local_e0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x213);
    local_34 = (**(code **)(*(int *)param_1[10] + 0x48))(0x214);
  }
  if (param_1[0x9f1f] != 0) {
    (**(code **)(*(int *)param_1[0x9f1f] + 0x60))(0);
    *(undefined4 *)(local_2f4 + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_2d8 + 0x3c) = 0;
      *(undefined4 *)(local_e0 + 0x3c) = 0;
      *(undefined4 *)(local_34 + 0x3c) = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x701);
  param_1[0xa175] = iVar3;
  local_30 = (**(code **)(*(int *)param_1[10] + 0x48))(0x702);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0xa175] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0xa175] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0xa175] + 0x58) * _DAT_005a34a0);
  }
  else {
    local_3a8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x703);
    local_38c = (**(code **)(*(int *)param_1[10] + 0x48))(0x704);
    local_370 = (**(code **)(*(int *)param_1[10] + 0x48))(0x705);
    local_3a4 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x708);
    local_f8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x709);
    local_1a0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x710);
    iVar3 = (**(code **)(*local_3a4 + 0x84))();
    *(undefined4 *)(iVar3 + 0x18) = 0xbfc90fdb;
  }
  if (param_1[0xa175] != 0) {
    (**(code **)(*(int *)param_1[0xa175] + 0x60))(0);
    *(undefined4 *)(local_30 + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_3a8 + 0x3c) = 0;
      *(undefined4 *)(local_38c + 0x3c) = 0;
      *(undefined4 *)(local_370 + 0x3c) = 0;
      *(undefined4 *)(local_f8 + 0x3c) = 0;
      *(undefined4 *)(local_1a0 + 0x3c) = 0;
      local_3a4[0xf] = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x721);
  param_1[0xa176] = iVar3;
  local_188 = (**(code **)(*(int *)param_1[10] + 0x48))(0x722);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0xa176] + 0x70))
              ((*(float *)(param_1[0x9ecd] + 0x54) * _DAT_005a34a0 +
                (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 + _DAT_005a3670) -
               *(float *)(param_1[0xa176] + 0x54),
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0xa176] + 0x58) * _DAT_005a34a0);
  }
  else {
    local_2c = (**(code **)(*(int *)param_1[10] + 0x48))(0x723);
    local_dc = (**(code **)(*(int *)param_1[10] + 0x48))(0x724);
    local_2d4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x725);
    local_2d0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x728);
    local_354 = (**(code **)(*(int *)param_1[10] + 0x48))(0x729);
    local_318 = (**(code **)(*(int *)param_1[10] + 0x48))(0x727);
  }
  if (param_1[0xa176] != 0) {
    (**(code **)(*(int *)param_1[0xa176] + 0x60))(0);
    *(undefined4 *)(local_188 + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_2c + 0x3c) = 0;
      *(undefined4 *)(local_dc + 0x3c) = 0;
      *(undefined4 *)(local_2d4 + 0x3c) = 0;
      *(undefined4 *)(local_2d0 + 0x3c) = 0;
      *(undefined4 *)(local_354 + 0x3c) = 0;
      *(undefined4 *)(local_318 + 0x3c) = 0;
    }
  }
  local_33c = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x240);
  local_28 = (**(code **)(*(int *)param_1[10] + 0x48))(0x241);
  if (DAT_005b892c == 2) {
    (**(code **)(*local_33c + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               (float)local_33c[0x15] * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               (float)local_33c[0x16] * _DAT_005a34a0);
  }
  else {
    local_d8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x242);
    local_f4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x243);
    local_184 = (**(code **)(*(int *)param_1[10] + 0x48))(0x244);
    local_314 = (**(code **)(*(int *)param_1[10] + 0x48))(0x245);
    local_300 = (**(code **)(*(int *)param_1[10] + 0x48))(0x246);
  }
  param_1[0xa17c] = (int)local_33c;
  if (local_33c != (int *)0x0) {
    (**(code **)(*local_33c + 0x60))(0);
    *(undefined4 *)(local_28 + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_d8 + 0x3c) = 0;
      *(undefined4 *)(local_f4 + 0x3c) = 0;
      *(undefined4 *)(local_184 + 0x3c) = 0;
      *(undefined4 *)(local_314 + 0x3c) = 0;
      *(undefined4 *)(local_300 + 0x3c) = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x800);
  param_1[0x9e2d] = iVar3;
  if (param_1[0x9e2d] != 0) {
    if (DAT_005b892c == 2) {
      (**(code **)(*(int *)param_1[0x9e2d] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 (*(float *)(param_1[0x9e2d] + 0x54) + _DAT_005a3c68) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9e2d] + 0x58) * _DAT_005a34a0);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x803);
    param_1[0x9e34] = iVar3;
    for (local_3e4 = 0; local_3e4 < 6; local_3e4 = local_3e4 + 1) {
      iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(local_3e4 + 0x805);
      param_1[local_3e4 + 0x9e2e] = iVar3;
    }
    *(undefined4 *)(param_1[0x9e2d] + 0x1dc) = 1;
    *(undefined4 *)(param_1[0x9e2d] + 0x3c) = 1;
    (**(code **)(*(int *)param_1[0x9e2d] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x761);
  param_1[0xa177] = iVar3;
  local_234 = (**(code **)(*(int *)param_1[10] + 0x48))(0x762);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0xa177] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0xa177] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0xa177] + 0x58) * _DAT_005a34a0);
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17f0);
    param_1[0xa178] = iVar3;
    FUN_00442dd0(param_1[0xa178],s_UI_descskillm_txt_005b79bc);
  }
  else {
    local_f0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x763);
    local_180 = (**(code **)(*(int *)param_1[10] + 0x48))(0x764);
    local_24 = (**(code **)(*(int *)param_1[10] + 0x48))(0x765);
    local_19c = (**(code **)(*(int *)param_1[10] + 0x48))(0x768);
    local_388 = (**(code **)(*(int *)param_1[10] + 0x48))(0x769);
    local_2cc = (**(code **)(*(int *)param_1[10] + 0x48))(0x767);
  }
  if (param_1[0xa177] != 0) {
    (**(code **)(*(int *)param_1[0xa177] + 0x60))(0);
    *(undefined4 *)(local_234 + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_f0 + 0x3c) = 0;
      *(undefined4 *)(local_180 + 0x3c) = 0;
      *(undefined4 *)(local_24 + 0x3c) = 0;
      *(undefined4 *)(local_19c + 0x3c) = 0;
      *(undefined4 *)(local_388 + 0x3c) = 0;
      *(undefined4 *)(local_2cc + 0x3c) = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x771);
  param_1[0x9f21] = iVar3;
  local_330 = (**(code **)(*(int *)param_1[10] + 0x48))(0x778);
  local_32c = (**(code **)(*(int *)param_1[10] + 0x48))(0x776);
  local_d4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x777);
  local_20 = (**(code **)(*(int *)param_1[10] + 0x48))(0x772);
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9f21] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f21] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f21] + 0x58) * _DAT_005a34a0);
  }
  else {
    local_d0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x773);
    local_2c8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x774);
    local_2f0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x775);
  }
  local_cc = (**(code **)(*(int *)param_1[10] + 0x48))(0x23c);
  if (param_1[0x9f21] != 0) {
    (**(code **)(*(int *)param_1[0x9f21] + 0x60))(0);
    if (local_330 != 0) {
      *(undefined4 *)(local_330 + 0x3c) = 0;
    }
    if (local_32c != 0) {
      *(undefined4 *)(local_32c + 0x3c) = 0;
    }
    if (local_d4 != 0) {
      *(undefined4 *)(local_d4 + 0x3c) = 0;
    }
    *(undefined4 *)(local_cc + 0x3c) = 0;
    *(undefined4 *)(local_20 + 0x3c) = 0;
    if (DAT_005b892c != 2) {
      *(undefined4 *)(local_d0 + 0x3c) = 0;
      *(undefined4 *)(local_2c8 + 0x3c) = 0;
      *(undefined4 *)(local_2f0 + 0x3c) = 0;
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x491);
  param_1[0x9eb3] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x492);
  param_1[0x9eb4] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x486);
  param_1[0x9eb6] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x487);
  param_1[0x9eb7] = iVar3;
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3204);
    param_1[0x9eb5] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3205);
    param_1[0x9eb8] = iVar3;
  }
  if (DAT_005b892c == 2) {
    if (param_1[0x9eb3] != 0) {
      (**(code **)(*(int *)param_1[0x9eb3] + 0x74))
                (*(undefined4 *)(param_1[0x9eb3] + 0x54),
                 *(float *)(param_1[0x9eb3] + 0x58) + _DAT_005a3794);
    }
    if (param_1[0x9eb4] != 0) {
      (**(code **)(*(int *)param_1[0x9eb4] + 0x74))
                (*(undefined4 *)(param_1[0x9eb4] + 0x54),
                 *(float *)(param_1[0x9eb4] + 0x58) + _DAT_005a3794);
    }
    if (param_1[0x9eb3] != 0) {
      (**(code **)(*(int *)param_1[0x9eb3] + 0x70))
                (*(undefined4 *)(param_1[0x9eb3] + 0x4c),
                 *(float *)(param_1[0x9eb3] + 0x50) - _DAT_005a40f0);
    }
    if (param_1[0x9eb4] != 0) {
      (**(code **)(*(int *)param_1[0x9eb4] + 0x70))
                (*(undefined4 *)(param_1[0x9eb4] + 0x4c),
                 *(float *)(param_1[0x9eb4] + 0x50) - _DAT_005a40f0);
    }
    if (param_1[0x9eb6] != 0) {
      (**(code **)(*(int *)param_1[0x9eb6] + 0x74))
                (*(undefined4 *)(param_1[0x9eb6] + 0x54),
                 *(float *)(param_1[0x9eb6] + 0x58) + _DAT_005a3794);
    }
    if (param_1[0x9eb7] != 0) {
      (**(code **)(*(int *)param_1[0x9eb7] + 0x74))
                (*(undefined4 *)(param_1[0x9eb7] + 0x54),
                 *(float *)(param_1[0x9eb7] + 0x58) + _DAT_005a3794);
    }
    if (param_1[0x9eb6] != 0) {
      (**(code **)(*(int *)param_1[0x9eb6] + 0x70))
                (*(undefined4 *)(param_1[0x9eb6] + 0x4c),
                 *(float *)(param_1[0x9eb6] + 0x50) - _DAT_005a40f0);
    }
    if (param_1[0x9eb7] != 0) {
      (**(code **)(*(int *)param_1[0x9eb7] + 0x70))
                (*(undefined4 *)(param_1[0x9eb7] + 0x4c),
                 *(float *)(param_1[0x9eb7] + 0x50) - _DAT_005a40f0);
    }
    if (param_1[0x9eb5] != 0) {
      (**(code **)(*(int *)param_1[0x9eb5] + 0x74))
                (*(undefined4 *)(param_1[0x9eb5] + 0x54),
                 *(float *)(param_1[0x9eb5] + 0x58) + _DAT_005a3794);
    }
    if (param_1[0x9eb5] != 0) {
      (**(code **)(*(int *)param_1[0x9eb5] + 0x70))
                (*(undefined4 *)(param_1[0x9eb5] + 0x4c),
                 *(float *)(param_1[0x9eb5] + 0x50) - _DAT_005a40f0);
    }
    if (param_1[0x9eb8] != 0) {
      (**(code **)(*(int *)param_1[0x9eb5] + 0x74))
                (*(undefined4 *)(param_1[0x9eb8] + 0x54),
                 *(float *)(param_1[0x9eb8] + 0x58) + _DAT_005a3794);
    }
    if (param_1[0x9eb8] != 0) {
      (**(code **)(*(int *)param_1[0x9eb5] + 0x70))
                (*(undefined4 *)(param_1[0x9eb8] + 0x4c),
                 *(float *)(param_1[0x9eb8] + 0x50) - _DAT_005a40f0);
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x402);
  param_1[0x9eac] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x401);
  param_1[0x9ead] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x404);
  param_1[0x9eae] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x403);
  param_1[0x9eaf] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3202);
  param_1[0x9eb0] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3203);
  param_1[0x9eb1] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x302);
  param_1[0x9eab] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x490);
  param_1[0x9eb2] = iVar3;
  if (param_1[0x9eb2] != 0) {
    (**(code **)(*(int *)param_1[0x9eb2] + 0x60))(0);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x370);
  param_1[0xa17d] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x371);
  param_1[0xa17f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x372);
  param_1[0xa180] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x373);
  param_1[0xa181] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x374);
  param_1[0xa182] = iVar3;
  if (param_1[0xa17d] != 0) {
    *(undefined4 *)(param_1[0xa17d] + 0x3c) = 0;
  }
  if (param_1[0xa17d] != 0) {
    (**(code **)(*(int *)param_1[0xa17d] + 0x60))(0);
  }
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17dc);
    param_1[0xa17e] = iVar3;
    (**(code **)(*(int *)param_1[0xa17d] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0xa17d] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               _DAT_005a431c * DAT_005b4910);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17f7);
  param_1[0xa203] = iVar3;
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17f6);
    param_1[0xa204] = iVar3;
    (**(code **)(*(int *)param_1[0xa203] + 0x70))(0x43960000,0x43960000);
  }
  iVar3 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0xa265] = iVar3;
  param_1[0xa266] = 0;
  _memset(param_1 + 0xa205,0,0x180);
  local_36c = DAT_013b71e8 + 0x6ec;
  local_384 = (int)(uint)*(ushort *)(DAT_013b71e8 + 0x70c) >> 7;
  local_3a0 = (int)(uint)*(ushort *)(DAT_013b71e8 + 0x70e) >> 7;
  FUN_0058f078(local_230,s_env_Field_02d_02d_trn_005b79d0,local_384,local_3a0);
  FUN_0058f078(local_c8,s_env_Field_02d_02d_dat_005b79e8,local_384,local_3a0);
  pvVar2 = operator_new(0x20a70);
  local_8 = 0x15;
  if (pvVar2 == (void *)0x0) {
    local_9e0 = 0;
  }
  else {
    local_9e0 = FUN_00532390();
  }
  local_8 = 0xffffffff;
  param_1[0x1a] = local_9e0;
  iVar3 = FUN_00533dd7(local_230);
  if (iVar3 == 0) {
    if (param_1[0x15] == 0) {
      FUN_0049ed58(2,0,0,0,0);
    }
    param_1[0x15] = 1;
  }
  param_1[0x10] = param_1[0x1a];
  FUN_00534968();
  pvVar2 = operator_new(0x404c);
  local_8 = 0x16;
  if (pvVar2 == (void *)0x0) {
    local_9e4 = 0;
  }
  else {
    local_9e4 = FUN_00540ea3(param_1[0x10]);
  }
  local_8 = 0xffffffff;
  param_1[0x1c] = local_9e4;
  if (param_1[0x1c] != 0) {
    FUN_0054ac09(param_1[0x1c]);
  }
  FUN_00433bae();
  pvVar2 = operator_new(0x894);
  local_8 = 0x17;
  if (pvVar2 == (void *)0x0) {
    local_9e8 = 0;
  }
  else {
    local_9e8 = FUN_004f7ea6(param_1);
  }
  local_8 = 0xffffffff;
  param_1[0x13] = local_9e8;
  if (param_1[0x13] != 0) {
    *(undefined4 *)(param_1[0x13] + 0x20) = *(undefined4 *)(DAT_013b71e8 + 0xc5c);
  }
  local_1b0 = *(byte *)(local_36c + 0xd);
  local_198 = *(undefined2 *)(local_36c + 0xe);
  *(ushort *)(param_1[0x13] + 0x192) = (ushort)local_1b0;
  *(undefined2 *)(param_1[0x13] + 0x194) = local_198;
  *(undefined1 *)(param_1[0x13] + 0x196) = *(undefined1 *)(local_36c + 0xc);
  *(undefined1 *)(local_36c + 0xc) = 0;
  *(undefined1 *)(local_36c + 0xf) = 0;
  FUN_0058f078(param_1[0x13] + 0x168,&DAT_005b7a00,local_36c);
  FUN_00524ded(local_36c);
  FUN_004fa8a1(param_1[0x9b9f]);
  if ((*(char *)(param_1[0x13] + 0x79c) == '\x01') &&
     ((((*(int *)(param_1[0x13] + 0x7a0) == 0x1f || (*(int *)(param_1[0x13] + 0x7a0) == 0x28)) ||
       ((*(int *)(param_1[0x13] + 0x7a0) == 0x14 && (*(short *)(param_1[0x13] + 0x1a2) != 7)))) ||
      (*(int *)(param_1[0x13] + 0x7a0) == 0x27)))) {
    *(undefined4 *)(param_1[0x13] + 0x220) = 0x40400000;
  }
  local_1ac = *(undefined2 *)(DAT_013b71e8 + 0x2c4 + *(char *)(DAT_013b71e8 + 0xef4) * 2);
  *(undefined2 *)(param_1[0x13] + 0x682) = *(undefined2 *)(local_36c + 0xbc);
  *(undefined2 *)(param_1[0x13] + 0x680) = local_1ac;
  local_380 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2000);
  local_39c = (**(code **)(*(int *)param_1[10] + 0x48))(0x2001);
  local_350 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2002);
  local_368 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2003);
  local_328 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2004);
  local_338 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2005);
  local_2fc = (**(code **)(*(int *)param_1[10] + 0x48))(0x2006);
  local_310 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2007);
  local_2c4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2008);
  local_1c = (**(code **)(*(int *)param_1[10] + 0x48))(0x2009);
  local_48 = (**(code **)(*(int *)param_1[10] + 0x48))(0x200a);
  local_37c = (**(code **)(*(int *)param_1[10] + 0x48))(0x200b);
  local_398 = (**(code **)(*(int *)param_1[10] + 0x48))(0x200c);
  local_34c = (**(code **)(*(int *)param_1[10] + 0x48))(0x200d);
  local_364 = (**(code **)(*(int *)param_1[10] + 0x48))(0x200e);
  *(undefined4 *)(local_380 + 0x400) = 9;
  *(undefined4 *)(local_39c + 0x400) = 9;
  *(undefined4 *)(local_350 + 0x400) = 9;
  *(undefined4 *)(local_368 + 0x400) = 9;
  *(undefined4 *)(local_328 + 0x400) = 9;
  *(undefined4 *)(local_338 + 0x400) = 9;
  *(undefined4 *)(local_2fc + 0x400) = 9;
  *(undefined4 *)(local_310 + 0x400) = 9;
  *(undefined4 *)(local_2c4 + 0x400) = 9;
  *(undefined4 *)(local_1c + 0x400) = 9;
  *(undefined4 *)(local_48 + 0x400) = 9;
  *(undefined4 *)(local_37c + 0x400) = 9;
  *(undefined4 *)(local_398 + 0x400) = 9;
  *(undefined4 *)(local_34c + 0x400) = 9;
  *(undefined4 *)(local_364 + 0x400) = 9;
  local_324 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2100);
  local_334 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2101);
  local_2f8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2102);
  local_30c = (**(code **)(*(int *)param_1[10] + 0x48))(0x2103);
  local_2c0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2104);
  local_2ec = (**(code **)(*(int *)param_1[10] + 0x48))(0x2105);
  local_194 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2106);
  local_1a8 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2107);
  local_ec = (**(code **)(*(int *)param_1[10] + 0x48))(0x2108);
  local_18 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2109);
  local_44 = (**(code **)(*(int *)param_1[10] + 0x48))(0x210a);
  local_378 = (**(code **)(*(int *)param_1[10] + 0x48))(0x210b);
  local_394 = (**(code **)(*(int *)param_1[10] + 0x48))(0x210c);
  local_348 = (**(code **)(*(int *)param_1[10] + 0x48))(0x210d);
  local_360 = (**(code **)(*(int *)param_1[10] + 0x48))(0x210e);
  *(undefined4 *)(local_324 + 0x400) = 6;
  *(undefined4 *)(local_334 + 0x400) = 6;
  *(undefined4 *)(local_2f8 + 0x400) = 6;
  *(undefined4 *)(local_30c + 0x400) = 6;
  *(undefined4 *)(local_2c0 + 0x400) = 6;
  *(undefined4 *)(local_2ec + 0x400) = 6;
  *(undefined4 *)(local_194 + 0x400) = 6;
  *(undefined4 *)(local_1a8 + 0x400) = 6;
  *(undefined4 *)(local_ec + 0x400) = 6;
  *(undefined4 *)(local_18 + 0x400) = 6;
  *(undefined4 *)(local_44 + 0x400) = 6;
  *(undefined4 *)(local_378 + 0x400) = 6;
  *(undefined4 *)(local_394 + 0x400) = 6;
  *(undefined4 *)(local_348 + 0x400) = 6;
  *(undefined4 *)(local_360 + 0x400) = 6;
  local_1a4 = (**(code **)(*(int *)param_1[10] + 0x48))(0x259);
  local_40 = (**(code **)(*(int *)param_1[10] + 0x48))(0x269);
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x726);
  param_1[0xa1ce] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x151);
  param_1[0xa1cf] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x152);
  param_1[0xa1d0] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x157);
  param_1[0xa1d1] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x158);
  param_1[0xa1d2] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x159);
  param_1[0xa1d3] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x160);
  param_1[0xa1d4] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x153);
  param_1[0xa1d5] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x154);
  param_1[0xa1d6] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x165);
  param_1[0xa1d7] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x166);
  param_1[0xa1d8] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x161);
  param_1[0xa1d9] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x162);
  param_1[0xa1da] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x163);
  param_1[0xa1db] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164);
  param_1[0xa1dc] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x167);
  param_1[0xa1dd] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x168);
  param_1[0xa1de] = iVar3;
  *(undefined4 *)(param_1[0xa1de] + 0x400) = 8;
  *(undefined4 *)(param_1[0xa1cf] + 0x1f8) = 1;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x706);
  param_1[0xa1cc] = iVar3;
  *(undefined4 *)(param_1[0xa1cc] + 0x1f8) = 1;
  *(undefined4 *)(param_1[0xa1cc] + 0x400) = 1;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x766);
  param_1[0xa1cd] = iVar3;
  if (DAT_005b892c == 2) {
    *(undefined4 *)(param_1[0xa1cd] + 0x1f8) = 0;
  }
  else {
    *(undefined4 *)(param_1[0xa1cd] + 0x1f8) = 1;
  }
  *(undefined4 *)(param_1[0xa1cd] + 0x400) = 4;
  local_190 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x726);
  local_190[0x7e] = 1;
  local_190[0x100] = 2;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x23b);
  param_1[0xa1c5] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x23d);
  param_1[0xa1c6] = iVar3;
  *(undefined4 *)(param_1[0xa1c6] + 0x400) = 5;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x24a);
  param_1[0xa1c7] = iVar3;
  *(undefined4 *)(param_1[0xa1c7] + 0x400) = 5;
  (**(code **)(*(int *)param_1[0xa1c7] + 0x60))(0);
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x24b);
  param_1[0xa1c8] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x24c);
  param_1[0xa1c9] = iVar3;
  (**(code **)(*(int *)param_1[0xa1c8] + 0x8c))(1);
  local_308 = FUN_0054cd07(local_36c + 0x5c,0x12);
  if ((0x28 < *(short *)(local_36c + 100)) && (local_308 != 0x15)) {
    local_3e8 = 0;
    local_3ec = operator_new(8);
    FUN_0058f220(local_3ec,local_36c + 100,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x18;
    if (pvVar2 == (void *)0x0) {
      local_9ec = 0;
    }
    else {
      local_9ec = FUN_0040d13e(0,local_3ec,0,0);
    }
    local_8 = 0xffffffff;
    local_3e8 = local_9ec;
    (**(code **)(*(int *)param_1[0xa1d0] + 0x8c))(local_9ec,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x6c)) {
    local_3f0 = 0;
    local_3f4 = operator_new(8);
    FUN_0058f220(local_3f4,local_36c + 0x6c,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x19;
    if (pvVar2 == (void *)0x0) {
      local_9f0 = 0;
    }
    else {
      local_9f0 = FUN_0040d13e(0,local_3f4,0,0);
    }
    local_8 = 0xffffffff;
    local_3f0 = local_9f0;
    (**(code **)(*(int *)param_1[0xa1d1] + 0x8c))(local_9f0,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x74)) {
    local_3f8 = 0;
    local_3fc = operator_new(8);
    FUN_0058f220(local_3fc,local_36c + 0x74,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x1a;
    if (pvVar2 == (void *)0x0) {
      local_9f4 = 0;
    }
    else {
      local_9f4 = FUN_0040d13e(0,local_3fc,0,0);
    }
    local_8 = 0xffffffff;
    local_3f8 = local_9f4;
    (**(code **)(*(int *)param_1[0xa1d2] + 0x8c))(local_9f4,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x7c)) {
    local_400 = 0;
    local_404 = operator_new(8);
    FUN_0058f220(local_404,local_36c + 0x7c,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x1b;
    if (pvVar2 == (void *)0x0) {
      local_9f8 = 0;
    }
    else {
      local_9f8 = FUN_0040d13e(0,local_404,0,0);
    }
    local_8 = 0xffffffff;
    local_400 = local_9f8;
    (**(code **)(*(int *)param_1[0xa1d3] + 0x8c))(local_9f8,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x84)) {
    local_40c = 0;
    local_408 = operator_new(8);
    FUN_0058f220(local_408,local_36c + 0x84,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x1c;
    if (pvVar2 == (void *)0x0) {
      local_9fc = 0;
    }
    else {
      local_9fc = FUN_0040d13e(0,local_408,0,0);
    }
    local_8 = 0xffffffff;
    local_40c = local_9fc;
    (**(code **)(*(int *)param_1[0xa1d4] + 0x8c))(local_9fc,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x94)) {
    local_410 = 0;
    local_414 = operator_new(8);
    FUN_0058f220(local_414,local_36c + 0x94,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x1d;
    if (pvVar2 == (void *)0x0) {
      local_a00 = 0;
    }
    else {
      local_a00 = FUN_0040d13e(0,local_414,0,0);
    }
    local_8 = 0xffffffff;
    local_410 = local_a00;
    (**(code **)(*(int *)param_1[0xa1d5] + 0x8c))(local_a00,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x8c)) {
    local_41c = 0;
    local_418 = operator_new(8);
    FUN_0058f220(local_418,local_36c + 0x8c,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x1e;
    if (pvVar2 == (void *)0x0) {
      local_a04 = 0;
    }
    else {
      local_a04 = FUN_0040d13e(0,local_418,0,0);
    }
    local_8 = 0xffffffff;
    local_41c = local_a04;
    (**(code **)(*(int *)param_1[0xa1d6] + 0x8c))(local_a04,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xbc)) {
    local_420 = 0;
    local_424 = operator_new(8);
    FUN_0058f220(local_424,local_36c + 0xbc,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x1f;
    if (pvVar2 == (void *)0x0) {
      local_a08 = 0;
    }
    else {
      local_a08 = FUN_0040d13e(0,local_424,0,0);
    }
    local_8 = 0xffffffff;
    local_420 = local_a08;
    (**(code **)(*(int *)param_1[0xa1d7] + 0x8c))(local_a08,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xc4)) {
    local_42c = 0;
    local_428 = operator_new(8);
    FUN_0058f220(local_428,local_36c + 0xc4,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x20;
    if (pvVar2 == (void *)0x0) {
      local_a0c = 0;
    }
    else {
      local_a0c = FUN_0040d13e(0,local_428,0,0);
    }
    local_8 = 0xffffffff;
    local_42c = local_a0c;
    (**(code **)(*(int *)param_1[0xa1d8] + 0x8c))(local_a0c,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xcc)) {
    local_434 = 0;
    local_430 = operator_new(8);
    FUN_0058f220(local_430,local_36c + 0xcc,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x21;
    if (pvVar2 == (void *)0x0) {
      local_a10 = 0;
    }
    else {
      local_a10 = FUN_0040d13e(0,local_430,0,0);
    }
    local_8 = 0xffffffff;
    local_434 = local_a10;
    (**(code **)(*(int *)param_1[0xa1dd] + 0x8c))(local_a10,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xd4)) {
    local_438 = 0;
    local_43c = operator_new(8);
    FUN_0058f220(local_43c,local_36c + 0xd4,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x22;
    if (pvVar2 == (void *)0x0) {
      local_a14 = 0;
    }
    else {
      local_a14 = FUN_0040d13e(0,local_43c,0,0);
    }
    local_8 = 0xffffffff;
    local_438 = local_a14;
    (**(code **)(*(int *)param_1[0xa1de] + 0x8c))(local_a14,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0x9c)) {
    local_444 = 0;
    local_440 = operator_new(8);
    FUN_0058f220(local_440,local_36c + 0x9c,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x23;
    if (pvVar2 == (void *)0x0) {
      local_a18 = 0;
    }
    else {
      local_a18 = FUN_0040d13e(0,local_440,0,0);
    }
    local_8 = 0xffffffff;
    local_444 = local_a18;
    (**(code **)(*(int *)param_1[0xa1d9] + 0x8c))(local_a18,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xa4)) {
    local_44c = 0;
    local_448 = operator_new(8);
    FUN_0058f220(local_448,local_36c + 0xa4,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x24;
    if (pvVar2 == (void *)0x0) {
      local_a1c = 0;
    }
    else {
      local_a1c = FUN_0040d13e(0,local_448,0,0);
    }
    local_8 = 0xffffffff;
    local_44c = local_a1c;
    (**(code **)(*(int *)param_1[0xa1da] + 0x8c))(local_a1c,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xac)) {
    local_450 = 0;
    local_454 = operator_new(8);
    FUN_0058f220(local_454,local_36c + 0xac,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x25;
    if (pvVar2 == (void *)0x0) {
      local_a20 = 0;
    }
    else {
      local_a20 = FUN_0040d13e(0,local_454,0,0);
    }
    local_8 = 0xffffffff;
    local_450 = local_a20;
    (**(code **)(*(int *)param_1[0xa1db] + 0x8c))(local_a20,0,0);
  }
  if (0x28 < *(short *)(local_36c + 0xb4)) {
    local_45c = 0;
    local_458 = operator_new(8);
    FUN_0058f220(local_458,local_36c + 0xb4,8);
    pvVar2 = operator_new(0x678);
    local_8 = 0x26;
    if (pvVar2 == (void *)0x0) {
      local_a24 = 0;
    }
    else {
      local_a24 = FUN_0040d13e(0,local_458,0,0);
    }
    local_8 = 0xffffffff;
    local_45c = local_a24;
    (**(code **)(*(int *)param_1[0xa1dc] + 0x8c))(local_a24,0,0);
  }
  for (local_2e8 = 0; local_2e8 < 0x3f; local_2e8 = local_2e8 + 1) {
    if (0x28 < *(short *)(local_36c + 0xdc + local_2e8 * 8)) {
      local_464 = 0;
      local_460 = operator_new(8);
      FUN_0058f220(local_460,local_36c + 0xdc + local_2e8 * 8,8);
      pvVar2 = operator_new(0x678);
      local_8 = 0x27;
      if (pvVar2 == (void *)0x0) {
        local_a28 = 0;
      }
      else {
        local_a28 = FUN_0040d13e(0,local_460,0,0);
      }
      local_8 = 0xffffffff;
      local_464 = local_a28;
      (**(code **)(*(int *)param_1[0xa1cf] + 0x8c))(local_a28,local_2e8 % 9,local_2e8 / 9);
    }
  }
  local_2e4 = DAT_013b71e8 + 0x2ec;
  for (local_2bc = 0; local_2bc < 0x7e; local_2bc = local_2bc + 1) {
    if (*(short *)(DAT_013b71e8 + 0x2ec + local_2bc * 8) != 0) {
      local_468 = 0;
      local_46c = operator_new(8);
      FUN_0058f220(local_46c,local_2e4 + local_2bc * 8,8);
      pvVar2 = operator_new(0x678);
      local_8 = 0x28;
      if (pvVar2 == (void *)0x0) {
        local_a2c = 0;
      }
      else {
        local_a2c = FUN_0040d13e(0,local_46c,0,0);
      }
      local_8 = 0xffffffff;
      local_468 = local_a2c;
      (**(code **)(*local_190 + 0x8c))(local_a2c,local_2bc % 9,local_2bc / 9);
    }
  }
  FUN_0058f220(param_1[0x13] + 0x444,local_36c + 0x40,0x1c);
  local_17c = (float)(int)*(short *)(param_1[0x13] + 0x45a);
  FUN_005277a7(local_17c);
  FUN_004faf13(CONCAT22(extraout_var,*(undefined2 *)(local_36c + 0x5c)));
  local_14 = FUN_0054cd07(local_36c + 0x8c,0x15);
  if (local_14 == 0x29) {
    *(undefined2 *)(param_1[0x13] + 0x1ea) = *(undefined2 *)(param_1[0x13] + 0x1ee);
    *(undefined2 *)(param_1[0x13] + 0x1ec) = *(undefined2 *)(param_1[0x13] + 0x1f0);
    *(undefined1 *)(param_1[0x13] + 0x1f8) = *(undefined1 *)(param_1[0x13] + 0x1f9);
    *(undefined1 *)(param_1[0x13] + 0x200) = *(undefined1 *)(param_1[0x13] + 0x201);
  }
  (**(code **)(*(int *)param_1[0x13] + 0x38))();
  FUN_0051bb41(*(undefined2 *)(local_36c + 0x8c),
               CONCAT22(extraout_var_00,*(undefined2 *)(local_36c + 0x94)));
  (**(code **)(*(int *)param_1[0x13] + 0x40))(0,0x3ec90fdb,0);
  (**(code **)(*(int *)param_1[0x13] + 0x3c))
            ((float)*(ushort *)(local_36c + 0x20) + _DAT_005a34a0,0,
             (float)*(ushort *)(local_36c + 0x22) + _DAT_005a34a0);
  (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x48))(param_1[0x13]);
  *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x24) = 0;
  uVar1 = FUN_00433a60(*(undefined2 *)(local_36c + 0x20),*(undefined2 *)(local_36c + 0x22));
  *(undefined1 *)(param_1 + 0x9baa) = uVar1;
  FUN_004431e4(0);
  FUN_004470b9();
  *(undefined1 *)(DAT_013b71e8 + 0xef5) = 0;
  for (local_31c = 0; local_31c < 10; local_31c = local_31c + 1) {
    if ((-1 < *(char *)(DAT_013b71e8 + local_31c + 0xef6)) &&
       (*(char *)(DAT_013b71e8 + local_31c + 0xef6) < 'h')) {
      FUN_004528c5(local_31c + 0x31,0);
    }
  }
  iVar3 = FUN_004ff388();
  if ((iVar3 == 2) && (0x4e < *(short *)(DAT_013b71e8 + 0x72c))) {
    _DAT_00932544 = 1000;
  }
  iVar3 = FUN_00541065(local_c8);
  if (iVar3 == 0) {
    FUN_00431159(s_DataFile_Not_Found_005b7a04);
    if (param_1[0x15] == 0) {
      FUN_0049ed58(3,0,0,0,0);
    }
    param_1[0x15] = 1;
  }
  for (local_2e0 = 0; local_2e0 < 0x80; local_2e0 = local_2e0 + 1) {
    FUN_0058f220(param_1 + local_2e0 * 0x40 + 0x25,param_1[0x10] + 0xc0cc + local_2e0 * 0x80,0x80);
  }
  DAT_005b8924 = __ftol();
  DAT_005b8928 = __ftol();
  FUN_0054f18b(param_1 + 0x25,0x100);
  FUN_0058f220(param_1 + 0x4025,param_1 + 0x25,0x10000);
  pvVar2 = operator_new(0x24);
  local_8 = 0x29;
  if (pvVar2 == (void *)0x0) {
    local_a3c = 0;
  }
  else {
    local_a3c = FUN_0054a9e0(0);
  }
  local_8 = 0xffffffff;
  param_1[0xe] = local_a3c;
  FUN_0054ac09(param_1[0x1a]);
  pvVar2 = operator_new(0x16c);
  local_8 = 0x2a;
  if (pvVar2 == (void *)0x0) {
    local_a40 = 0;
  }
  else {
    local_a40 = FUN_004eef50();
  }
  local_8 = 0xffffffff;
  param_1[0x12] = local_a40;
  if (param_1[0x12] != 0) {
    (**(code **)(*(int *)param_1[0x12] + 0x38))();
    FUN_0054ac09(param_1[0x12]);
  }
  pvVar2 = operator_new(0x22a4);
  local_8 = 0x2b;
  if (pvVar2 == (void *)0x0) {
    local_a44 = 0;
  }
  else {
    local_a44 = FUN_00547890();
  }
  local_8 = 0xffffffff;
  param_1[0x11] = local_a44;
  *(undefined4 *)(param_1[0x11] + 0x48) = 0;
  FUN_0054ac09(param_1[0x11]);
  FUN_0054ac09(param_1[0xe]);
  FUN_00480c25();
  if ((*(uint *)(param_1[0x13] + 0x20) < 1000) && ((*(byte *)(param_1[0x13] + 0x44a) & 1) == 1)) {
    *(undefined1 *)(param_1[0x13] + 0x795) = 1;
  }
  else {
    *(undefined1 *)(param_1[0x13] + 0x795) = 0;
  }
  FUN_0054ac09(param_1[0x13]);
  if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x66), iVar3 != 0)) {
    FUN_00429a6d(0x66);
    iVar3 = FUN_0042afd0();
    if (iVar3 == 0) {
      uVar6 = 1;
      uVar5 = 0;
      FUN_00429a6d(0x66);
      FUN_0042ad2b(uVar5,uVar6);
    }
  }
  pvVar2 = operator_new(0x404);
  local_8 = 0x2c;
  if (pvVar2 == (void *)0x0) {
    local_a4c = 0;
  }
  else {
    local_a4c = FUN_004dcdc0();
  }
  local_8 = 0xffffffff;
  param_1[0x9ba7] = local_a4c;
  *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
  FUN_0054ac09(param_1[0x9ba7]);
  pvVar2 = operator_new(0xd68);
  local_8 = 0x2d;
  if (pvVar2 == (void *)0x0) {
    local_a50 = 0;
  }
  else {
    local_a50 = FUN_004de30e(0x3f800000);
  }
  local_8 = 0xffffffff;
  param_1[0x9ba8] = local_a50;
  *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
  FUN_0054ac09(param_1[0x9ba8]);
  pvVar2 = operator_new(0xd68);
  local_8 = 0x2e;
  if (pvVar2 == (void *)0x0) {
    local_a54 = 0;
  }
  else {
    local_a54 = FUN_004de30e(0x40000000);
  }
  local_8 = 0xffffffff;
  param_1[0x9ba9] = local_a54;
  *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
  FUN_0054ac09(param_1[0x9ba9]);
  pvVar2 = operator_new(0xcc);
  local_8 = 0x2f;
  if (pvVar2 == (void *)0x0) {
    local_a58 = 0;
  }
  else {
    local_a58 = FUN_004d2230(0x13c,0xff111188,0,0);
  }
  local_8 = 0xffffffff;
  param_1[0x9ba1] = local_a58;
  *(undefined4 *)(param_1[0x9ba1] + 0x6c) = 1;
  FUN_0054ac09(param_1[0x9ba1]);
  pvVar2 = operator_new(0xcc);
  local_8 = 0x30;
  if (pvVar2 == (void *)0x0) {
    local_a5c = 0;
  }
  else {
    local_a5c = FUN_004d2230(0x13d,0xffff0000,0,0);
  }
  local_8 = 0xffffffff;
  param_1[0x9ba2] = local_a5c;
  FUN_0054ac09(param_1[0x9ba2]);
  pvVar2 = operator_new(0x124);
  local_8 = 0x31;
  if (pvVar2 == (void *)0x0) {
    local_a60 = 0;
  }
  else {
    local_a60 = FUN_004cb900(0xe8,0,0x3f800000,0x3f800000,0x3f800000,0x3a03126f,0);
  }
  local_8 = 0xffffffff;
  param_1[0x9ba3] = local_a60;
  *(undefined4 *)(param_1[0x9ba3] + 0x6c) = 1;
  (**(code **)(*(int *)param_1[0x9ba3] + 0x3c))(0xff9999aa);
  FUN_0054ac09(param_1[0x9ba3]);
  *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
  *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f800000;
  *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f800000;
  *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0;
  *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3e4ccccd;
  *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3e99999a;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x121);
  param_1[0x9f66] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x122);
  param_1[0x9f67] = iVar3;
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1649);
    param_1[0x9f68] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164a);
    param_1[0x9f69] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164b);
    param_1[0x9f6a] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164c);
    param_1[0x9f6b] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164d);
    param_1[0x9f6c] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164e);
    param_1[0x9f6d] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x164f);
    param_1[0x9f6e] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1650);
    param_1[0x9f6f] = iVar3;
    for (local_470 = 0; local_470 < 8; local_470 = local_470 + 1) {
      if (param_1[local_470 + 0x9f68] != 0) {
        (**(code **)(*(int *)param_1[local_470 + 0x9f68] + 0x60))(0);
      }
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1651);
    param_1[0x9f70] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1652);
    param_1[0x9f71] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1653);
    param_1[0x9f72] = iVar3;
    if (param_1[0x9e93] != 0) {
      (**(code **)(*(int *)param_1[0x9e93] + 0x80))(param_1[0x13] + 0x168,1);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17f8);
    param_1[0x9f73] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x17f9);
    param_1[0x9f74] = iVar3;
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x123);
  param_1[0x9f65] = iVar3;
  local_344 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x128);
  if (param_1[0x9f66] != 0) {
    iVar3 = (**(code **)(*(int *)param_1[0x9f66] + 0x84))();
    *(undefined4 *)(iVar3 + 0x18) = 0xbf490fdb;
    *(undefined4 *)(param_1[0x9f66] + 0x3c) = 0;
    if (param_1[0x9f65] != 0) {
      *(undefined4 *)(param_1[0x9f65] + 0x3c) = 0;
    }
    if (param_1[0x9f67] != 0) {
      *(undefined4 *)(param_1[0x9f67] + 0x3c) = 0;
      (**(code **)(*(int *)param_1[0x9f67] + 0x60))(0);
    }
    (**(code **)(*(int *)param_1[0x9f66] + 0x60))(0);
    if (local_344 != (int *)0x0) {
      (**(code **)(*local_344 + 0x8c))(0);
    }
    *(undefined4 *)(param_1[0x9f66] + 0x94) = 0xd0ffffff;
  }
  for (local_31c = 0; local_31c < 0x100; local_31c = local_31c + 1) {
    pvVar2 = operator_new(0x1e4);
    local_8 = 0x32;
    if (pvVar2 == (void *)0x0) {
      local_a64 = 0;
    }
    else {
      local_a64 = FUN_00401466(0xfffffffe,0,0,0x40800000,0x40800000,
                               *(undefined4 *)(&DAT_005ccfc8 + local_31c * 0x34),4);
    }
    local_8 = 0xffffffff;
    param_1[local_31c + 0xa075] = local_a64;
    if (param_1[local_31c + 0xa075] != 0) {
      *(undefined4 *)(param_1[local_31c + 0xa075] + 0x3c) = 0;
      (**(code **)(*(int *)param_1[local_31c + 0xa075] + 0x60))(0);
      FUN_0054ac09(param_1[local_31c + 0xa075]);
    }
    pvVar2 = operator_new(0xe4c);
    local_8 = 0x33;
    if (pvVar2 == (void *)0x0) {
      local_a68 = 0;
    }
    else {
      local_a68 = FUN_00401d03(0xfffffffe,&DAT_005ccfcc + local_31c * 0x34,
                               *(undefined4 *)(&DAT_005ccfc8 + local_31c * 0x34),0,0,0x41000000,
                               0x41400000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    param_1[local_31c + 0x9f75] = local_a68;
    if (param_1[local_31c + 0x9f75] != 0) {
      *(undefined4 *)(param_1[local_31c + 0x9f75] + 0x3c) = 0;
      (**(code **)(*(int *)param_1[local_31c + 0x9f75] + 0x60))(0);
      FUN_0054ac09(param_1[local_31c + 0x9f75]);
    }
  }
  local_2b8 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x741);
  local_374 = (**(code **)(*(int *)param_1[10] + 0x48))(0x745);
  local_3c = (**(code **)(*(int *)param_1[10] + 0x48))(0x746);
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x747);
  param_1[0xa1cb] = iVar3;
  param_1[0x9f63] = (int)local_2b8;
  if (DAT_005b892c == 2) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x166e);
    param_1[0x9f64] = iVar3;
    if (local_2b8 != (int *)0x0) {
      (**(code **)(*local_2b8 + 0x70))
                (0,((float)*(uint *)(DAT_005ccf9c + 0x2a508) - (float)local_2b8[0x16]) -
                   _DAT_005a4308);
    }
    if (param_1[0x9f64] != 0) {
      (**(code **)(*(int *)param_1[0x9f64] + 0x70))
                (0,((float)*(uint *)(DAT_005ccf9c + 0x2a508) - (float)local_2b8[0x16]) -
                   _DAT_005a4308);
    }
    if (param_1[0x9f64] != 0) {
      (**(code **)(**(int **)(param_1[0x9f64] + 0x1f0) + 0x70))
                (*(float *)(*(int *)(param_1[0x9f64] + 0x1f0) + 0x4c) + _DAT_005a4104,
                 (float)local_2b8[0x16] * _DAT_005a34a0 - _DAT_005a4304);
    }
    (**(code **)(*param_1 + 0x58))(0x166e,0);
  }
  else {
    local_478 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x74a);
    local_474 = (**(code **)(*(int *)param_1[10] + 0x48))(0x74b);
    if (local_2b8 != (int *)0x0) {
      (**(code **)(*local_2b8 + 0x60))(0);
      if (local_478 != (int *)0x0) {
        (**(code **)(*local_478 + 0x60))(0);
      }
      if (local_374 != 0) {
        *(undefined4 *)(local_374 + 0x3c) = 0;
      }
      if (local_3c != 0) {
        *(undefined4 *)(local_3c + 0x3c) = 0;
      }
      if (local_474 != 0) {
        *(undefined4 *)(local_474 + 0x3c) = 0;
      }
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x29d);
  param_1[0x9f22] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x29e);
  param_1[0x9f23] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x27e);
  param_1[0x9f24] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x333);
  param_1[0x9f4e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x334);
  param_1[0x9f4f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x337);
  param_1[0x9f50] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x33c);
  param_1[0x9f51] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x346);
  param_1[0x9f52] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x339);
  param_1[0x9f53] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x33d);
  param_1[0x9f54] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x33e);
  param_1[0x9f55] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x348);
  param_1[0x9f56] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x345);
  param_1[0x9f57] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x347);
  param_1[0x9f58] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x344);
  param_1[0x9f59] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x340);
  param_1[0x9f5e] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x342);
  param_1[0x9f5f] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x34a);
  param_1[0x9f60] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x33f);
  param_1[0x9f61] = iVar3;
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x33b);
  param_1[0x9f62] = iVar3;
  FUN_00441823();
  iVar3 = FUN_00442dd0(param_1[0x9f23],s_notice_txt_005b7a1c);
  if (iVar3 == 0) {
    (**(code **)(*(int *)param_1[0x9f22] + 0x60))(0);
  }
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x9f22] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f22] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9f22] + 0x58) * _DAT_005a34a0);
  }
  (**(code **)(*(int *)param_1[0x9f22] + 0x60))(0);
  FUN_004819d5(DAT_005d03e4);
  if (param_1[0x9f32] != 0) {
    local_47c = 0;
    while ((local_47c < 99 &&
           ((*(char *)(DAT_013b71e8 + 0x1b098 + local_47c * 0x84) != '\0' ||
            (*(char *)(DAT_013b71e8 + 0x1b098 + (local_47c + 1) * 0x84) != '\0'))))) {
      pvVar2 = operator_new(0xe50);
      local_8 = 0x34;
      if (pvVar2 == (void *)0x0) {
        local_a8c = 0;
      }
      else {
        local_a8c = FUN_00407203(DAT_013b71e8 + 0x1b098 + local_47c * 0x84,
                                 *(undefined4 *)(DAT_013b71e8 + 0x1b094 + local_47c * 0x84),0,0,
                                 0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      local_480 = local_a8c;
      if (local_a8c != 0) {
        FUN_00408d33(local_a8c);
      }
      local_47c = local_47c + 1;
    }
    if ((0 < local_47c) && (param_1[0x9f3c] != 0)) {
      (**(code **)(*(int *)param_1[0x9f3c] + 0x60))(1);
    }
  }
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0x3b4);
    local_8 = 0x35;
    if (pvVar2 == (void *)0x0) {
      local_a90 = 0;
    }
    else {
      local_a90 = FUN_0040a189(0x14a,0x41a80000,0x429e0000,0x42780000,0x42780000,0x3f800000);
    }
    local_8 = 0xffffffff;
    param_1[0xa267] = local_a90;
    pvVar2 = operator_new(0x3b4);
    local_8 = 0x36;
    if (pvVar2 == (void *)0x0) {
      local_a94 = 0;
    }
    else {
      local_a94 = FUN_0040a189(0x154,0x41a80000,0x429e0000,0x42780000,0x42780000,0x3f800000);
    }
    param_1[0xa268] = local_a94;
  }
  else {
    pvVar2 = operator_new(0x3b4);
    local_8 = 0x37;
    if (pvVar2 == (void *)0x0) {
      local_a98 = 0;
    }
    else {
      local_a98 = FUN_0040a189(0x14a,0x42200000,0x42ce0000,0x42780000,0x42780000,0x3f800000);
    }
    local_8 = 0xffffffff;
    param_1[0xa267] = local_a98;
    pvVar2 = operator_new(0x3b4);
    local_8 = 0x38;
    if (pvVar2 == (void *)0x0) {
      local_a9c = 0;
    }
    else {
      local_a9c = FUN_0040a189(0x154,0x42200000,0x42ce0000,0x42780000,0x42780000,0x3f800000);
    }
    param_1[0xa268] = local_a9c;
  }
  local_8 = 0xffffffff;
  FUN_0054ac09(param_1[0xa267]);
  FUN_0054ac09(param_1[0xa268]);
  iVar3 = _strcmp(&DAT_005d03b8,(char *)(DAT_013b71e8 + 0x6ec));
  if ((iVar3 == 0) && (iVar3 = _strcmp(&DAT_005d03c8,&DAT_005b7a28), iVar3 != 0)) {
    _memset(local_500,0,0x80);
    local_4fa = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_4fc = 0x334;
    FUN_0058f078(local_4f4,&PTR_DAT_005b7a2c);
    FUN_0058f078(local_4e4,&DAT_005d03c8);
    FUN_0055f2dd(local_500,0x80);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3000);
  param_1[0xa2e4] = iVar3;
  if ((DAT_005b892c == 2) && (param_1[0xa2e4] != 0)) {
    DAT_005d04f4 = 0x8c;
    pvVar2 = operator_new(0x1e4);
    local_8 = 0x39;
    if (pvVar2 == (void *)0x0) {
      local_aa0 = 0;
    }
    else {
      local_aa0 = FUN_00401466(0xfffffe78,0,0,0x42200000,0x42340000,0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0xa2e5] = local_aa0;
    iVar3 = DAT_005d04f4;
    iVar4 = DAT_005d04f4 + -0x41;
    pvVar2 = operator_new(0x1e4);
    local_8 = 0x3a;
    if (pvVar2 == (void *)0x0) {
      local_aa4 = 0;
    }
    else {
      local_aa4 = FUN_00401466(0xfffffe77,0x41b80000,0,(float)iVar4,0x42340000,0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0xa2e6] = local_aa4;
    pvVar2 = operator_new(0x1e4);
    local_8 = 0x3b;
    if (pvVar2 == (void *)0x0) {
      local_aa8 = 0;
    }
    else {
      local_aa8 = FUN_00401466(0xfffffe76,(float)iVar4 + _DAT_005a3f84,0,0x42200000,0x42340000,
                               0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0xa2e7] = local_aa8;
    for (local_50c = 0; local_50c < 3; local_50c = local_50c + 1) {
      FUN_0054ac09(param_1[local_50c + 0xa2e5]);
    }
    (**(code **)(*(int *)param_1[0xa2e4] + 0x70))
              ((float)(*(uint *)(DAT_005ccf9c + 0x2a504) >> 1) -
               (float)(iVar3 + 0xf) / _DAT_005a365c,
               (float)(*(uint *)(DAT_005ccf9c + 0x2a508) >> 1) -
               *(float *)(param_1[0xa2e4] + 0x58) / _DAT_005a365c);
  }
  else {
    DAT_005d04f4 = 0x78;
  }
  if ((DAT_005b892c == 2) && (param_1[0xa2e4] != 0)) {
    pvVar2 = operator_new(0x1e4);
    local_8 = 0x3c;
    if (pvVar2 == (void *)0x0) {
      local_abc = 0;
    }
    else {
      local_abc = FUN_00401466(0xfffffe74,0xbf800000,0x42340000,
                               ((float)DAT_005d04f4 + _DAT_005a42fc) - _DAT_005a42fc,
                               (float)local_2e0 + _DAT_005a4300,0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0xa2e8] = local_abc;
    FUN_0054ac09(param_1[0xa2e8]);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3001);
  param_1[0xa2ee] = iVar3;
  if (param_1[0xa2ee] != 0) {
    if (param_1[0xa2ee] != 0) {
      (**(code **)(*(int *)param_1[0xa2ee] + 0x60))(0);
    }
    if (param_1[0xa2e4] != 0) {
      (**(code **)(*(int *)param_1[0xa2e4] + 0x60))(0);
    }
    if (param_1[10] == 0) {
      local_ac0 = 0;
    }
    else {
      local_ac0 = param_1[10] + 0x24;
    }
    (**(code **)(*(int *)param_1[0xa2ee] + 0x50))(local_ac0);
  }
  if (DAT_013b722c == 1) {
    (**(code **)(*(int *)param_1[10] + 0x40))(param_1[0x9ecb]);
  }
  FUN_0044e896();
  if (DAT_005b892c == 2) {
    (**(code **)(*(int *)param_1[0x18] + 0x70))
              (*(float *)(param_1[0x9eca] + 0x4c) + _DAT_005a40f0,
               *(float *)(param_1[0x9eca] + 0x50) + _DAT_005a4104);
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2201);
  param_1[0xa269] = iVar3;
  if (param_1[0xa269] != 0) {
    for (local_510 = 0; local_510 < 100; local_510 = local_510 + 1) {
      pvVar2 = operator_new(0x208);
      local_8 = 0x3d;
      if (pvVar2 == (void *)0x0) {
        local_acc = 0;
      }
      else {
        local_acc = FUN_00402f01(0x177,(float)((local_510 % 10) * 0x19) + _DAT_005a40f0,
                                 (float)((local_510 / 10) * 0x19) + _DAT_005a40f0,0x41c00000,
                                 0x41c00000,0x77777777,1,&DAT_005d0424);
      }
      local_8 = 0xffffffff;
      param_1[local_510 + 0xa26a] = local_acc;
      (**(code **)(*(int *)param_1[local_510 + 0xa26a] + 0x44))(local_510 + 0x2202);
      if (param_1[10] == 0) {
        local_ad0 = 0;
      }
      else {
        local_ad0 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[local_510 + 0xa26a] + 0x50))(local_ad0);
      FUN_0054ac09(param_1[local_510 + 0xa26a]);
    }
    *(undefined4 *)(param_1[0xa269] + 0x60) = 1;
    *(int *)(param_1[10] + 0x148) = param_1[0xa269];
    (**(code **)(*(int *)param_1[0xa269] + 0x60))(0);
    if (DAT_005b892c == 2) {
      (**(code **)(*(int *)param_1[0xa269] + 0x70))
                (((float)*(uint *)(DAT_005ccf9c + 0x2a504) - *(float *)(param_1[0xa269] + 0x54)) /
                 _DAT_005a365c,
                 ((float)*(uint *)(DAT_005ccf9c + 0x2a508) - *(float *)(param_1[0xa269] + 0x58)) /
                 _DAT_005a365c);
    }
  }
  iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2301);
  param_1[0xa2d1] = iVar3;
  if (param_1[0xa2d1] != 0) {
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2304);
    param_1[0xa2d2] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2312);
    param_1[0xa2d3] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2306);
    param_1[0xa2d4] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2303);
    param_1[0xa2da] = iVar3;
    *(undefined4 *)(param_1[0xa2da] + 0x1050) = 3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x2308);
    param_1[0xa2d5] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x230b);
    param_1[0xa2d6] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x230c);
    param_1[0xa2d7] = iVar3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x230d);
    param_1[0xa2d8] = iVar3;
    *(undefined4 *)(param_1[0xa2d8] + 0x1050) = 3;
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x230e);
    param_1[0xa2d9] = iVar3;
    *(undefined4 *)(param_1[0xa2d9] + 0x1050) = 3;
    *(undefined4 *)(param_1[0xa2d1] + 0x60) = 1;
    *(int *)(param_1[10] + 0x14c) = param_1[0xa2d1];
    (**(code **)(*(int *)param_1[0xa2d1] + 0x60))(0);
    if (DAT_005b892c == 2) {
      (**(code **)(*(int *)param_1[0xa2d1] + 0x70))
                (((float)*(uint *)(DAT_005ccf9c + 0x2a504) - *(float *)(param_1[0xa2d1] + 0x54)) /
                 _DAT_005a365c,
                 ((float)*(uint *)(DAT_005ccf9c + 0x2a508) - *(float *)(param_1[0xa2d1] + 0x58)) /
                 _DAT_005a365c);
    }
  }
  FUN_00558290(s_UI_TOTOGame_csv_005b7a30);
  FUN_0044eecf();
  _memset(param_1 + 0xa1e0,0,0x24);
  iVar3 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x9d36] = iVar3;
  iVar3 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x9d37] = iVar3;
  param_1[0x9d38] = 0;
  FUN_004bab41();
  local_320 = *(int *)(DAT_013b71e8 + 0x1b084);
  *(undefined4 *)(local_320 + 0xc0) = 0x41700000;
  if (DAT_005b892c == 2) {
    if (param_1[0x9f63] != 0) {
      (**(code **)(*(int *)param_1[0x9f63] + 0x60))(0);
    }
    for (local_31c = 0; local_31c < 0x10; local_31c = local_31c + 1) {
      pvVar2 = operator_new(0x1e4);
      local_8 = 0x3e;
      if (pvVar2 == (void *)0x0) {
        local_af4 = 0;
      }
      else {
        local_af4 = FUN_00401466(200,0,0,0x41b80000,0x41b80000,0x77777777,4);
      }
      local_8 = 0xffffffff;
      param_1[local_31c + 0xa1b4] = local_af4;
      (**(code **)(*(int *)param_1[local_31c + 0xa1b4] + 0x44))(local_31c + 0x3206);
      if (param_1[10] == 0) {
        local_af8 = 0;
      }
      else {
        local_af8 = param_1[10] + 0x24;
      }
      (**(code **)(*(int *)param_1[local_31c + 0xa1b4] + 0x50))(local_af8);
      (**(code **)(*(int *)param_1[10] + 0x44))(param_1[local_31c + 0xa1b4]);
    }
    iVar3 = (**(code **)(*(int *)param_1[10] + 0x48))(0x3222);
    param_1[0xa1c4] = iVar3;
    FUN_0040137c();
    *(float *)(param_1[0xa1c4] + 0x4c) = *(float *)(param_1[0xa1c4] + 0x4c) - _DAT_005a3c68;
    *(float *)(param_1[0xa1c4] + 0x50) = *(float *)(param_1[0x9e47] + 0x50) - _DAT_005a4314;
  }
  FUN_00431159(s_>>_Init_Field_Scene__End_005b7a40);
  ExceptionList = local_10;
  return 1;
}

