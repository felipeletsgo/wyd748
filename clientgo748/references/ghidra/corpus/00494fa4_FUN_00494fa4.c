// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00494fa4 | Name: FUN_00494fa4


/* WARNING: Type propagation algorithm not settling */

undefined4 __thiscall FUN_00494fa4(int *param_1,undefined4 param_2,int param_3,char *param_4)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  size_t sVar4;
  int local_8a0;
  int *local_89c;
  int local_898;
  int *local_894;
  int local_890;
  int *local_88c;
  int local_888;
  int *local_884;
  int local_880;
  int *local_87c;
  int *local_878;
  int local_874;
  int *local_870;
  int *local_86c;
  int *local_868;
  int local_814;
  int *local_810;
  int local_80c;
  undefined4 local_808;
  int local_804;
  int local_800 [5];
  int local_7ec;
  undefined4 local_7e8;
  int local_7e4;
  int local_7e0;
  int local_7dc;
  int local_7d8;
  undefined4 local_7d4;
  undefined4 local_7d0;
  undefined4 local_7cc;
  undefined4 local_7c8;
  undefined4 local_7c4;
  undefined1 local_7c0 [48];
  int local_790;
  undefined1 local_78c [48];
  undefined4 local_75c;
  char local_758 [4];
  uint local_754;
  undefined4 local_750;
  int local_74c;
  undefined4 local_748;
  undefined4 local_744;
  int local_740;
  int local_73c;
  int local_738;
  undefined4 local_734;
  undefined4 local_730;
  int local_72c;
  char local_728 [4];
  int local_724;
  int local_720;
  undefined4 local_71c;
  undefined4 local_718;
  undefined4 local_714;
  int local_710;
  int local_70c;
  int local_708;
  int local_704;
  undefined4 local_700;
  undefined4 local_6fc;
  uint local_6f8;
  undefined4 local_6f4;
  undefined4 local_6f0;
  undefined1 local_6ec [48];
  char local_6bc [4];
  int local_6b8;
  undefined1 local_6b4 [48];
  undefined4 local_684;
  int *local_680;
  undefined4 local_67c;
  int *local_678;
  int local_674;
  undefined4 local_670;
  int local_66c;
  int local_668;
  int local_664;
  undefined4 local_660;
  int local_65c;
  undefined4 local_658;
  undefined1 local_654 [48];
  int local_624;
  int local_620;
  undefined4 local_61c;
  int local_618;
  int local_614;
  int local_610;
  int local_60c;
  undefined4 local_608;
  int local_604;
  undefined1 local_600 [48];
  undefined4 local_5d0;
  undefined4 local_5cc;
  undefined4 local_5c8;
  int local_5c4;
  undefined4 local_5c0;
  int *local_5bc;
  int local_5b8;
  int local_5b4;
  undefined4 local_5b0;
  undefined4 local_5ac;
  undefined4 local_5a8;
  int local_5a4;
  int local_5a0;
  int local_59c;
  int local_598;
  undefined4 local_594;
  undefined4 local_590;
  undefined4 local_58c;
  int local_588;
  int local_584;
  int local_580;
  undefined4 local_57c;
  int local_578;
  undefined4 local_574;
  undefined1 local_570 [48];
  undefined4 local_540;
  undefined4 local_53c;
  int local_538;
  undefined1 local_534 [48];
  undefined4 local_504;
  undefined4 local_500;
  int local_4fc;
  char local_4f8 [4];
  char local_4f4 [128];
  int local_474;
  int local_470;
  undefined4 local_46c;
  int local_468;
  undefined4 local_464;
  int local_460;
  uint local_45c;
  undefined4 local_458;
  char local_454 [4];
  int local_450;
  int local_44c;
  undefined4 local_448;
  int local_444;
  int local_440;
  int local_43c;
  int local_438;
  undefined4 local_434;
  uint local_430;
  undefined4 local_42c;
  undefined4 local_428;
  undefined4 local_424;
  undefined4 local_420;
  uint local_41c;
  undefined1 local_418 [128];
  undefined4 local_398;
  char local_394 [4];
  uint local_390;
  undefined1 local_38c [48];
  char local_35c [4];
  undefined4 local_358;
  undefined4 local_354;
  int local_350;
  undefined1 local_34c [48];
  int *local_31c;
  undefined4 local_318;
  int local_314;
  int *local_310;
  int local_30c;
  char local_308 [4];
  int local_304;
  int local_300;
  undefined4 local_2fc;
  int local_2f8;
  undefined4 local_2f4;
  int local_2f0;
  undefined4 local_2ec;
  char local_2e8 [4];
  undefined4 local_2e4;
  char local_2e0 [4];
  uint local_2dc;
  undefined1 local_2d8 [48];
  int local_2a8;
  int local_2a4;
  undefined4 local_2a0;
  int local_29c;
  int local_298;
  int local_294;
  int local_290;
  undefined4 local_28c;
  uint local_288;
  undefined4 local_284;
  undefined4 local_280;
  undefined4 local_27c;
  int local_278;
  undefined4 local_274;
  undefined4 local_270;
  int local_26c;
  undefined1 local_268 [48];
  undefined4 local_238;
  int local_234;
  int local_230;
  undefined4 local_22c;
  int local_228;
  int local_224;
  int local_220;
  int local_21c;
  undefined4 local_218;
  uint local_214;
  int local_210;
  char local_20c [4];
  int local_208;
  uint local_204;
  int local_200;
  undefined4 local_1fc;
  int local_1f8;
  int local_1f4;
  int local_1f0;
  int *local_1ec;
  undefined4 local_1e8;
  int local_1e4;
  undefined4 local_1e0;
  undefined1 local_1dc [48];
  int local_1ac;
  undefined1 local_1a8 [48];
  int local_178;
  int local_174;
  undefined4 local_170;
  int local_16c;
  int local_168;
  int local_164;
  int *local_160;
  undefined4 local_15c;
  int local_158;
  undefined1 local_154 [48];
  int local_124;
  int local_120;
  undefined4 local_11c;
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  undefined1 local_104 [48];
  undefined4 local_d4;
  undefined4 local_d0;
  char local_cc [4];
  int local_c8;
  undefined4 local_c4;
  int local_c0;
  int local_bc;
  int local_b8;
  undefined4 local_b4;
  char local_b0 [4];
  int local_ac;
  int local_a8;
  int local_a4;
  undefined4 local_a0;
  int local_9c;
  int local_98;
  int local_94;
  int local_90;
  undefined4 local_8c;
  undefined4 local_88;
  int local_84;
  undefined4 local_80;
  undefined1 local_7c [48];
  int *local_4c;
  int local_48;
  undefined1 local_44 [48];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a007a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = _strcmp(param_4,s__PANEL__005b89e4);
  if (iVar1 == 0) {
    local_b0[0] = 'I';
    local_80 = 0;
    local_c0 = 0;
    local_ac = 0;
    local_bc = 0;
    local_c8 = 0;
    local_c4 = 0;
    local_48 = 0;
    local_b8 = 0;
    local_cc[0] = 'F';
    FUN_00590a68(param_2,&DAT_005b89ec,local_7c);
    FUN_00590a68(param_2,&DAT_005b89f0,local_44);
    local_48 = (**(code **)(*param_1 + 0x48))(local_7c);
    if (local_48 == 0) {
      ExceptionList = local_10;
      return 0;
    }
    FUN_00590a68(param_2,s__d__d__d__d__d__x__c__c_005b89f4,&local_80,&local_c0,&local_ac,&local_bc,
                 &local_c8,&local_c4,local_b0,local_cc);
    if (local_b0[0] == 'T') {
      local_14 = 3;
    }
    else if (local_b0[0] == 'S') {
      local_14 = 4;
    }
    else {
      local_14 = 2;
    }
    pvVar2 = operator_new(0x1e4);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_868 = (int *)0x0;
    }
    else {
      local_868 = (int *)FUN_00401466(local_80,(float)local_c0,(float)local_ac,(float)local_bc,
                                      (float)local_c8,local_c4,local_14);
    }
    local_8 = 0xffffffff;
    local_4c = local_868;
    if (local_868 == (int *)0x0) {
      uVar3 = FUN_00494dcf();
      FUN_00431159(s_Can_t_Create_Panel___s__in_Scene_005b8a0c,local_7c,uVar3);
      ExceptionList = local_10;
      return 0;
    }
    (**(code **)(*local_868 + 0x44))(local_48);
    (**(code **)(*local_4c + 0x7c))
              (local_48,(float)local_c0,(float)local_ac,(float)local_bc,(float)local_c8);
    if (local_cc[0] == 'T') {
      local_4c[0x77] = 1;
    }
    local_b8 = (**(code **)(*param_1 + 0x48))(local_44);
    if (local_b8 == 0) {
      (**(code **)(*(int *)param_1[10] + 0x44))(local_4c);
    }
    else {
      local_d0 = (**(code **)(*(int *)param_1[10] + 0x48))(local_b8);
      FUN_0054ac09(local_4c);
    }
    local_b4 = 1;
    local_a8 = local_48;
    local_a4 = local_b8;
    local_a0 = local_80;
    local_9c = local_c0;
    local_98 = local_ac;
    local_94 = local_bc;
    local_90 = local_c8;
    local_8c = local_c4;
    local_88 = local_14;
    local_84 = local_4c[0x77];
    if (param_3 != 0) {
      FUN_005904b6(&local_b4,4,1,param_3);
      FUN_005904b6(&local_a8,0x28,1,param_3);
    }
  }
  else {
    iVar1 = _strcmp(param_4,s__3DOBJ__005b8a34);
    if (iVar1 == 0) {
      local_170 = 0;
      local_16c = 0;
      local_158 = 0;
      local_168 = 0;
      local_174 = 0;
      local_108 = 0;
      local_164 = 0;
      FUN_00590a68(param_2,&DAT_005b8a3c,local_154);
      FUN_00590a68(param_2,&DAT_005b8a40,local_104);
      local_108 = (**(code **)(*param_1 + 0x48))(local_154);
      if (local_108 == 0) {
        ExceptionList = local_10;
        return 0;
      }
      FUN_00590a68(param_2,s__d__d__d__d__d_005b8a44,&local_170,&local_16c,&local_158,&local_168,
                   &local_174);
      local_d4 = 5;
      pvVar2 = operator_new(0x1d0);
      local_8 = 1;
      if (pvVar2 == (void *)0x0) {
        local_86c = (int *)0x0;
      }
      else {
        local_86c = (int *)FUN_00401ac0(local_170,(float)local_16c,(float)local_158,(float)local_168
                                        ,(float)local_174);
      }
      local_8 = 0xffffffff;
      local_160 = local_86c;
      if (local_86c == (int *)0x0) {
        uVar3 = FUN_00494dcf();
        FUN_00431159(s_Can_t_Create_3DObject___s__in_Sc_005b8a54,local_154,uVar3);
        ExceptionList = local_10;
        return 0;
      }
      (**(code **)(*local_86c + 0x44))(local_108);
      (**(code **)(*local_160 + 0x7c))
                (local_108,(float)local_16c,(float)local_158,(float)local_168,(float)local_174);
      local_164 = (**(code **)(*param_1 + 0x48))(local_104);
      if (local_164 == 0) {
        (**(code **)(*(int *)param_1[10] + 0x44))(local_160);
      }
      else {
        local_178 = (**(code **)(*(int *)param_1[10] + 0x48))(local_164);
        if (local_178 != 0) {
          FUN_0054ac09(local_160);
        }
      }
      local_15c = 0xf;
      local_124 = local_108;
      local_120 = local_164;
      local_11c = local_170;
      local_118 = local_16c;
      local_114 = local_158;
      local_110 = local_168;
      local_10c = local_174;
      if (param_3 != 0) {
        FUN_005904b6(&local_15c,4,1,param_3);
        FUN_005904b6(&local_124,0x1c,1,param_3);
      }
    }
    else {
      iVar1 = _strcmp(param_4,s__BUTTON__005b8a80);
      if (iVar1 == 0) {
        local_204 = 0;
        local_1e0 = 0;
        local_1f8 = 0;
        local_1e4 = 0;
        local_1f4 = 0;
        local_200 = 0;
        local_1fc = 0;
        local_20c[0] = '\0';
        local_1ac = 0;
        local_1f0 = 0;
        local_208 = 0;
        FUN_00590a68(param_2,&DAT_005b8a8c,local_1dc);
        FUN_00590a68(param_2,&DAT_005b8a90,local_1a8);
        local_1ac = (**(code **)(*param_1 + 0x48))(local_1dc);
        if (local_1ac == 0) {
          ExceptionList = local_10;
          return 0;
        }
        FUN_00590a68(param_2,s__d__d__d__d__d__x__c__d_005b8a94,&local_1e0,&local_1f8,&local_1e4,
                     &local_1f4,&local_200,&local_1fc,local_20c,&local_208);
        local_204 = (uint)(local_20c[0] == 'S');
        pvVar2 = operator_new(0x208);
        local_8 = 2;
        if (pvVar2 == (void *)0x0) {
          local_870 = (int *)0x0;
        }
        else {
          local_870 = (int *)FUN_00402f01(local_1e0,(float)local_1f8,(float)local_1e4,
                                          (float)local_1f4,(float)local_200,local_1fc,local_204,
                                          &DAT_00e38540 + local_208 * 0x40);
        }
        local_8 = 0xffffffff;
        local_1ec = local_870;
        if (local_870 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create_pButton___s__in_Sce_005b8aac,local_1dc,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_870 + 0x44))(local_1ac);
        (**(code **)(*local_1ec + 0x7c))
                  (local_1ac,(float)local_1f8,(float)local_1e4,(float)local_1f4,(float)local_200);
        local_1f0 = (**(code **)(*param_1 + 0x48))(local_1a8);
        if (local_1f0 == 0) {
          (**(code **)(*(int *)param_1[10] + 0x44))(local_1ec);
        }
        else {
          local_238 = (**(code **)(*(int *)param_1[10] + 0x48))(local_1f0);
          FUN_0054ac09(local_1ec);
        }
        if (param_1[10] == 0) {
          local_874 = 0;
        }
        else {
          local_874 = param_1[10] + 0x24;
        }
        (**(code **)(*local_1ec + 0x50))(local_874);
        local_1e8 = 2;
        local_234 = local_1ac;
        local_230 = local_1f0;
        local_22c = local_1e0;
        local_228 = local_1f8;
        local_224 = local_1e4;
        local_220 = local_1f4;
        local_21c = local_200;
        local_218 = local_1fc;
        local_214 = local_204;
        local_210 = local_208;
        if (param_3 != 0) {
          FUN_005904b6(&local_1e8,4,1,param_3);
          FUN_005904b6(&local_234,0x28,1,param_3);
        }
      }
      else {
        iVar1 = _strcmp(param_4,s__TEXT__005b8ad8);
        if (iVar1 == 0) {
          local_2e4 = 0;
          local_304 = 0;
          local_2f0 = 0;
          local_300 = 0;
          local_30c = 0;
          local_2ec = 0;
          local_2fc = 0;
          local_308[0] = '\0';
          local_2e8[0] = '\0';
          local_2e0[0] = '\0';
          local_314 = 0;
          local_2dc = 0;
          local_274 = 0;
          local_270 = 1;
          local_26c = 0;
          local_2f8 = 0;
          FUN_00590a68(param_2,&DAT_005b8ae0,local_2d8);
          FUN_00590a68(param_2,&DAT_005b8ae4,local_268);
          local_26c = (**(code **)(*param_1 + 0x48))(local_2d8);
          if (local_26c == 0) {
            ExceptionList = local_10;
            return 0;
          }
          FUN_00590a68(param_2,s__d__d__d__d__d__x__c__x__c__c__d_005b8ae8,&local_2e4,&local_304,
                       &local_2f0,&local_300,&local_30c,&local_2ec,local_308,&local_2fc,local_2e8,
                       local_2e0,&local_314);
          local_2dc = (uint)(local_308[0] == 'B');
          if (local_2e8[0] == 'S') {
            local_274 = 1;
          }
          else if (local_2e8[0] == 'T') {
            local_274 = 2;
          }
          else {
            local_274 = 0;
          }
          if (local_2e0[0] == 'L') {
            local_270 = 0;
          }
          else if (local_2e0[0] == 'N') {
            local_270 = 3;
          }
          else if (local_2e0[0] == 'R') {
            local_270 = 2;
          }
          else {
            local_270 = 1;
          }
          pvVar2 = operator_new(0xe4c);
          local_8 = 3;
          if (pvVar2 == (void *)0x0) {
            local_878 = (int *)0x0;
          }
          else {
            local_878 = (int *)FUN_00401d03(local_2e4,&DAT_00e38540 + local_314 * 0x40,local_2ec,
                                            (float)local_304,(float)local_2f0,(float)local_300,
                                            (float)local_30c,local_2dc,local_2fc,local_274,local_270
                                           );
          }
          local_8 = 0xffffffff;
          local_310 = local_878;
          if (local_878 == (int *)0x0) {
            uVar3 = FUN_00494dcf();
            FUN_00431159(s_Can_t_Create_Text___s__in_Scene___005b8b0c,local_2d8,uVar3);
            ExceptionList = local_10;
            return 0;
          }
          (**(code **)(*local_878 + 0x44))(local_26c);
          (**(code **)(*local_310 + 0x7c))
                    (local_26c,(float)local_304,(float)local_2f0,(float)local_300,(float)local_30c);
          local_2f8 = (**(code **)(*param_1 + 0x48))(local_268);
          if (local_2f8 == 0) {
            (**(code **)(*(int *)param_1[10] + 0x44))(local_310);
          }
          else {
            local_318 = (**(code **)(*(int *)param_1[10] + 0x48))(local_2f8);
            FUN_0054ac09(local_310);
          }
          local_2f4 = 0xc;
          local_2a8 = local_26c;
          local_2a4 = local_2f8;
          local_2a0 = local_2e4;
          local_29c = local_304;
          local_298 = local_2f0;
          local_294 = local_300;
          local_290 = local_30c;
          local_28c = local_2ec;
          local_288 = local_2dc;
          local_284 = local_2fc;
          local_280 = local_274;
          local_27c = local_270;
          local_278 = local_314;
          if (param_3 != 0) {
            FUN_005904b6(&local_2f4,4,1,param_3);
            FUN_005904b6(&local_2a8,0x34,1,param_3);
          }
        }
        else {
          iVar1 = _strcmp(param_4,s__EDIT__005b8b34);
          if (iVar1 == 0) {
            local_398 = 0;
            local_474 = 0;
            local_460 = 0;
            local_470 = 0;
            local_4fc = 0;
            local_458 = 0;
            local_46c = 0;
            local_500 = 0;
            local_35c[0] = '\0';
            local_4f8[0] = '\0';
            local_454[0] = '\0';
            local_394[0] = '\0';
            local_390 = 0;
            local_45c = 0;
            local_358 = 0;
            local_354 = 1;
            local_350 = 0;
            local_468 = 0;
            FUN_00590a68(param_2,&DAT_005b8b3c,local_38c);
            FUN_00590a68(param_2,&DAT_005b8b40,local_34c);
            local_350 = (**(code **)(*param_1 + 0x48))(local_38c);
            if (local_350 == 0) {
              ExceptionList = local_10;
              return 0;
            }
            FUN_00590a68(param_2,s__d__d__d__d__d__x__c__x__c__c__d_005b8b44,&local_398,&local_474,
                         &local_460,&local_470,&local_4fc,&local_458,local_4f8,&local_46c,local_454,
                         local_394,&local_500,local_35c,local_4f4);
            local_390 = (uint)(local_4f8[0] == 'B');
            if (local_454[0] == 'S') {
              local_358 = 1;
            }
            else if (local_454[0] == 'T') {
              local_358 = 2;
            }
            else {
              local_358 = 0;
            }
            if (local_394[0] == 'L') {
              local_354 = 0;
            }
            else if (local_394[0] == 'R') {
              local_354 = 2;
            }
            else {
              local_354 = 1;
            }
            local_45c = (uint)(local_35c[0] == 'P');
            pvVar2 = operator_new(0x1064);
            local_8 = 4;
            if (pvVar2 == (void *)0x0) {
              local_87c = (int *)0x0;
            }
            else {
              local_87c = (int *)FUN_004067e9(local_398,local_4f4,local_500,local_45c,local_458,
                                              (float)local_474,(float)local_460,(float)local_470,
                                              (float)local_4fc,local_390,local_46c,local_358,
                                              local_354);
            }
            local_8 = 0xffffffff;
            local_31c = local_87c;
            if (local_87c == (int *)0x0) {
              uVar3 = FUN_00494dcf();
              FUN_00431159(s_Can_t_Create_Edit___s__in_Scene___005b8b6c,local_38c,uVar3);
              ExceptionList = local_10;
              return 0;
            }
            (**(code **)(*local_87c + 0x44))(local_350);
            (**(code **)(*local_31c + 0x7c))
                      (local_350,(float)local_474,(float)local_460,(float)local_470,(float)local_4fc
                      );
            local_468 = (**(code **)(*param_1 + 0x48))(local_34c);
            if (local_468 == 0) {
              (**(code **)(*(int *)param_1[10] + 0x44))(local_31c);
            }
            else {
              local_504 = (**(code **)(*(int *)param_1[10] + 0x48))(local_468);
              FUN_0054ac09(local_31c);
            }
            if (param_1[10] == 0) {
              local_880 = 0;
            }
            else {
              local_880 = param_1[10] + 0x24;
            }
            (**(code **)(*local_31c + 0x50))(local_880);
            local_464 = 0xd;
            local_450 = local_350;
            local_44c = local_468;
            local_448 = local_398;
            local_444 = local_474;
            local_440 = local_460;
            local_43c = local_470;
            local_438 = local_4fc;
            local_434 = local_458;
            local_430 = local_390;
            local_42c = local_46c;
            local_428 = local_358;
            local_424 = local_354;
            local_420 = local_500;
            local_41c = local_45c;
            sVar4 = _strlen(local_4f4);
            if ((sVar4 == 1) && (local_4f4[0] == ' ')) {
              local_4f4[0] = '\0';
            }
            if (param_3 != 0) {
              FUN_005904b6(&local_464,4,1,param_3);
              FUN_0058f078(local_418,&DAT_005b8b94,local_4f4);
              FUN_005904b6(&local_450,0xb8,1,param_3);
            }
          }
          else {
            iVar1 = _strcmp(param_4,s__PROGRESS__005b8b98);
            if (iVar1 == 0) {
              local_574 = 0;
              local_540 = 0;
              local_5cc = 0;
              local_588 = 0;
              local_578 = 0;
              local_584 = 0;
              local_5c4 = 0;
              local_5c8 = 0;
              local_5c0 = 0;
              local_53c = 1;
              local_538 = 0;
              local_580 = 0;
              FUN_00590a68(param_2,&DAT_005b8ba4,local_570);
              FUN_00590a68(param_2,&DAT_005b8ba8,local_534);
              local_538 = (**(code **)(*param_1 + 0x48))(local_570);
              if (local_538 == 0) {
                ExceptionList = local_10;
                return 0;
              }
              FUN_00590a68(param_2,s__d__d__d__d__d__d__d__x__x__d_005b8bac,&local_574,&local_540,
                           &local_5cc,&local_588,&local_578,&local_584,&local_5c4,&local_5c8,
                           &local_5c0,&local_53c);
              pvVar2 = operator_new(0x374);
              local_8 = 5;
              if (pvVar2 == (void *)0x0) {
                local_884 = (int *)0x0;
              }
              else {
                local_884 = (int *)FUN_00405065(local_574,local_540,local_5cc,(float)local_588,
                                                (float)local_578,(float)local_584,(float)local_5c4,
                                                local_5c8,local_5c0,local_53c);
              }
              local_8 = 0xffffffff;
              local_5bc = local_884;
              if (local_884 == (int *)0x0) {
                uVar3 = FUN_00494dcf();
                FUN_00431159(s_Can_t_Create_Progress___s__in_Sc_005b8bcc,local_570,uVar3);
                ExceptionList = local_10;
                return 0;
              }
              (**(code **)(*local_884 + 0x44))(local_538);
              (**(code **)(*local_5bc + 0x7c))
                        (local_538,(float)local_588,(float)local_578,(float)local_584,
                         (float)local_5c4);
              local_580 = (**(code **)(*param_1 + 0x48))(local_534);
              if (local_580 == 0) {
                (**(code **)(*(int *)param_1[10] + 0x44))(local_5bc);
              }
              else {
                local_5d0 = (**(code **)(*(int *)param_1[10] + 0x48))(local_580);
                FUN_0054ac09(local_5bc);
              }
              if (param_1[10] == 0) {
                local_888 = 0;
              }
              else {
                local_888 = param_1[10] + 0x24;
              }
              (**(code **)(*local_5bc + 0x50))(local_888);
              local_57c = 10;
              local_5b8 = local_538;
              local_5b4 = local_580;
              local_5b0 = local_574;
              local_5ac = local_540;
              local_5a8 = local_5cc;
              local_5a4 = local_588;
              local_5a0 = local_578;
              local_59c = local_584;
              local_598 = local_5c4;
              local_594 = local_5c8;
              local_590 = local_5c0;
              local_58c = local_53c;
              if (param_3 != 0) {
                FUN_005904b6(&local_57c,4,1,param_3);
                FUN_005904b6(&local_5b8,0x30,1,param_3);
              }
            }
            else {
              iVar1 = _strcmp(param_4,s__CHECKBOX__005b8bf8);
              if (iVar1 == 0) {
                local_658 = 0;
                local_66c = 0;
                local_65c = 0;
                local_668 = 0;
                local_674 = 0;
                local_670 = 0;
                local_604 = 0;
                local_664 = 0;
                FUN_00590a68(param_2,&DAT_005b8c04,local_654);
                FUN_00590a68(param_2,&DAT_005b8c08,local_600);
                local_604 = (**(code **)(*param_1 + 0x48))(local_654);
                if (local_604 == 0) {
                  ExceptionList = local_10;
                  return 0;
                }
                FUN_00590a68(param_2,s__d__d__d__d__d__x_005b8c0c,&local_658,&local_66c,&local_65c,
                             &local_668,&local_674,&local_670);
                pvVar2 = operator_new(500);
                local_8 = 6;
                if (pvVar2 == (void *)0x0) {
                  local_88c = (int *)0x0;
                }
                else {
                  local_88c = (int *)FUN_00409ab6(local_658,(float)local_66c,(float)local_65c,
                                                  (float)local_668,(float)local_674,local_670);
                }
                local_8 = 0xffffffff;
                local_678 = local_88c;
                if (local_88c == (int *)0x0) {
                  uVar3 = FUN_00494dcf();
                  FUN_00431159(s_Can_t_Create_CheckBox___s__in_Sc_005b8c20,local_654,uVar3);
                  ExceptionList = local_10;
                  return 0;
                }
                (**(code **)(*local_88c + 0x44))(local_604);
                (**(code **)(*local_678 + 0x7c))
                          (local_604,(float)local_66c,(float)local_65c,(float)local_668,
                           (float)local_674);
                local_664 = (**(code **)(*param_1 + 0x48))(local_600);
                if (local_664 == 0) {
                  (**(code **)(*(int *)param_1[10] + 0x44))(local_678);
                }
                else {
                  local_67c = (**(code **)(*(int *)param_1[10] + 0x48))(local_664);
                  FUN_0054ac09(local_678);
                }
                if (param_1[10] == 0) {
                  local_890 = 0;
                }
                else {
                  local_890 = param_1[10] + 0x24;
                }
                (**(code **)(*local_678 + 0x50))(local_890);
                local_660 = 3;
                local_624 = local_604;
                local_620 = local_664;
                local_61c = local_658;
                local_618 = local_66c;
                local_614 = local_65c;
                local_610 = local_668;
                local_60c = local_674;
                local_608 = local_670;
                if (param_3 != 0) {
                  FUN_005904b6(&local_660,4,1,param_3);
                  FUN_005904b6(&local_624,0x20,1,param_3);
                }
              }
              else {
                iVar1 = _strcmp(param_4,s__LISTBOX__005b8c4c);
                if (iVar1 == 0) {
                  local_6f0 = 0;
                  local_730 = 0;
                  local_744 = 0;
                  local_740 = 0;
                  local_72c = 0;
                  local_73c = 0;
                  local_74c = 0;
                  local_748 = 0;
                  local_6bc[0] = '\0';
                  local_758[0] = '\0';
                  local_728[0] = '\0';
                  local_6b8 = 0;
                  local_738 = 0;
                  FUN_00590a68(param_2,&DAT_005b8c58,local_6ec);
                  FUN_00590a68(param_2,&DAT_005b8c5c,local_6b4);
                  local_6b8 = (**(code **)(*param_1 + 0x48))(local_6ec);
                  if (local_6b8 == 0) {
                    ExceptionList = local_10;
                    return 0;
                  }
                  FUN_00590a68(param_2,s__d__d__d__d__d__d__d__x__c__c__c_005b8c60,&local_6f0,
                               &local_730,&local_744,&local_740,&local_72c,&local_73c,&local_74c,
                               &local_748,local_728,local_6bc,local_758);
                  if (local_728[0] == 'T') {
                    local_684 = 3;
                  }
                  else if (local_728[0] == 'S') {
                    local_684 = 4;
                  }
                  else {
                    local_684 = 2;
                  }
                  local_754 = (uint)(local_6bc[0] == 'S');
                  if (local_758[0] == 'S') {
                    local_750 = 1;
                  }
                  else if (local_758[0] == 's') {
                    local_750 = 2;
                  }
                  else {
                    local_750 = 0;
                  }
                  pvVar2 = operator_new(0x3a4);
                  local_8 = 7;
                  if (pvVar2 == (void *)0x0) {
                    local_894 = (int *)0x0;
                  }
                  else {
                    local_894 = (int *)FUN_004087c4(local_6f0,local_730,local_744,(float)local_740,
                                                    (float)local_72c,(float)local_73c,
                                                    (float)local_74c,local_748,local_684,local_754,
                                                    local_750,0);
                  }
                  local_8 = 0xffffffff;
                  local_680 = local_894;
                  if (local_894 == (int *)0x0) {
                    uVar3 = FUN_00494dcf();
                    FUN_00431159(s_Can_t_Create_ListBox___s__in_Sce_005b8c84,local_6ec,uVar3);
                    ExceptionList = local_10;
                    return 0;
                  }
                  (**(code **)(*local_894 + 0x44))(local_6b8);
                  (**(code **)(*local_680 + 0x7c))
                            (local_6b8,(float)local_740,(float)local_72c,(float)local_73c,
                             (float)local_74c);
                  local_738 = (**(code **)(*param_1 + 0x48))(local_6b4);
                  if (local_738 == 0) {
                    (**(code **)(*(int *)param_1[10] + 0x44))(local_680);
                  }
                  else {
                    local_75c = (**(code **)(*(int *)param_1[10] + 0x48))(local_738);
                    FUN_0054ac09(local_680);
                  }
                  if (param_1[10] == 0) {
                    local_898 = 0;
                  }
                  else {
                    local_898 = param_1[10] + 0x24;
                  }
                  (**(code **)(*local_680 + 0x50))(local_898);
                  local_734 = 6;
                  local_724 = local_6b8;
                  local_720 = local_738;
                  local_71c = local_6f0;
                  local_718 = local_730;
                  local_714 = local_744;
                  local_710 = local_740;
                  local_70c = local_72c;
                  local_708 = local_73c;
                  local_704 = local_74c;
                  local_700 = local_748;
                  local_6fc = local_684;
                  local_6f8 = local_754;
                  local_6f4 = local_750;
                  if (param_3 != 0) {
                    FUN_005904b6(&local_734,4,1,param_3);
                    FUN_005904b6(&local_724,0x34,1,param_3);
                  }
                }
                else {
                  iVar1 = _strcmp(param_4,s__GRID__005b8cb0);
                  if (iVar1 == 0) {
                    local_790 = 0;
                    local_80c = 0;
                    local_7c8 = 0xffffffff;
                    local_808 = 0;
                    local_7c4 = 0;
                    local_804 = 0;
                    local_800[2] = 0;
                    local_800[0] = 0;
                    local_814 = 0;
                    local_800[3] = 0;
                    FUN_00590a68(param_2,&DAT_005b8cb8,local_7c0);
                    FUN_00590a68(param_2,&DAT_005b8cbc,local_78c);
                    local_790 = (**(code **)(*param_1 + 0x48))(local_7c0);
                    if (local_790 == 0) {
                      ExceptionList = local_10;
                      return 0;
                    }
                    FUN_00590a68(param_2,s__d__d__d__d__d__d__d__d_005b8cc0,&local_7c8,&local_808,
                                 &local_7c4,&local_804,local_800 + 2,local_800,&local_814,
                                 local_800 + 3);
                    pvVar2 = operator_new(0x594);
                    local_8 = 8;
                    if (pvVar2 == (void *)0x0) {
                      local_89c = (int *)0x0;
                    }
                    else {
                      local_89c = (int *)FUN_0040df9e(local_7c8,local_808,local_7c4,(float)local_804
                                                      ,(float)local_800[2],(float)local_800[0],
                                                      (float)local_814,local_800[3]);
                    }
                    local_8 = 0xffffffff;
                    local_810 = local_89c;
                    if (local_89c == (int *)0x0) {
                      uVar3 = FUN_00494dcf();
                      FUN_00431159(s_Can_t_Create_Grid___s__in_Scene___005b8cd8,local_7c0,uVar3);
                      ExceptionList = local_10;
                      return 0;
                    }
                    (**(code **)(*local_89c + 0x44))(local_790);
                    (**(code **)(*local_810 + 0x7c))
                              (local_790,(float)local_804,(float)local_800[2],(float)local_800[0],
                               (float)local_814);
                    if (param_1[10] == 0) {
                      local_8a0 = 0;
                    }
                    else {
                      local_8a0 = param_1[10] + 0x24;
                    }
                    (**(code **)(*local_810 + 0x50))(local_8a0);
                    local_80c = (**(code **)(*param_1 + 0x48))(local_78c);
                    if (local_80c == 0) {
                      (**(code **)(*(int *)param_1[10] + 0x44))(local_810);
                    }
                    else {
                      (**(code **)(*(int *)param_1[10] + 0x48))(local_80c);
                      FUN_0054ac09(local_810);
                    }
                    local_800[1] = 0x10;
                    local_800[4] = local_790;
                    local_7ec = local_80c;
                    local_7e8 = local_7c8;
                    local_7d4 = local_808;
                    local_7d0 = local_7c4;
                    local_7e4 = local_804;
                    local_7e0 = local_800[2];
                    local_7dc = local_800[0];
                    local_7d8 = local_814;
                    local_7cc = local_800[3];
                    if (param_3 != 0) {
                      FUN_005904b6(local_800 + 1,4,1,param_3);
                      FUN_005904b6(local_800 + 4,0x28,1,param_3);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

