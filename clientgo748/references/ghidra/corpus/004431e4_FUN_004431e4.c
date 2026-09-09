// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004431e4 | Name: FUN_004431e4


void __thiscall FUN_004431e4(int param_1,uint param_2)

{
  short sVar1;
  bool bVar2;
  undefined4 uVar3;
  void *pvVar4;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  int iVar5;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_EDX;
  undefined4 extraout_EDX_00;
  undefined4 extraout_EDX_01;
  undefined4 extraout_EDX_02;
  undefined4 extraout_EDX_03;
  undefined4 extraout_EDX_04;
  undefined4 extraout_EDX_05;
  undefined4 extraout_EDX_06;
  undefined4 extraout_EDX_07;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 in_stack_fffffa04;
  undefined4 local_5bc;
  int local_5b8;
  undefined1 local_59c [8];
  void *local_594;
  int local_590;
  undefined2 *local_58c;
  undefined1 local_588 [256];
  int local_488;
  short local_484;
  int local_480;
  int local_47c;
  int local_478;
  int local_474;
  int local_470;
  uint local_46c;
  uint local_468;
  uint local_464;
  int local_460;
  int local_45c;
  int local_458;
  undefined4 local_454;
  short *local_450;
  short *local_44c;
  int local_448;
  undefined4 *local_444;
  int *local_440;
  uint local_43c;
  short *local_438;
  int local_434;
  undefined2 *local_430;
  int *local_42c;
  uint local_428;
  int local_424;
  int local_420;
  int local_41c;
  int local_418;
  undefined1 local_414 [256];
  undefined1 local_314;
  undefined4 local_313;
  int local_214;
  int local_210;
  int local_20c;
  int local_208;
  int local_204;
  uint local_200;
  int local_1fc;
  int local_1f8;
  int local_1f4;
  int local_1f0;
  undefined1 local_1ec [64];
  undefined *local_1ac [4];
  undefined4 local_19c;
  int local_198;
  uint local_194;
  int local_190;
  int local_18c;
  uint local_188;
  int local_184;
  int local_180;
  int local_17c;
  int local_178;
  int local_174;
  int local_170;
  uint local_16c;
  undefined1 local_168 [128];
  undefined1 local_e8 [128];
  int local_68;
  uint local_64;
  uint local_60;
  int local_58;
  int local_54;
  char local_50;
  undefined4 local_4c;
  uint local_48;
  undefined1 local_44 [32];
  int local_24;
  int local_20;
  int local_1c;
  int *local_18;
  undefined4 local_14;
  void *pvStack_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f5ba;
  pvStack_10 = ExceptionList;
  local_1fc = DAT_013b71e8 + 0x6ec;
  ExceptionList = &pvStack_10;
  if (*(short *)(DAT_013b71e8 + 0x734) < *(short *)(DAT_013b71e8 + 0x738)) {
    ExceptionList = &pvStack_10;
    *(undefined2 *)(DAT_013b71e8 + 0x738) = *(undefined2 *)(DAT_013b71e8 + 0x734);
  }
  if (*(int *)(param_1 + 0x27acc) != 0) {
    FUN_013c0db6();
    (**(code **)(**(int **)(param_1 + 0x27acc) + 0x8c))(extraout_ECX);
  }
  if (*(int *)(param_1 + 0x27acc) != 0) {
    uVar3 = FUN_013c0dd1();
    (**(code **)(**(int **)(param_1 + 0x27acc) + 0x88))(uVar3);
  }
  if (*(int *)(param_1 + 0x27ad8) != 0) {
    FUN_013c0deb();
    (**(code **)(**(int **)(param_1 + 0x27ad8) + 0x8c))(extraout_EDX);
  }
  if (*(int *)(param_1 + 0x27ad8) != 0) {
    FUN_013c0e06();
    (**(code **)(**(int **)(param_1 + 0x27ad8) + 0x88))(extraout_EDX_00);
  }
  FUN_013c0e24();
  FUN_0058f078(local_168,&DAT_005b7bc4,extraout_ECX_00);
  if (*(int *)(param_1 + 0x2793c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2793c) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27940) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27940) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27ad0) != 0) {
    FUN_013c0e3f();
    (**(code **)(**(int **)(param_1 + 0x27ad0) + 0x8c))(extraout_EDX_01);
  }
  if (*(int *)(param_1 + 0x27ad0) != 0) {
    FUN_013c0e5a();
    (**(code **)(**(int **)(param_1 + 0x27ad0) + 0x88))(extraout_EDX_02);
  }
  if (*(int *)(param_1 + 0x27adc) != 0) {
    FUN_013c0e75();
    (**(code **)(**(int **)(param_1 + 0x27adc) + 0x8c))(extraout_EDX_03);
  }
  if (*(int *)(param_1 + 0x27adc) != 0) {
    FUN_013c0e90();
    (**(code **)(**(int **)(param_1 + 0x27adc) + 0x88))(extraout_EDX_04);
  }
  FUN_013c0eae();
  FUN_0058f078(local_168,&DAT_005b7bc8,extraout_ECX_01);
  if (*(int *)(param_1 + 0x27944) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27944) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27948) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27948) + 0x80))(local_168,0);
  }
  local_1f8 = 0;
  local_1f4 = 0;
  local_17c = 0;
  local_18 = *(int **)(param_1 + 0x2873c);
  local_18c = 0;
  for (local_178 = 0; local_178 < 7; local_178 = local_178 + 1) {
    for (local_204 = 0; local_204 < 9; local_204 = local_204 + 1) {
      local_18c = (**(code **)(*local_18 + 0xb8))(local_204,local_178);
      local_208 = 0;
      if (local_18c != 0) {
        local_208 = FUN_0054cd07(*(undefined4 *)(local_18c + 0x670),0x26);
      }
      if ((local_18c != 0) &&
         (((((**(short **)(local_18c + 0x670) == 0x1001 ||
             (**(short **)(local_18c + 0x670) == 0xd67)) ||
            (**(short **)(local_18c + 0x670) == 0xcfa)) ||
           ((0x1ab < **(short **)(local_18c + 0x670) && (**(short **)(local_18c + 0x670) < 0x1b0))))
          || ((0x2a7 < **(short **)(local_18c + 0x670) && (**(short **)(local_18c + 0x670) < 0x2ae))
             )))) {
        local_20c = FUN_0054e93b(*(undefined4 *)(local_18c + 0x670));
        local_1f8 = local_1f8 + local_20c;
      }
      if ((local_18c != 0) &&
         (((**(short **)(local_18c + 0x670) == 0xcfb ||
           ((0x1af < **(short **)(local_18c + 0x670) && (**(short **)(local_18c + 0x670) < 0x1b4))))
          || ((0x2ad < **(short **)(local_18c + 0x670) && (**(short **)(local_18c + 0x670) < 0x2b4))
             )))) {
        local_210 = FUN_0054e93b(*(undefined4 *)(local_18c + 0x670));
        local_1f4 = local_1f4 + local_210;
      }
      if ((local_18c == 0) ||
         (((**(short **)(local_18c + 0x670) < 400 || (0x194 < **(short **)(local_18c + 0x670))) &&
          (**(short **)(local_18c + 0x670) != 0x19f)))) {
        if (((local_18c == 0) || (**(short **)(local_18c + 0x670) < 0x195)) ||
           (0x199 < **(short **)(local_18c + 0x670))) {
          if ((local_18c != 0) && (local_208 == 0xe6)) {
            local_214 = FUN_0054e93b(*(undefined4 *)(local_18c + 0x670));
            if (local_214 == 0) {
              local_17c = local_17c + 1;
            }
            else {
              local_17c = local_17c + local_214;
            }
          }
        }
        else {
          local_1f4 = local_1f4 + 1;
        }
      }
      else {
        local_1f8 = local_1f8 + 1;
      }
    }
  }
  if (*(char *)(*(int *)(param_1 + 0x4c) + 0x788) == '\x01') {
    if (*(int *)(param_1 + 0x2796c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x2796c) + 0x80))(&DAT_005b7bcc,0);
    }
    if (*(int *)(param_1 + 0x27970) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27970) + 0x80))(&DAT_005b7bd0,0);
    }
    if (DAT_005b892c == 2) {
      if (*(int *)(param_1 + 0x27a64) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a64) + 0x80))(&DAT_005b7bd4,0);
      }
      if (*(int *)(param_1 + 0x27a68) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a68) + 0x80))(&DAT_005b7bd8,0);
      }
    }
  }
  else {
    FUN_0058f078(local_168,&DAT_005b7bdc,local_1f8);
    if (*(int *)(param_1 + 0x2796c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x2796c) + 0x80))(local_168,0);
    }
    if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x27a64) != 0)) {
      (**(code **)(**(int **)(param_1 + 0x27a64) + 0x80))(local_168,0);
    }
    FUN_0058f078(local_168,&DAT_005b7be0,local_1f4);
    if (*(int *)(param_1 + 0x27970) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27970) + 0x80))(local_168,0);
    }
    if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x27a68) != 0)) {
      (**(code **)(**(int **)(param_1 + 0x27a68) + 0x80))(local_168,0);
    }
  }
  FUN_0058f078(local_168,&DAT_005b7be4,local_17c);
  if (*(int *)(param_1 + 0x27974) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27974) + 0x80))(local_168,0);
  }
  if ((param_2 & 0x10) != 0) {
    ExceptionList = pvStack_10;
    return;
  }
  FUN_0058f078(local_168,&DAT_005b7be8,*(undefined4 *)(local_1fc + 0x1c));
  if (*(int *)(param_1 + 0x2794c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2794c) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27954) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27954) + 0x80))(local_168,0);
  }
  local_194 = 0;
  local_174 = FUN_004ff388();
  local_1c = (int)*(short *)(local_1fc + 0x40);
  if (local_174 == 2) {
    local_194 = (int)(&PTR_DAT_005c1900)[local_1c] - *(int *)(&DAT_005c18fc + local_1c * 4);
  }
  else {
    local_194 = *(int *)(&DAT_005c12b4 + local_1c * 4) - *(int *)(&DAT_005c12b0 + local_1c * 4);
  }
  local_194 = local_194 / 10;
  local_194 = __ftol();
  if (DAT_005b892c == 2) {
    FUN_0058f078(local_168,&DAT_00a3d978,*(undefined4 *)(DAT_013b71e8 + 0xc50),local_194);
  }
  else {
    FUN_0058f078(local_168,s__12d____d___005b7bec,*(undefined4 *)(DAT_013b71e8 + 0xc50),local_194);
  }
  if (*(int *)(param_1 + 0x279a8) != 0) {
    (**(code **)(**(int **)(param_1 + 0x279a8) + 0x80))(local_168,0);
    if ((int)local_194 < 0x50) {
      (**(code **)(**(int **)(param_1 + 0x279a8) + 0x84))(0xffffffff);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x279a8) + 0x84))(0xffff0000);
    }
  }
  if (*(int *)(param_1 + 0x27ac8) != 0) {
    if (*(int *)(DAT_013b71e8 + 0xc50) < 1) {
      (**(code **)(**(int **)(param_1 + 0x27ac8) + 0x60))(0);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x27ac8) + 0x60))(1);
    }
  }
  FUN_0058f078(local_168,&DAT_005b7bf8,*(short *)(local_1fc + 0x40) + 1);
  if (*(int *)(param_1 + 0x2795c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2795c) + 0x80))(local_168,0);
  }
  if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x27a54) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x27a54) + 0x80))(local_168,0);
  }
  local_174 = FUN_004ff388();
  local_1c = (int)*(short *)(local_1fc + 0x40);
  if (local_174 == 2) {
    FUN_0058f078(local_168,&DAT_005b7bfc,(&PTR_DAT_005c1900)[local_1c]);
  }
  else {
    FUN_0058f078(local_168,&DAT_005b7c00,*(undefined4 *)(&DAT_005c12b4 + local_1c * 4));
  }
  if (DAT_005b892c == 2) {
    local_314 = DAT_005d0428;
    puVar7 = &local_313;
    for (iVar5 = 0x3f; iVar5 != 0; iVar5 = iVar5 + -1) {
      *puVar7 = 0;
      puVar7 = puVar7 + 1;
    }
    *(undefined2 *)puVar7 = 0;
    *(undefined1 *)((int)puVar7 + 2) = 0;
    uVar3 = (**(code **)(**(int **)(param_1 + 0x2794c) + 0x88))(local_168);
    FUN_0058f078(&local_314,s__s____s_005b7c04,uVar3);
    (**(code **)(**(int **)(param_1 + 0x2794c) + 0x80))(&local_314,0);
  }
  if (*(int *)(param_1 + 0x27950) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27950) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27958) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27958) + 0x80))(local_168,0);
  }
  FUN_013c0ec9();
  FUN_0058f078(local_168,&DAT_005b7c0c,extraout_ECX_02);
  if (*(int *)(param_1 + 0x27960) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27960) + 0x80))(local_168,0);
  }
  FUN_013c0ee7();
  FUN_0058f078(local_168,&DAT_005b7c10,extraout_EDX_05);
  if (*(int *)(param_1 + 0x27964) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27964) + 0x80))(local_168,0);
  }
  FUN_013c0f05();
  FUN_0058f078(local_168,&DAT_005b7c14,extraout_ECX_03);
  if (*(int *)(param_1 + 0x27968) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27968) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_1ec,&DAT_005b7c18,*(undefined4 *)(local_1fc + 0x18));
  if (*(int *)(param_1 + 0x27920) != 0) {
    *(undefined1 *)(*(int *)(param_1 + 0x27920) + 0xcdd) = 1;
    (**(code **)(**(int **)(param_1 + 0x27920) + 0x80))(local_1ec,0);
  }
  if (*(int *)(param_1 + 0x27924) != 0) {
    *(undefined1 *)(*(int *)(param_1 + 0x27924) + 0xcdd) = 1;
    (**(code **)(**(int **)(param_1 + 0x27924) + 0x80))(local_1ec,0);
  }
  FUN_0058f078(local_1ec,&DAT_005b7c20,*(int *)(local_1fc + 0x18) - *(int *)(param_1 + 0x276b0));
  if (*(int *)(param_1 + 0x27928) != 0) {
    *(undefined1 *)(*(int *)(param_1 + 0x27928) + 0xcdd) = 1;
    (**(code **)(**(int **)(param_1 + 0x27928) + 0x80))(local_1ec,0);
  }
  FUN_0058f078(local_e8,&DAT_005b7c28,local_1fc);
  if (*(int *)(param_1 + 0x2792c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2792c) + 0x80))(local_e8,1);
  }
  local_1ac[0] = &DAT_00a37df8;
  local_1ac[1] = &DAT_00a37e78;
  local_1ac[2] = &DAT_00a37ef8;
  local_1ac[3] = &DAT_00a37f78;
  if (DAT_005b892c == 2) {
    if (*(short *)(local_1fc + 0x12) == 0) {
      (**(code **)(**(int **)(param_1 + 0x27930) + 0x80))(&DAT_005d042c,0);
      (**(code **)(**(int **)(param_1 + 0x27930) + 0x84))(0xffffffff);
    }
    else {
      if (DAT_013b71e8 + 0xf18 + *(char *)(DAT_013b71e8 + 0xef4) * 0xc != 0) {
        FUN_0058f078(local_414,&DAT_005b7c30,
                     DAT_013b71e8 + 0xf18 + *(char *)(DAT_013b71e8 + 0xef4) * 0xc);
      }
      (**(code **)(**(int **)(param_1 + 0x27930) + 0x80))(local_414,0);
      (**(code **)(**(int **)(param_1 + 0x27930) + 0x84))(0xffaaaaaa);
    }
  }
  if (*(short *)(local_1fc + 0x5c) < 0x28) {
    if ((int)*(short *)(local_1fc + 0x5c) % 10 < 6) {
      (**(code **)(**(int **)(param_1 + 0x27934) + 0x80))(local_1ac[*(byte *)(local_1fc + 0x14)],0);
      (**(code **)(**(int **)(param_1 + 0x27938) + 0x80))(&DAT_005b7c34,0);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x27934) + 0x80))(local_1ac[*(byte *)(local_1fc + 0x14)],0);
      (**(code **)(**(int **)(param_1 + 0x27938) + 0x80))
                (local_1ac[(int)*(short *)(local_1fc + 0x5c) / 10],0);
    }
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27934) + 0x80))(s_Monster_005b7c38,0);
    (**(code **)(**(int **)(param_1 + 0x27938) + 0x80))(&DAT_005b7c40,0);
  }
  FUN_013c0f23();
  FUN_0058f078(local_168,&DAT_005b7c44,extraout_ECX_04);
  if (*(int *)(param_1 + 0x27978) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27978) + 0x80))(local_168,0);
  }
  FUN_013c0f3e();
  FUN_0058f078(local_168,&DAT_005b7c48,extraout_EDX_06);
  if (*(int *)(param_1 + 0x2797c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2797c) + 0x80))(local_168,0);
  }
  FUN_013c0f59();
  FUN_0058f078(local_168,&DAT_005b7c4c,extraout_ECX_05);
  if (*(int *)(param_1 + 0x27980) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27980) + 0x80))(local_168,0);
  }
  FUN_013c0f74();
  FUN_0058f078(local_168,&DAT_005b7c50,extraout_EDX_07);
  if (*(int *)(param_1 + 0x27984) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27984) + 0x80))(local_168,0);
  }
  local_184 = ((*(short *)(local_1fc + 0x40) + 1) * 3) / 2;
  if (200 < local_184) {
    local_184 = 200;
  }
  iVar5 = FUN_004ff388();
  if (iVar5 == 2) {
    local_184 = 200;
  }
  local_170 = local_184;
  local_68 = local_184;
  local_58 = local_184;
  iVar5 = FUN_00434315(0x1f);
  if (iVar5 == 1) {
    local_58 = 0xff;
  }
  iVar5 = FUN_00434315(0x27);
  if (iVar5 == 1) {
    local_68 = 0xff;
  }
  iVar5 = FUN_00434315(0x2f);
  if (iVar5 == 1) {
    local_170 = 0xff;
  }
  FUN_0058f078(local_168,s__3d___3d_005b7c54,*(undefined1 *)(local_1fc + 0x58),local_184);
  if (*(int *)(param_1 + 0x27988) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27988) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,s__3d___3d_005b7c60,*(undefined1 *)(local_1fc + 0x59),local_58);
  if (*(int *)(param_1 + 0x2798c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2798c) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,s__3d___3d_005b7c6c,*(undefined1 *)(local_1fc + 0x5a),local_68);
  if (*(int *)(param_1 + 0x27990) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27990) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,s__3d___3d_005b7c78,*(undefined1 *)(local_1fc + 0x5b),local_170);
  if (*(int *)(param_1 + 0x27994) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27994) + 0x80))(local_168,0);
  }
  if (local_174 == 2) {
    local_16c = (int)(&PTR_DAT_005c1900)[local_1c] - *(int *)(&DAT_005c18fc + local_1c * 4);
    local_200 = *(int *)(local_1fc + 0x1c) - *(int *)(&DAT_005c18fc + local_1c * 4);
  }
  else {
    local_16c = *(int *)(&DAT_005c12b4 + local_1c * 4) - *(int *)(&DAT_005c12b0 + local_1c * 4);
    local_200 = *(int *)(local_1fc + 0x1c) - *(int *)(&DAT_005c12b0 + local_1c * 4);
  }
  local_16c = local_16c >> 2;
  local_180 = 4;
  if (local_200 < local_16c) {
    local_180 = 1;
  }
  else if (local_200 < local_16c << 1) {
    local_180 = 2;
  }
  else if (local_200 < local_16c * 3) {
    local_180 = 3;
  }
  for (local_1f0 = 0; local_1f0 < 3; local_1f0 = local_1f0 + 1) {
    if (*(int *)(param_1 + 0x279ac + local_1f0 * 4) != 0) {
      if (local_1f0 < local_180 + -1) {
        *(undefined4 *)(*(int *)(param_1 + 0x279ac + local_1f0 * 4) + 0xa4) = 1;
      }
      else {
        *(undefined4 *)(*(int *)(param_1 + 0x279ac + local_1f0 * 4) + 0xa4) = 0;
      }
    }
  }
  local_200 = local_200 - (local_180 + -1) * local_16c;
  local_64 = local_16c;
  if (*(int *)(param_1 + 0x279b8) != 0) {
    (**(code **)(**(int **)(param_1 + 0x279b8) + 0x8c))(local_16c);
    (**(code **)(**(int **)(param_1 + 0x279b8) + 0x88))(local_200);
  }
  local_54 = (int)*(short *)(local_1fc + 0x5c);
  local_190 = 0;
  if (local_54 < 0x28) {
    if (local_54 == 0x20) {
      local_190 = 2;
    }
    else {
      local_41c = local_54 % 10;
      local_418 = local_54 / 10;
      local_190 = local_418;
      if ((5 < local_41c) && (local_41c < 10)) {
        local_190 = local_41c + -6;
      }
    }
    *(undefined1 *)(local_1fc + 0x14) = (undefined1)local_190;
  }
  if ((param_2 & 1) == 0) {
    local_420 = 0;
    local_424 = 0;
    for (local_188 = 0; (int)local_188 < 0x18; local_188 = local_188 + 1) {
      local_428 = 1 << ((byte)local_188 & 0x1f);
      if (*(int *)(param_1 + 0x279bc + local_188 * 4) != 0) {
        if ((*(uint *)(local_1fc + 0x2dc) & local_428) == local_428) {
          local_42c = (int *)(**(code **)(**(int **)(param_1 + 0x279bc + local_188 * 4) + 0xb4))
                                       (0,0);
          local_420 = (uint)*(byte *)(local_1fc + 0x14) * 0x18 + 5000 + local_188;
          if (local_42c != (int *)0x0) {
            local_438 = (short *)(**(code **)(*local_42c + 0x98))();
            local_424 = (int)*local_438;
            if ((0 < local_424) && (local_420 == local_424)) {
              local_42c = (int *)0x0;
              goto LAB_00444ac9;
            }
          }
          (**(code **)(**(int **)(param_1 + 0x279bc + local_188 * 4) + 0xbc))();
          local_58c = operator_new(8);
          local_430 = local_58c;
          _memset(local_58c,0,8);
          *local_430 = (undefined2)local_420;
          local_594 = operator_new(0x678);
          local_8 = 0;
          if (local_594 == (void *)0x0) {
            local_5b8 = 0;
          }
          else {
            local_5b8 = FUN_0040d13e(0,local_430,0,0);
          }
          local_590 = local_5b8;
          local_8 = 0xffffffff;
          local_434 = local_5b8;
          if (local_5b8 != 0) {
            (**(code **)(**(int **)(param_1 + 0x279bc + local_188 * 4) + 0x94))(local_59c,local_5b8)
            ;
          }
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x279bc + local_188 * 4) + 0xbc))();
        }
      }
LAB_00444ac9:
    }
    for (local_188 = 0; (int)local_188 < 8; local_188 = local_188 + 1) {
      local_43c = 1 << ((char)local_188 + 0x18U & 0x1f);
      if (*(int *)(param_1 + 0x28714) != 0) {
        local_448 = 0;
        local_444 = (undefined4 *)0x0;
        local_440 = (int *)0x0;
        local_420 = local_188 + 0x13e8;
        if (DAT_005b892c == 2) {
          uVar6 = local_188 & 0x80000003;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
          }
          local_440 = (int *)(**(code **)(**(int **)(param_1 + 0x28714) + 0xb4))
                                       (uVar6,(int)(local_188 + ((int)local_188 >> 0x1f & 3U)) >> 2)
          ;
        }
        else {
          uVar6 = local_188 & 0x80000003;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
          }
          local_440 = (int *)(**(code **)(**(int **)(param_1 + 0x28714) + 0xb4))
                                       (uVar6,(int)(local_188 + ((int)local_188 >> 0x1f & 3U)) >> 2)
          ;
        }
        if (local_440 != (int *)0x0) {
          local_44c = (short *)(**(code **)(*local_440 + 0x98))();
          local_424 = (int)*local_44c;
          if ((0 < local_424) && (local_420 == local_424)) {
            local_440 = (int *)0x0;
            goto LAB_00444d40;
          }
        }
        if (DAT_005b892c == 2) {
          uVar6 = local_188 & 0x80000003;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
          }
          local_444 = (undefined4 *)
                      (**(code **)(**(int **)(param_1 + 0x28714) + 0xa4))
                                (uVar6,(int)(local_188 + ((int)local_188 >> 0x1f & 3U)) >> 2);
        }
        else {
          local_444 = (undefined4 *)(**(code **)(**(int **)(param_1 + 0x28714) + 0xa4))(local_188,0)
          ;
        }
        if ((*(uint *)(local_1fc + 0x2dc) & local_43c) == local_43c) {
          local_450 = operator_new(8);
          _memset(local_450,0,8);
          *local_450 = (short)local_188 + 0x13e8;
          pvVar4 = operator_new(0x678);
          local_8 = 1;
          if (pvVar4 == (void *)0x0) {
            local_5bc = 0;
          }
          else {
            local_5bc = FUN_0040d13e(0,local_450,0,0);
          }
          local_8 = 0xffffffff;
          local_454 = local_5bc;
          if (DAT_005b892c == 2) {
            uVar6 = local_188 & 0x80000003;
            if ((int)uVar6 < 0) {
              uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
            }
            (**(code **)(**(int **)(param_1 + 0x28714) + 0x8c))
                      (local_5bc,uVar6,(int)(local_188 + ((int)local_188 >> 0x1f & 3U)) >> 2);
          }
          else {
            (**(code **)(**(int **)(param_1 + 0x28714) + 0x8c))(local_5bc,local_188,0);
          }
        }
        if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
           (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == local_444)) {
          *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
        }
        if ((local_448 != 1) && (local_444 != (undefined4 *)0x0)) {
          if (local_444 != (undefined4 *)0x0) {
            (**(code **)*local_444)(1);
          }
          local_444 = (undefined4 *)0x0;
        }
      }
LAB_00444d40:
    }
  }
  if ((*(int *)(param_1 + 0x27a1c) == 0) || (*(char *)(local_1fc + 0x14) != '\0')) {
    if (*(int *)(param_1 + 0x27a1c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27a1c) + 0x60))(1);
      if (*(char *)(local_1fc + 0x14) == '\x01') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a1c) + 0xa4) = 0;
      }
      if (*(char *)(local_1fc + 0x14) == '\x02') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a1c) + 0xa4) = 1;
      }
      if (*(char *)(local_1fc + 0x14) == '\x03') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a1c) + 0xa4) = 2;
      }
    }
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27a1c) + 0x60))(0);
  }
  if ((*(int *)(param_1 + 0x27a20) == 0) || (*(char *)(local_1fc + 0x14) != '\0')) {
    if (*(int *)(param_1 + 0x27a20) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27a20) + 0x60))(1);
      if (*(char *)(local_1fc + 0x14) == '\x01') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a20) + 0xa4) = 0;
      }
      if (*(char *)(local_1fc + 0x14) == '\x02') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a20) + 0xa4) = 1;
      }
      if (*(char *)(local_1fc + 0x14) == '\x03') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a20) + 0xa4) = 2;
      }
    }
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27a20) + 0x60))(0);
  }
  if ((*(int *)(param_1 + 0x27a24) == 0) || (*(char *)(local_1fc + 0x14) != '\0')) {
    if (*(int *)(param_1 + 0x27a24) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27a24) + 0x60))(1);
      if (*(char *)(local_1fc + 0x14) == '\x01') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a24) + 0xa4) = 0;
      }
      if (*(char *)(local_1fc + 0x14) == '\x02') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a24) + 0xa4) = 1;
      }
      if (*(char *)(local_1fc + 0x14) == '\x03') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a24) + 0xa4) = 2;
      }
    }
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27a24) + 0x60))(0);
  }
  if ((*(int *)(param_1 + 0x27a28) == 0) || (*(char *)(local_1fc + 0x14) != '\0')) {
    if (*(int *)(param_1 + 0x27a28) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27a28) + 0x60))(1);
      if (*(char *)(local_1fc + 0x14) == '\x01') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a28) + 0xa4) = 0;
      }
      if (*(char *)(local_1fc + 0x14) == '\x02') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a28) + 0xa4) = 1;
      }
      if (*(char *)(local_1fc + 0x14) == '\x03') {
        *(undefined4 *)(*(int *)(param_1 + 0x27a28) + 0xa4) = 2;
      }
    }
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27a28) + 0x60))(0);
  }
  if (DAT_005b892c == 2) {
    switch(*(undefined1 *)(local_1fc + 0x14)) {
    case 0:
      (**(code **)(**(int **)(param_1 + 0x27998) + 0x80))(&DAT_00a3ba78,0);
      (**(code **)(**(int **)(param_1 + 0x2799c) + 0x80))(&DAT_00a3baf8,0);
      (**(code **)(**(int **)(param_1 + 0x279a0) + 0x80))(&DAT_00a3bb78,0);
      in_stack_fffffa04 = 0;
      (**(code **)(**(int **)(param_1 + 0x279a4) + 0x80))(&DAT_00a3bbf8,0);
      (**(code **)(**(int **)(param_1 + 0x27a2c) + 0x80))(&DAT_00a376f8,0);
      (**(code **)(**(int **)(param_1 + 0x27a30) + 0x80))(&DAT_00a37778,0);
      (**(code **)(**(int **)(param_1 + 0x27a34) + 0x80))(&DAT_00a377f8,0);
      break;
    case 1:
      (**(code **)(**(int **)(param_1 + 0x27998) + 0x80))(&DAT_00a3bc78,0);
      (**(code **)(**(int **)(param_1 + 0x2799c) + 0x80))(&DAT_00a3bcf8,0);
      (**(code **)(**(int **)(param_1 + 0x279a0) + 0x80))(&DAT_00a3bd78,0);
      in_stack_fffffa04 = 0;
      (**(code **)(**(int **)(param_1 + 0x279a4) + 0x80))(&DAT_00a3bdf8,0);
      (**(code **)(**(int **)(param_1 + 0x27a2c) + 0x80))(&DAT_00a37878,0);
      (**(code **)(**(int **)(param_1 + 0x27a30) + 0x80))(&DAT_00a378f8,0);
      (**(code **)(**(int **)(param_1 + 0x27a34) + 0x80))(&DAT_00a37978,0);
      break;
    case 2:
      (**(code **)(**(int **)(param_1 + 0x27998) + 0x80))(&DAT_00a3be78,0);
      (**(code **)(**(int **)(param_1 + 0x2799c) + 0x80))(&DAT_00a3bef8,0);
      (**(code **)(**(int **)(param_1 + 0x279a0) + 0x80))(&DAT_00a3bf78,0);
      in_stack_fffffa04 = 0;
      (**(code **)(**(int **)(param_1 + 0x279a4) + 0x80))(&DAT_00a3bff8,0);
      (**(code **)(**(int **)(param_1 + 0x27a2c) + 0x80))(&DAT_00a379f8,0);
      (**(code **)(**(int **)(param_1 + 0x27a30) + 0x80))(&DAT_00a37a78,0);
      (**(code **)(**(int **)(param_1 + 0x27a34) + 0x80))(&DAT_00a37af8,0);
      break;
    case 3:
      (**(code **)(**(int **)(param_1 + 0x27998) + 0x80))(&DAT_00a3c078,0);
      (**(code **)(**(int **)(param_1 + 0x2799c) + 0x80))(&DAT_00a3c0f8,0);
      (**(code **)(**(int **)(param_1 + 0x279a0) + 0x80))(&DAT_00a3c178,0);
      in_stack_fffffa04 = 0;
      (**(code **)(**(int **)(param_1 + 0x279a4) + 0x80))(&DAT_00a3c1f8,0);
      (**(code **)(**(int **)(param_1 + 0x27a2c) + 0x80))(&DAT_00a383f8,0);
      (**(code **)(**(int **)(param_1 + 0x27a30) + 0x80))(&DAT_00a38478,0);
      (**(code **)(**(int **)(param_1 + 0x27a34) + 0x80))(&DAT_00a384f8,0);
      break;
    default:
      (**(code **)(**(int **)(param_1 + 0x27998) + 0x80))(&DAT_00a3ba78,0);
      (**(code **)(**(int **)(param_1 + 0x2799c) + 0x80))(&DAT_00a3baf8,0);
      (**(code **)(**(int **)(param_1 + 0x279a0) + 0x80))(&DAT_00a3bb78,0);
      in_stack_fffffa04 = 0;
      (**(code **)(**(int **)(param_1 + 0x279a4) + 0x80))(&DAT_00a3bbf8,0);
      (**(code **)(**(int **)(param_1 + 0x27a2c) + 0x80))(&DAT_00a376f8,0);
      (**(code **)(**(int **)(param_1 + 0x27a30) + 0x80))(&DAT_00a37778,0);
      (**(code **)(**(int **)(param_1 + 0x27a34) + 0x80))(&DAT_00a377f8,0);
    }
  }
  local_60 = (uint)*(byte *)(local_1fc + 0x58);
  local_14 = FUN_013c0f8f();
  if (100 < (int)local_60) {
    local_60 = 100;
  }
  FUN_0058f078(local_168,&DAT_005b7c84,local_14);
  if (*(int *)(param_1 + 0x27a6c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a6c) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27a70) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a70) + 0x80))(local_168,0);
  }
  local_19c = *(undefined4 *)(param_1 + 0x27a90);
  local_50 = *(char *)(DAT_013b71e8 + 0xef6 + (int)*(char *)(DAT_013b71e8 + 0xef5));
  if ((local_50 < '\0') || ('g' < local_50)) {
    FUN_0058f078(local_168,&DAT_005b7c90,0);
    if (*(int *)(param_1 + 0x27a90) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27a90) + 0x80))(local_168,0);
    }
    if (*(int *)(param_1 + 0x27a74) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27a74) + 0x80))(local_168,0);
    }
  }
  else {
    FUN_004431d9();
    local_458 = (int)*(short *)(DAT_013b71e8 + 0x778);
    local_460 = DAT_005d03e4;
    if (DAT_005d03e4 == 3) {
      local_460 = 2;
    }
    iVar5 = __ftol();
    if ((((iVar5 >> 7 < 0x1b) || (iVar5 = __ftol(), 0x1e < iVar5 >> 7)) ||
        (iVar5 = __ftol(), iVar5 >> 7 < 0x15)) || (iVar5 = __ftol(), 0x18 < iVar5 >> 7)) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    if (bVar2) {
      local_460 = 2;
    }
    uVar3 = FUN_00442ff7((int)*(short *)(DAT_013b71e8 + 0xc4 +
                                        *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
    local_45c = FUN_00557aff((int)local_50,local_1fc,local_460,uVar3);
    if (local_45c < 0) {
      FUN_0058f078(local_168,&DAT_005b7c8c,-local_45c);
      if (*(int *)(param_1 + 0x27a90) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a90) + 0x80))(local_168,0);
      }
      if (*(int *)(param_1 + 0x27a74) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a74) + 0x80))(local_168,0);
      }
      if (*(int *)(param_1 + 0x27a90) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a90) + 0x84))(0xffaaffaa);
      }
      if (*(int *)(param_1 + 0x27a74) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a74) + 0x84))(0xffaaffaa);
      }
    }
    else {
      uVar3 = FUN_013c0c60();
      FUN_0058f078(local_168,&DAT_005b7c88,uVar3);
      if (*(int *)(param_1 + 0x27a90) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a90) + 0x80))(local_168,0);
      }
      if (*(int *)(param_1 + 0x27a74) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a74) + 0x80))(local_168,0);
      }
      if (*(int *)(param_1 + 0x27a90) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a90) + 0x84))(0xffbbbbff);
      }
      if (*(int *)(param_1 + 0x27a74) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27a74) + 0x84))(0xffbbbbff);
      }
    }
  }
  if (*(char *)(*(int *)(param_1 + 0x4c) + 0x246) == '\0') {
    FUN_004fa8a1(*(undefined4 *)(param_1 + 0x26e7c));
  }
  local_4c = __ftol();
  FUN_0058f078(local_168,&DAT_005b7c94,local_4c);
  if (*(int *)(param_1 + 0x27a78) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a78) + 0x80))(local_168,0);
  }
  FUN_013c0fa9();
  FUN_0058f078(local_168,&DAT_005b7c98,extraout_ECX_06);
  if (*(int *)(param_1 + 0x27a7c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a7c) + 0x80))(local_168,0);
  }
  if (*(int *)(param_1 + 0x27a80) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a84) + 0x80))(local_168,0);
  }
  sVar1 = *(short *)(local_1fc + 0x26);
  iVar5 = FUN_0054eb20(local_1fc,3);
  local_24 = FUN_0054eb20(local_1fc,0x35);
  local_24 = sVar1 + iVar5 + local_24;
  FUN_0058f078(local_168,&DAT_005b7c9c,local_24);
  if (*(int *)(param_1 + 0x27a80) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a80) + 0x80))(local_168,0);
  }
  local_20 = 0;
  if ((*(int *)(*(int *)(param_1 + 0x4c) + 0x24c) == 0x1a) &&
     (*(short *)(*(int *)(param_1 + 0x4c) + 0x1d2) == 0)) {
    local_20 = 0x14;
  }
  else if ((*(int *)(*(int *)(param_1 + 0x4c) + 0x24c) == 0x1a) &&
          (*(short *)(*(int *)(param_1 + 0x4c) + 0x1d2) == 1)) {
    local_20 = 0;
    if ((*(uint *)(local_1fc + 0x2dc) & 0x80000) == 0x80000) {
      local_20 = 0x14;
    }
  }
  else if (*(int *)(*(int *)(param_1 + 0x4c) + 0x24c) == 0x21) {
    local_20 = 0x14;
    if ((*(uint *)(local_1fc + 0x2dc) & 0x200000) == 0x200000) {
      local_20 = 0x28;
    }
  }
  else if (*(int *)(*(int *)(param_1 + 0x4c) + 0x24c) == 0x28) {
    local_20 = 0x14;
  }
  else if (*(int *)(*(int *)(param_1 + 0x4c) + 0x24c) == 0x3f) {
    local_20 = 0x1e;
  }
  iVar5 = FUN_0054eb20(local_1fc,0x1a);
  local_198 = local_20 + 100 + iVar5 + (int)*(short *)(local_1fc + 0x54) / 5 +
              ((int)*(char *)(*(int *)(param_1 + 0x4c) + 0x783) -
              (int)*(char *)(*(int *)(param_1 + 0x4c) + 0x784)) *
              (*(byte *)(local_1fc + 0x5a) / 10 + 10);
  if (*(char *)(*(int *)(param_1 + 0x4c) + 0x77f) == '\x01') {
    local_198 = local_198 + -0x1e;
  }
  FUN_0058f078(local_168,&DAT_005b7ca0,local_198);
  if (*(int *)(param_1 + 0x27a88) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a88) + 0x80))(local_168,0);
  }
  local_48 = (uint)*(byte *)(local_1fc + 0x2e6) << 1;
  FUN_0058f078(local_168,s__d__d___005b7ca8,local_48 / 10,local_48 % 10);
  if (*(int *)(param_1 + 0x27a8c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a8c) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,&DAT_005b7cb0,(int)*(char *)(local_1fc + 0x2f2));
  if (*(int *)(param_1 + 0x27a94) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a94) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,&DAT_005b7cb4,(int)*(char *)(local_1fc + 0x2f3));
  if (*(int *)(param_1 + 0x27a98) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a98) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,&DAT_005b7cb8,(int)*(char *)(local_1fc + 0x2f0));
  if (*(int *)(param_1 + 0x27a9c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27a9c) + 0x80))(local_168,0);
  }
  FUN_0058f078(local_168,&DAT_005b7cbc,(int)*(char *)(local_1fc + 0x2f1));
  if (*(int *)(param_1 + 0x27aa0) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27aa0) + 0x80))(local_168,0);
  }
  for (local_188 = 0; (int)local_188 < 0x10; local_188 = local_188 + 1) {
    if ((DAT_005b892c != 2) && ((int)local_188 < 4)) {
      local_468 = (int)(uint)*(ushort *)(*(int *)(param_1 + 0x4c) + 0x684 + local_188 * 2) >> 8;
      if (local_468 < 0x28) {
        (**(code **)(**(int **)(param_1 + 0x28628 + local_188 * 4) + 0x80))
                  (&DAT_005c0d38 + local_468 * 0x18,0);
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x28628 + local_188 * 4) + 0x80))(&DAT_005b7cc0,0);
      }
    }
    local_464 = (int)(uint)*(ushort *)(*(int *)(param_1 + 0x4c) + 0x684 + local_188 * 2) >> 8;
    if (*(int *)(param_1 + 0x28690 + local_188 * 4) != 0) {
      if (local_464 < 0x28) {
        (**(code **)(**(int **)(param_1 + 0x28690 + local_188 * 4) + 0x80))
                  (&DAT_005c0d38 + local_464 * 0x18,0);
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x28690 + local_188 * 4) + 0x80))(&DAT_005b7cc4,0);
      }
    }
  }
  local_188 = 0;
  do {
    if (0xf < (int)local_188) {
      if (*(int *)(param_1 + 0x27aa4) != 0) {
        FUN_0058f078(local_44,&DAT_005b7cd0,*(undefined4 *)(DAT_013b71e8 + 0xc58));
        *(undefined1 *)(*(int *)(param_1 + 0x27aa4) + 0xcdd) = 1;
        (**(code **)(**(int **)(param_1 + 0x27aa4) + 0x80))(local_44,0);
        if (*(int *)(param_1 + 0x27aa8) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27aa8) + 0x80))(local_44,0);
        }
      }
      if (*(int *)(*(int *)(param_1 + 0x4c) + 0x198) == 0) {
        (**(code **)(**(int **)(param_1 + 0x27c80) + 0x60))(0);
      }
      else if (*(short *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x198) + 0x24a) == 0x13) {
        (**(code **)(**(int **)(param_1 + 0x27c80) + 0x60))(0);
      }
      else {
        _memset(local_588,0,0x100);
        *(uint *)(*(int *)(param_1 + 0x27c80) + 0xa4) =
             (uint)*(ushort *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x198) + 0x24a);
        uVar3 = FUN_00526f0f(*(undefined4 *)(*(int *)(param_1 + 0x27c80) + 0xa4));
        *(undefined4 *)(*(int *)(param_1 + 0x27c80) + 0xa4) = uVar3;
        (**(code **)(**(int **)(param_1 + 0x287a8) + 0x80))
                  (&DAT_00d44950 + *(short *)(local_1fc + 0xd4) * 0x8c,0);
        uVar3 = FUN_0054cd07(local_1fc + 0xd4,3);
        FUN_0058f078(local_588,s__s__d_005b7cd8,&DAT_00a36978,uVar3);
        (**(code **)(**(int **)(param_1 + 0x287ac) + 0x80))(local_588,0);
        uVar3 = FUN_0054cd07(local_1fc + 0xd4,4);
        FUN_0058f078(local_588,s__s__d_005b7ce0,&DAT_00a369f8,uVar3);
        (**(code **)(**(int **)(param_1 + 0x287b0) + 0x80))(local_588,0);
        in_stack_fffffa04 = 1;
        (**(code **)(**(int **)(param_1 + 0x27c80) + 0x60))(1);
      }
      if (DAT_005d0408 == 1) {
        (**(code **)(**(int **)(param_1 + 0x27918) + 0x60))(1);
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x27918) + 0x60))(0);
      }
      FUN_013d3a00(in_stack_fffffa04);
      return;
    }
    if (*(char *)(*(int *)(param_1 + 0x4c) + 0x6a4 + local_188 * 8) == '\0') {
      if ((DAT_005b892c != 2) && ((int)local_188 < 4)) {
        (**(code **)(**(int **)(param_1 + 0x28638 + local_188 * 4) + 0x80))(&DAT_005d0430,0);
      }
      if (*(int *)(param_1 + 0x28690 + local_188 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x28650 + local_188 * 4) + 0x80))(&DAT_005d0434,0);
        (**(code **)(**(int **)(param_1 + 0x28650 + local_188 * 4) + 0x60))(0);
        (**(code **)(**(int **)(param_1 + 0x28690 + local_188 * 4) + 0x60))(0);
      }
      if ((*(int *)(param_1 + 0x286d0 + local_188 * 4) != 0) && (*(int *)(param_1 + 0x287fc) != 0))
      {
        (**(code **)(**(int **)(param_1 + 0x286d0 + local_188 * 4) + 0x60))(0);
        *(undefined4 *)(param_1 + 0x28e84 + local_188 * 4) = 0;
      }
    }
    else {
      if (*(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_188 * 8) < 1000000) {
        if (DAT_005b892c == 2) {
          FUN_0058f078(local_44,&PTR_DAT_005b7cc8,
                       *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_188 * 8));
        }
        else {
          FUN_0058f078(local_44,&DAT_005b7ccc,
                       *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_188 * 8));
        }
      }
      else {
        local_47c = *(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_188 * 8) / 1000000;
        local_46c = (*(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_188 * 8) % 1000000) / 10000;
        local_470 = *(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_188 * 8) % 10000;
        local_478 = 0x16d;
        uVar6 = local_46c & 0x80000003;
        if ((int)uVar6 < 0) {
          uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
        }
        if (uVar6 == 0) {
          local_478 = 0x16e;
        }
        local_474 = 0;
        if (local_47c == 4) {
          local_474 = 7;
        }
        else if (local_47c == 5) {
          local_474 = 0xf;
        }
        else if (local_47c == 6) {
          local_474 = 0x1e;
        }
        if (local_47c != 0) {
          local_480 = (local_474 - (*(int *)(param_1 + 0x276a4) - local_46c) * local_478) -
                      (*(int *)(param_1 + 0x276a8) - local_470);
          if (DAT_005b892c == 2) {
            FUN_0058f078(local_44,&DAT_00a3d2f8,local_480);
          }
          else {
            FUN_0058f078(local_44,&DAT_00a3d2f8,local_480);
          }
        }
      }
      if ((DAT_005b892c != 2) && ((int)local_188 < 4)) {
        (**(code **)(**(int **)(param_1 + 0x28638 + local_188 * 4) + 0x80))(local_44,0);
      }
      if (*(int *)(param_1 + 0x28690 + local_188 * 4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x28650 + local_188 * 4) + 0x80))(local_44,0);
        if (local_188 == 0) {
          local_484 = *(short *)(*(int *)(param_1 + 0x4c) + 0x6a8);
          if (local_484 < 0x1ccf) {
            for (local_488 = 0; local_488 < 7; local_488 = local_488 + 1) {
              if (local_484 < *(short *)((int)&PTR_DAT_005b4d6c + local_488 * 2)) {
                (**(code **)(**(int **)(param_1 + 0x28650) + 0x84))
                          (*(undefined4 *)(&DAT_005b4d50 + local_488 * 4));
                break;
              }
            }
          }
          else {
            (**(code **)(**(int **)(param_1 + 0x28650) + 0x84))(0xffffffff);
          }
        }
        (**(code **)(**(int **)(param_1 + 0x28650 + local_188 * 4) + 0x60))(1);
        (**(code **)(**(int **)(param_1 + 0x28690 + local_188 * 4) + 0x60))(1);
      }
      if (DAT_005b892c == 2) {
        if (*(int *)(param_1 + 0x28690 + local_188 * 4) != 0) {
          (**(code **)(**(int **)(param_1 + 0x28650 + local_188 * 4) + 0x60))(0);
          (**(code **)(**(int **)(param_1 + 0x28690 + local_188 * 4) + 0x60))(0);
        }
        if (*(int *)(param_1 + 0x287fc) != 0) {
          if (*(byte *)(*(int *)(param_1 + 0x4c) + 0x6a4 + local_188 * 8) < 0x28) {
            *(undefined4 *)(*(int *)(param_1 + 0x286d0 + local_188 * 4) + 0xa4) =
                 *(undefined4 *)
                  (&DAT_005b77c0 +
                  (uint)*(byte *)(*(int *)(param_1 + 0x4c) + 0x6a4 + local_188 * 8) * 4);
          }
          else {
            *(undefined4 *)(*(int *)(param_1 + 0x286d0 + local_188 * 4) + 0xa4) = 0;
          }
          *(undefined4 *)(*(int *)(param_1 + 0x286d0 + local_188 * 4) + 0x1ac) = 0x1d;
        }
      }
    }
    local_188 = local_188 + 1;
  } while( true );
}

