// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004974ec | Name: FUN_004974ec


undefined4 __thiscall FUN_004974ec(int param_1,undefined4 param_2)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  int local_3f8;
  int *local_3f4;
  int local_3f0;
  int *local_3ec;
  int local_3e8;
  int *local_3e4;
  int local_3e0;
  int *local_3dc;
  int *local_3d8;
  int local_3d4;
  int *local_3d0;
  int *local_3cc;
  int local_3c8;
  int *local_3c4;
  int *local_3c0;
  undefined4 local_36c;
  int local_368;
  undefined4 local_364;
  undefined4 local_360;
  undefined4 local_35c;
  int local_358;
  int local_354;
  int local_350;
  int local_34c;
  undefined4 local_348;
  undefined4 local_344;
  undefined4 local_340;
  undefined4 local_33c;
  int *local_338;
  undefined4 local_334;
  int *local_330;
  undefined4 local_32c;
  int local_328;
  undefined4 local_324;
  int local_320;
  int local_31c;
  int local_318;
  int local_314;
  undefined4 local_310;
  undefined4 local_30c;
  int *local_308;
  undefined4 local_304;
  int local_300;
  undefined4 local_2fc;
  undefined4 local_2f8;
  undefined4 local_2f4;
  int local_2f0;
  int local_2ec;
  int local_2e8;
  int local_2e4;
  undefined4 local_2e0;
  undefined4 local_2dc;
  undefined4 local_2d8;
  undefined4 local_2d4;
  undefined4 local_2d0;
  int local_2cc;
  undefined4 local_2c8;
  int local_2c4;
  int local_2c0;
  int local_2bc;
  int local_2b8;
  undefined4 local_2b4;
  undefined4 local_2b0;
  undefined4 local_2ac;
  undefined4 local_2a8;
  undefined4 local_2a4;
  undefined4 local_2a0;
  undefined4 local_29c;
  undefined1 local_298 [128];
  int *local_218;
  undefined4 local_214;
  int local_210;
  int *local_20c;
  undefined1 local_208 [128];
  undefined4 local_188;
  int local_184;
  undefined4 local_180;
  int local_17c;
  int local_178;
  int local_174;
  int local_170;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  int local_158;
  undefined4 local_154;
  undefined4 local_150;
  int local_14c;
  undefined4 local_148;
  int local_144;
  int local_140;
  int local_13c;
  int local_138;
  undefined4 local_134;
  undefined4 local_130;
  int local_12c;
  int local_128;
  undefined1 local_124 [128];
  int *local_a4;
  int local_a0;
  int *local_9c;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  int local_8c;
  int local_88;
  int local_84;
  int local_80;
  undefined4 local_7c;
  int *local_78;
  undefined4 local_74;
  int local_70;
  undefined4 local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  int local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  int *local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0102;
  local_10 = ExceptionList;
  local_14 = 0;
  ExceptionList = &local_10;
  local_14 = FUN_0058f716(param_2,&DAT_005b8d00);
  if (local_14 == 0) {
    uVar3 = 0;
  }
  else {
    local_1c = 0;
    local_18 = 0;
    while (local_18 = FUN_00590d68(&local_1c,4,1,local_14), local_18 != 0) {
      if (local_1c == 1) {
        iVar1 = FUN_00590d68(&local_48,0x28,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8d04,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(0x1e4);
        local_8 = 0;
        if (pvVar2 == (void *)0x0) {
          local_3c0 = (int *)0x0;
        }
        else {
          local_3c0 = (int *)FUN_00401466(local_40,(float)local_3c,(float)local_38,(float)local_34,
                                          (float)local_30,local_2c,local_28);
        }
        local_8 = 0xffffffff;
        local_20 = local_3c0;
        if (local_3c0 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8d2c,local_48,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3c0 + 0x44))(local_48);
        (**(code **)(*local_20 + 0x7c))
                  (local_48,(float)local_3c,(float)local_38,(float)local_34,(float)local_30);
        if (local_44 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_20);
        }
        else {
          local_4c = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_44);
          FUN_0054ac09(local_20);
        }
        if (DAT_005b892c == 2) {
          local_20[0x77] = local_24;
        }
      }
      else if (local_1c == 0x10) {
        iVar1 = FUN_00590d68(&local_74,0x28,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8d50,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(0x594);
        local_8 = 1;
        if (pvVar2 == (void *)0x0) {
          local_3c4 = (int *)0x0;
        }
        else {
          local_3c4 = (int *)FUN_0040df9e(local_6c,local_58,local_54,(float)local_68,(float)local_64
                                          ,(float)local_60,(float)local_5c,local_50);
        }
        local_8 = 0xffffffff;
        local_78 = local_3c4;
        if (local_3c4 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8d78,local_74,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3c4 + 0x44))(local_74);
        (**(code **)(*local_78 + 0x7c))
                  (local_74,(float)local_68,(float)local_64,(float)local_60,(float)local_5c);
        if (*(int *)(param_1 + 0x28) == 0) {
          local_3c8 = 0;
        }
        else {
          local_3c8 = *(int *)(param_1 + 0x28) + 0x24;
        }
        (**(code **)(*local_78 + 0x50))(local_3c8);
        if (local_70 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_78);
        }
        else {
          local_7c = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_70);
          FUN_0054ac09(local_78);
        }
      }
      else if (local_1c == 0xf) {
        iVar1 = FUN_00590d68(&local_98,0x1c,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8d9c,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(0x1d0);
        local_8 = 2;
        if (pvVar2 == (void *)0x0) {
          local_3cc = (int *)0x0;
        }
        else {
          local_3cc = (int *)FUN_00401ac0(local_90,(float)local_8c,(float)local_88,(float)local_84,
                                          (float)local_80);
        }
        local_8 = 0xffffffff;
        local_9c = local_3cc;
        if (local_3cc == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8dc4,local_98,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3cc + 0x44))(local_98);
        (**(code **)(*local_9c + 0x7c))
                  (local_98,(float)local_8c,(float)local_88,(float)local_84,(float)local_80);
        if (local_94 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_9c);
        }
        else {
          local_a0 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_94);
          if (local_a0 != 0) {
            FUN_0054ac09(local_9c);
          }
        }
      }
      else if (local_1c == 2) {
        iVar1 = FUN_00590d68(&local_150,0x28,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8de8,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        local_128 = local_12c;
        FUN_0058f078(local_124,&DAT_005b8e10,&DAT_00e38540 + local_12c * 0x40);
        pvVar2 = operator_new(0x208);
        local_8 = 3;
        if (pvVar2 == (void *)0x0) {
          local_3d0 = (int *)0x0;
        }
        else {
          local_3d0 = (int *)FUN_00402f01(local_148,(float)local_144,(float)local_140,
                                          (float)local_13c,(float)local_138,local_134,local_130,
                                          local_124);
        }
        local_8 = 0xffffffff;
        local_a4 = local_3d0;
        if (local_3d0 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8e14,local_150,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3d0 + 0x44))(local_150);
        (**(code **)(*local_a4 + 0x7c))
                  (local_150,(float)local_144,(float)local_140,(float)local_13c,(float)local_138);
        if (local_14c == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_a4);
        }
        else {
          local_154 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_14c);
          FUN_0054ac09(local_a4);
        }
        if (*(int *)(param_1 + 0x28) == 0) {
          local_3d4 = 0;
        }
        else {
          local_3d4 = *(int *)(param_1 + 0x28) + 0x24;
        }
        (**(code **)(*local_a4 + 0x50))(local_3d4);
      }
      else if (local_1c == 0xc) {
        iVar1 = FUN_00590d68(&local_188,0x34,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8e38,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        local_210 = local_158;
        FUN_0058f078(local_208,&DAT_005b8e60,&DAT_00e38540 + local_158 * 0x40);
        pvVar2 = operator_new(0xe4c);
        local_8 = 4;
        if (pvVar2 == (void *)0x0) {
          local_3d8 = (int *)0x0;
        }
        else {
          local_3d8 = (int *)FUN_00401d03(local_180,local_208,local_16c,(float)local_17c,
                                          (float)local_178,(float)local_174,(float)local_170,
                                          local_168,local_164,local_160,local_15c);
        }
        local_8 = 0xffffffff;
        local_20c = local_3d8;
        if (local_3d8 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8e64,local_188,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3d8 + 0x44))(local_188);
        (**(code **)(*local_20c + 0x7c))
                  (local_188,(float)local_17c,(float)local_178,(float)local_174,(float)local_170);
        if (local_184 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_20c);
        }
        else {
          local_214 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_184);
          FUN_0054ac09(local_20c);
        }
      }
      else if (local_1c == 0xd) {
        iVar1 = FUN_00590d68(&local_2d0,0xb8,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8e88,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(0x1064);
        local_8 = 5;
        if (pvVar2 == (void *)0x0) {
          local_3dc = (int *)0x0;
        }
        else {
          local_3dc = (int *)FUN_004067e9(local_2c8,local_298,local_2a0,local_29c,local_2b4,
                                          (float)local_2c4,(float)local_2c0,(float)local_2bc,
                                          (float)local_2b8,local_2b0,local_2ac,local_2a8,local_2a4);
        }
        local_8 = 0xffffffff;
        local_218 = local_3dc;
        if (local_3dc == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8eb0,local_2d0,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3dc + 0x44))(local_2d0);
        (**(code **)(*local_218 + 0x7c))
                  (local_2d0,(float)local_2c4,(float)local_2c0,(float)local_2bc,(float)local_2b8);
        if (local_2cc == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_218);
        }
        else {
          local_2d4 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_2cc);
          FUN_0054ac09(local_218);
        }
        if (*(int *)(param_1 + 0x28) == 0) {
          local_3e0 = 0;
        }
        else {
          local_3e0 = *(int *)(param_1 + 0x28) + 0x24;
        }
        (**(code **)(*local_218 + 0x50))(local_3e0);
      }
      else if (local_1c == 10) {
        iVar1 = FUN_00590d68(&local_304,0x30,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8ed4,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(0x374);
        local_8 = 6;
        if (pvVar2 == (void *)0x0) {
          local_3e4 = (int *)0x0;
        }
        else {
          local_3e4 = (int *)FUN_00405065(local_2fc,local_2f8,local_2f4,(float)local_2f0,
                                          (float)local_2ec,(float)local_2e8,(float)local_2e4,
                                          local_2e0,local_2dc,local_2d8);
        }
        local_8 = 0xffffffff;
        local_308 = local_3e4;
        if (local_3e4 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8efc,local_304,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3e4 + 0x44))(local_304);
        (**(code **)(*local_308 + 0x7c))
                  (local_304,(float)local_2f0,(float)local_2ec,(float)local_2e8,(float)local_2e4);
        if (local_300 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_308);
        }
        else {
          local_30c = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_300);
          FUN_0054ac09(local_308);
        }
        if (*(int *)(param_1 + 0x28) == 0) {
          local_3e8 = 0;
        }
        else {
          local_3e8 = *(int *)(param_1 + 0x28) + 0x24;
        }
        (**(code **)(*local_308 + 0x50))(local_3e8);
      }
      else if (local_1c == 3) {
        iVar1 = FUN_00590d68(&local_32c,0x20,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8f20,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(500);
        local_8 = 7;
        if (pvVar2 == (void *)0x0) {
          local_3ec = (int *)0x0;
        }
        else {
          local_3ec = (int *)FUN_00409ab6(local_324,(float)local_320,(float)local_31c,
                                          (float)local_318,(float)local_314,local_310);
        }
        local_8 = 0xffffffff;
        local_330 = local_3ec;
        if (local_3ec == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8f48,local_32c,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3ec + 0x44))(local_32c);
        (**(code **)(*local_330 + 0x7c))
                  (local_32c,(float)local_320,(float)local_31c,(float)local_318,(float)local_314);
        if (local_328 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_330);
        }
        else {
          local_334 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_328);
          FUN_0054ac09(local_330);
        }
        if (*(int *)(param_1 + 0x28) == 0) {
          local_3f0 = 0;
        }
        else {
          local_3f0 = *(int *)(param_1 + 0x28) + 0x24;
        }
        (**(code **)(*local_330 + 0x50))(local_3f0);
      }
      else {
        if (local_1c != 6) {
          FUN_00431159(s_Not_Support_Control_Type___d__005b8fb8,local_1c);
          ExceptionList = local_10;
          return 0;
        }
        iVar1 = FUN_00590d68(&local_36c,0x34,1,local_14);
        if (iVar1 == 0) {
          FUN_00431159(s_Can_t_Read_Resource_Data__d__in___005b8f6c,local_1c,param_2);
          FUN_0058f62c(local_14);
          ExceptionList = local_10;
          return 0;
        }
        pvVar2 = operator_new(0x3a4);
        local_8 = 8;
        if (pvVar2 == (void *)0x0) {
          local_3f4 = (int *)0x0;
        }
        else {
          local_3f4 = (int *)FUN_004087c4(local_364,local_360,local_35c,(float)local_358,
                                          (float)local_354,(float)local_350,(float)local_34c,
                                          local_348,local_344,local_340,local_33c,0);
        }
        local_8 = 0xffffffff;
        local_338 = local_3f4;
        if (local_3f4 == (int *)0x0) {
          uVar3 = FUN_00494dcf();
          FUN_00431159(s_Can_t_Create___d__in_Scene___d__005b8f94,local_36c,uVar3);
          ExceptionList = local_10;
          return 0;
        }
        (**(code **)(*local_3f4 + 0x44))(local_36c);
        (**(code **)(*local_338 + 0x7c))
                  (local_36c,(float)local_358,(float)local_354,(float)local_350,(float)local_34c);
        if (local_368 == 0) {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_338);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_368);
          FUN_0054ac09(local_338);
        }
        if (*(int *)(param_1 + 0x28) == 0) {
          local_3f8 = 0;
        }
        else {
          local_3f8 = *(int *)(param_1 + 0x28) + 0x24;
        }
        (**(code **)(*local_338 + 0x50))(local_3f8);
      }
    }
    FUN_0058f62c(local_14);
    uVar3 = 1;
  }
  ExceptionList = local_10;
  return uVar3;
}

