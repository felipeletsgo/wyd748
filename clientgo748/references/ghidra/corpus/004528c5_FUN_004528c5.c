// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004528c5 | Name: FUN_004528c5


undefined4 __thiscall FUN_004528c5(int param_1,char param_2,int param_3)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_28c [128];
  int *local_20c;
  int local_208;
  int local_204;
  int local_200;
  undefined1 local_1fc [128];
  int *local_17c;
  int local_178;
  int local_174;
  int *local_170;
  int local_16c;
  int local_168;
  int *local_164;
  int local_160;
  int *local_15c;
  char local_158;
  undefined4 local_154;
  undefined4 local_150;
  undefined4 local_14c;
  int local_148;
  int local_144;
  undefined1 local_140 [128];
  int *local_c0;
  int local_bc;
  int local_b8;
  int local_b4;
  undefined1 local_b0 [128];
  int *local_30;
  int local_2c;
  int local_28;
  int *local_24;
  int local_20;
  int local_1c;
  int *local_18;
  int local_14;
  int *local_10;
  char local_c;
  int local_8;
  
  if (*(int *)(DAT_013b71e0 + 0x10c) == 0) {
    if ((((('/' < param_2) && (param_2 < ':')) || (param_2 == '!')) ||
        (((param_2 == '@' || (param_2 == '#')) ||
         ((param_2 == '$' || ((param_2 == '%' || (param_2 == '^')))))))) ||
       ((param_2 == '&' || (((param_2 == '*' || (param_2 == '(')) || (param_2 == ')')))))) {
      if (*(int *)(param_1 + 0x26eb0) == 1) {
        *(undefined4 *)(param_1 + 0x26eb0) = 0;
        return 1;
      }
      if ((param_2 < '0') || ('9' < param_2)) {
        if (param_2 == '!') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 0;
        }
        else if (param_2 == '@') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 1;
        }
        else if (param_2 == '#') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 2;
        }
        else if (param_2 == '$') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 3;
        }
        else if (param_2 == '%') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 4;
        }
        else if (param_2 == '^') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 5;
        }
        else if (param_2 == '&') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 6;
        }
        else if (param_2 == '*') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 7;
        }
        else if (param_2 == '(') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 8;
        }
        else if (param_2 == ')') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 9;
        }
      }
      else {
        *(char *)(DAT_013b71e8 + 0xef5) = param_2 + -0x31;
        if (param_2 == '0') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 9;
        }
      }
      local_8 = (int)*(char *)(DAT_013b71e8 + 0xef5);
      iVar2 = FUN_0040c0f0();
      if (iVar2 == 1) {
        local_8 = local_8 + 10;
      }
      if (param_3 == 0) {
        if (local_8 < 10) {
          local_18 = *(int **)(param_1 + 0x28718);
          for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
            local_1c = (**(code **)(*local_18 + 0xb4))(local_14,0);
            if ((local_14 == local_8) && (local_1c != 0)) {
              *(undefined4 *)(local_1c + 0xa0) = 2;
              if (DAT_005b892c == 2) {
                *(undefined4 *)(local_1c + 0xa0) = 200;
              }
            }
            else if ((local_1c != 0) && (*(undefined4 *)(local_1c + 0xa0) = 1, DAT_005b892c == 2)) {
              *(undefined4 *)(local_1c + 0xa0) = 199;
            }
          }
        }
        else {
          local_24 = *(int **)(param_1 + 0x2871c);
          for (local_20 = 0; local_20 < 10; local_20 = local_20 + 1) {
            local_28 = (**(code **)(*local_24 + 0xb4))(local_20,0);
            if ((local_20 + 10 == local_8) && (local_28 != 0)) {
              *(undefined4 *)(local_28 + 0xa0) = 2;
              if (DAT_005b892c == 2) {
                *(undefined4 *)(local_28 + 0xa0) = 200;
              }
            }
            else if ((local_28 != 0) && (*(undefined4 *)(local_28 + 0xa0) = 1, DAT_005b892c == 2)) {
              *(undefined4 *)(local_28 + 0xa0) = 199;
            }
          }
        }
        local_10 = *(int **)(param_1 + 0x27a90);
        local_c = *(char *)(DAT_013b71e8 + local_8 + 0xef6);
        if ((local_c < '\0') || ('g' < local_c)) {
          local_c0 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x450);
          FUN_0058f078(local_140,&DAT_005b7d80,0);
          if (local_10 != (int *)0x0) {
            (**(code **)(*local_10 + 0x80))(local_140,0);
          }
          if (local_c0 != (int *)0x0) {
            (**(code **)(*local_c0 + 0x80))(local_140,0);
          }
        }
        else {
          FUN_004431d9();
          local_2c = (int)*(short *)(DAT_013b71e8 + 0x778);
          local_b8 = DAT_013b71e8 + 0x6ec;
          local_bc = DAT_005d03e4;
          if (DAT_005d03e4 == 3) {
            local_bc = 2;
          }
          iVar2 = __ftol();
          if ((((iVar2 >> 7 < 0x1b) || (iVar2 = __ftol(), 0x1e < iVar2 >> 7)) ||
              (iVar2 = __ftol(), iVar2 >> 7 < 0x15)) || (iVar2 = __ftol(), 0x18 < iVar2 >> 7)) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            local_bc = 2;
          }
          uVar3 = FUN_00442ff7((int)*(short *)(DAT_013b71e8 + 0xc4 +
                                              *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
          local_b4 = FUN_00557aff((int)local_c,local_b8,local_bc,uVar3);
          local_30 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x450);
          if (local_b4 < 0) {
            FUN_0058f078(local_b0,&DAT_005b7d7c,-local_b4);
            if (local_10 != (int *)0x0) {
              (**(code **)(*local_10 + 0x80))(local_b0,0);
            }
            if (local_30 != (int *)0x0) {
              (**(code **)(*local_30 + 0x80))(local_b0,0);
            }
            if (local_10 != (int *)0x0) {
              (**(code **)(*local_10 + 0x84))(0xffaaffaa);
            }
            if (local_30 != (int *)0x0) {
              (**(code **)(*local_30 + 0x84))(0xffaaffaa);
            }
          }
          else {
            FUN_0058f078(local_b0,&DAT_005b7d78,local_b4);
            if (local_10 != (int *)0x0) {
              (**(code **)(*local_10 + 0x80))(local_b0,0);
            }
            if (local_30 != (int *)0x0) {
              (**(code **)(*local_30 + 0x80))(local_b0,0);
            }
            if (local_10 != (int *)0x0) {
              (**(code **)(*local_10 + 0x84))(0xffbbbbff);
            }
            if (local_30 != (int *)0x0) {
              (**(code **)(*local_30 + 0x84))(0xffbbbbff);
            }
          }
        }
      }
      if (param_3 == 0) {
        iVar2 = FUN_0040c0f0();
        if (iVar2 == 0) {
          return 0;
        }
        *(undefined1 *)(DAT_013b71e8 + 0xef5) = (undefined1)local_8;
      }
      else {
        local_148 = (int)*(char *)(DAT_013b71e8 + 0xef5);
        local_144 = local_8;
        iVar2 = local_144;
        local_144._0_1_ = (undefined1)local_8;
        *(undefined1 *)(DAT_013b71e8 + 0xef5) = (undefined1)local_144;
        if ((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) != 2) &&
           (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) != 3)) {
          local_144 = iVar2;
          FUN_0049bcb8(&local_154);
          uVar3 = (**(code **)(*DAT_0092e654 + 8))(1,0);
          FUN_004595ec(*(undefined4 *)(param_1 + 0x275d0),*(undefined4 *)(param_1 + 0x275d4),
                       local_154,local_150,local_14c,uVar3);
        }
        *(undefined1 *)(DAT_013b71e8 + 0xef5) = (undefined1)local_148;
      }
      return 1;
    }
  }
  else if (((((('/' < param_2) && (param_2 < ':')) ||
             ((param_2 == '!' || (((param_2 == '@' || (param_2 == '#')) || (param_2 == '$')))))) ||
            ((param_2 == '%' || (param_2 == '^')))) || (param_2 == '&')) ||
          (((param_2 == '*' || (param_2 == '(')) || (param_2 == ')')))) {
    if (*(int *)(param_1 + 0x26eb0) != 1) {
      if ((param_2 < '0') || ('9' < param_2)) {
        if (param_2 == '!') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 0;
        }
        else if (param_2 == '@') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 1;
        }
        else if (param_2 == '#') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 2;
        }
        else if (param_2 == '$') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 3;
        }
        else if (param_2 == '%') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 4;
        }
        else if (param_2 == '^') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 5;
        }
        else if (param_2 == '&') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 6;
        }
        else if (param_2 == '*') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 7;
        }
        else if (param_2 == '(') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 8;
        }
        else if (param_2 == ')') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 9;
        }
      }
      else {
        *(char *)(DAT_013b71e8 + 0xef5) = param_2 + -0x31;
        if (param_2 == '0') {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = 9;
        }
      }
      iVar2 = FUN_0040c0f0();
      if (iVar2 == 1) {
        *(char *)(DAT_013b71e8 + 0xef5) = *(char *)(DAT_013b71e8 + 0xef5) + '\n';
      }
      if (*(char *)(DAT_013b71e8 + 0xef5) < '\n') {
        local_164 = *(int **)(param_1 + 0x28718);
        for (local_160 = 0; local_160 < 10; local_160 = local_160 + 1) {
          local_168 = (**(code **)(*local_164 + 0xb4))(local_160,0);
          if ((local_160 == *(char *)(DAT_013b71e8 + 0xef5)) && (local_168 != 0)) {
            *(undefined4 *)(local_168 + 0xa0) = 2;
            if (DAT_005b892c == 2) {
              *(undefined4 *)(local_168 + 0xa0) = 200;
            }
          }
          else if ((local_168 != 0) && (*(undefined4 *)(local_168 + 0xa0) = 1, DAT_005b892c == 2)) {
            *(undefined4 *)(local_168 + 0xa0) = 199;
          }
        }
      }
      else {
        local_170 = *(int **)(param_1 + 0x2871c);
        for (local_16c = 0; local_16c < 10; local_16c = local_16c + 1) {
          local_174 = (**(code **)(*local_170 + 0xb4))(local_16c,0);
          if ((local_16c + 10 == (int)*(char *)(DAT_013b71e8 + 0xef5)) && (local_174 != 0)) {
            *(undefined4 *)(local_174 + 0xa0) = 2;
            if (DAT_005b892c == 2) {
              *(undefined4 *)(local_174 + 0xa0) = 200;
            }
          }
          else if ((local_174 != 0) && (*(undefined4 *)(local_174 + 0xa0) = 1, DAT_005b892c == 2)) {
            *(undefined4 *)(local_174 + 0xa0) = 199;
          }
        }
      }
      local_15c = *(int **)(param_1 + 0x27a90);
      local_158 = *(char *)(DAT_013b71e8 + 0xef6 + (int)*(char *)(DAT_013b71e8 + 0xef5));
      if ((local_158 < '\0') || ('g' < local_158)) {
        local_20c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x450);
        FUN_0058f078(local_28c,&DAT_005b7d8c,0);
        (**(code **)(*local_15c + 0x80))(local_28c,0);
        (**(code **)(*local_20c + 0x80))(local_28c,0);
      }
      else {
        FUN_004431d9();
        local_178 = (int)*(short *)(DAT_013b71e8 + 0x778);
        local_204 = DAT_013b71e8 + 0x6ec;
        local_208 = DAT_005d03e4;
        if (DAT_005d03e4 == 3) {
          local_208 = 2;
        }
        iVar2 = __ftol();
        if ((((iVar2 >> 7 < 0x1b) || (iVar2 = __ftol(), 0x1e < iVar2 >> 7)) ||
            (iVar2 = __ftol(), iVar2 >> 7 < 0x15)) || (iVar2 = __ftol(), 0x18 < iVar2 >> 7)) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) {
          local_208 = 2;
        }
        uVar3 = FUN_00442ff7((int)*(short *)(DAT_013b71e8 + 0xc4 +
                                            *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
        local_200 = FUN_00557aff((int)local_158,local_204,local_208,uVar3);
        local_17c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x450);
        if (local_200 < 0) {
          FUN_0058f078(local_1fc,&DAT_005b7d88,-local_200);
          (**(code **)(*local_15c + 0x80))(local_1fc,0);
          (**(code **)(*local_17c + 0x80))(local_1fc,0);
          (**(code **)(*local_15c + 0x84))(0xffaaffaa);
          (**(code **)(*local_17c + 0x84))(0xffaaffaa);
        }
        else {
          FUN_0058f078(local_1fc,&DAT_005b7d84,local_200);
          (**(code **)(*local_15c + 0x80))(local_1fc,0);
          (**(code **)(*local_17c + 0x80))(local_1fc,0);
          (**(code **)(*local_15c + 0x84))(0xffbbbbff);
          (**(code **)(*local_17c + 0x84))(0xffbbbbff);
        }
      }
      return 1;
    }
    *(undefined4 *)(param_1 + 0x26eb0) = 0;
    return 1;
  }
  return 0;
}

