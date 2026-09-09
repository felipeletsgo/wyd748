// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00454763 | Name: FUN_00454763


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00454763(int param_1,uint param_2)

{
  bool bVar1;
  int iVar2;
  SHORT SVar3;
  undefined2 uVar4;
  undefined4 uVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  int local_244;
  int local_240;
  short local_22c;
  undefined1 local_220 [4];
  undefined2 local_21c;
  undefined2 local_21a;
  undefined4 local_214;
  int local_210;
  int local_20c;
  int local_208;
  int local_204;
  int local_200;
  int *local_1fc;
  undefined1 local_1f8 [128];
  int *local_178;
  int local_174;
  int local_170;
  int local_16c;
  undefined1 local_168 [128];
  int *local_e8;
  int local_e4;
  int local_e0;
  int *local_dc;
  int local_d8;
  int local_d4;
  int *local_d0;
  int local_cc;
  int *local_c8;
  char local_c4;
  undefined4 local_c0;
  int local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  int *local_a8;
  uint local_a4;
  float local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  uint local_94;
  float local_90;
  int local_8c;
  undefined1 local_88 [4];
  undefined2 local_84;
  undefined2 local_82;
  undefined1 local_7c [116];
  uint local_8;
  
  local_8 = (**(code **)(*DAT_0092e654 + 8))();
  if (local_8 < DAT_013b7220 + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27138) + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x2713c) + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27140) + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27148) + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27144) + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x2714c) + 6000U) {
    uVar5 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27150) + 6000U) {
    uVar5 = 1;
  }
  else {
    iVar6 = FUN_0049abde(param_2);
    if (iVar6 == 1) {
      uVar5 = 1;
    }
    else {
      if (param_2 == 0x2d) {
        _memset(local_88,0,0x80);
        local_82 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
        local_84 = 0x334;
        FUN_0058f078(local_7c,&DAT_005b7d90);
        FUN_0055f2dd(local_88,0x80);
      }
      iVar6 = FUN_0040c0f0();
      if (((iVar6 != 0) && (0x6f < param_2)) && (param_2 < 0x7a)) {
        if (local_8 < *(int *)(param_1 + 0x2746c) + 500U) {
          return 1;
        }
        SVar3 = GetKeyState(0x10);
        local_94 = CONCAT22(local_94._2_2_,SVar3);
        local_90 = DAT_005b490c;
        local_a0 = DAT_005b4910;
        local_9c = 8;
        local_98 = 1;
        if (DAT_005b892c == 2) {
          local_90 = 1.0;
          local_a0 = 1.0;
          local_9c = 4;
          local_98 = 2;
        }
        local_8c = 0;
        iVar6 = FUN_0040c0f0();
        if (iVar6 == 1) {
          local_8c = 10;
        }
        if ((int)(local_94 & 0xffff) >> 8 != 0) {
          for (local_a4 = 0; local_a4 < 0x19; local_a4 = local_a4 + 1) {
            local_a8 = *(int **)(param_1 + 0x279bc + local_a4 * 4);
            if (local_a8 != (int *)0x0) {
              uVar5 = __ftol(local_a8[0x1a],local_a8[0x1b],local_90 * (float)local_a8[0x1c],
                             local_a0 * (float)local_a8[0x1d]);
              uVar5 = __ftol(uVar5);
              iVar6 = FUN_00401000(uVar5);
              if (iVar6 == 1) {
                local_ac = (**(code **)(*local_a8 + 0xb4))(0,0);
                FUN_0044e494(local_8c + -0x70 + param_2,local_ac);
                break;
              }
            }
          }
          uVar5 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_90 * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_a0 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar5 = __ftol(uVar5);
          iVar6 = FUN_00401000(uVar5);
          if (iVar6 == 1) {
            local_b8 = __ftol();
            local_c0 = __ftol();
            local_b0 = __ftol();
            local_b4 = __ftol();
            local_bc = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(local_b0,local_b4);
            if (local_bc != 0) {
              FUN_0044e494(local_8c + -0x70 + param_2,local_bc);
            }
          }
          *(uint *)(param_1 + 0x2746c) = local_8;
        }
      }
      if ((param_2 < 0x70) || (0x79 < param_2)) {
        if (param_2 == 0x7a) {
          if ((local_8 < *(int *)(param_1 + 0x2746c) + 500U) || (*(int *)(param_1 + 0x26e1c) == 1))
          {
            uVar5 = 1;
          }
          else if (((*(int *)(param_1 + 0x4c) == 0) ||
                   (_DAT_005a40f4 <= *(float *)(*(int *)(param_1 + 0x4c) + 0x230))) ||
                  (*(float *)(*(int *)(param_1 + 0x4c) + 0x230) <= _DAT_005a3534)) {
            local_1fc = *(int **)(param_1 + 0x2873c);
            local_20c = 0;
            local_208 = 0;
            local_200 = 0;
            for (local_204 = 8; -1 < local_204; local_204 = local_204 + -1) {
              for (local_208 = 6; -1 < local_208; local_208 = local_208 + -1) {
                local_20c = (**(code **)(*local_1fc + 0xb4))(local_204,local_208);
                if ((local_20c != 0) &&
                   (iVar6 = FUN_0054cd07(*(undefined4 *)(local_20c + 0x670),0x26), iVar6 == 0xb)) {
                  local_200 = 1;
                  break;
                }
              }
              if (local_200 == 1) break;
            }
            if ((local_200 == 1) && (local_20c != 0)) {
              local_210 = FUN_0054cd07(*(undefined4 *)(local_20c + 0x670),0x26);
              if (local_210 == 0xb) {
                (**(code **)(*local_1fc + 0xc0))
                          (*(undefined4 *)(*(int *)(local_20c + 0x1e4) + 0x1e4),
                           *(undefined4 *)(*(int *)(local_20c + 0x1e4) + 0x400));
                local_22c = (**(code **)(*local_1fc + 0xc4))
                                      (*(undefined4 *)(*(int *)(local_20c + 0x1e4) + 0x1e4));
                if (local_22c == -1) {
                  local_22c = (short)*(undefined4 *)(local_20c + 0x1d0) +
                              (short)*(undefined4 *)(local_20c + 0x1d4) * 9;
                }
                uVar5 = (**(code **)(*DAT_0092e654 + 8))();
                *(undefined4 *)(param_1 + 0x27474) = uVar5;
                *(undefined4 *)(param_1 + 0x27144) = *(undefined4 *)(param_1 + 0x27474);
                *(undefined1 *)(param_1 + 0x271d1) = 1;
                local_21a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
                local_21c = 0x3ae;
                local_214 = 1;
                FUN_0055f2dd(local_220,0x10);
                _memset((void *)(param_1 + 0x27154),0,0x24);
                *(undefined2 *)(param_1 + 0x2715a) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
                *(undefined2 *)(param_1 + 0x27158) = 0x373;
                *(undefined4 *)(param_1 + 0x27160) = 1;
                *(int *)(param_1 + 0x27164) = (int)local_22c;
                *(undefined2 *)(param_1 + 0x27174) = 0;
                uVar4 = __ftol();
                *(undefined2 *)(param_1 + 0x27170) = uVar4;
                uVar4 = __ftol();
                *(undefined2 *)(param_1 + 0x27172) = uVar4;
                iVar6 = FUN_0054e93b(*(undefined4 *)(local_20c + 0x670));
                if (iVar6 < 2) {
                  puVar7 = (undefined4 *)(**(code **)(*local_1fc + 0xa4))(local_204,local_208);
                  if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
                     (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar7)) {
                    *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
                  }
                  if ((puVar7 != (undefined4 *)0x0) && (puVar7 != (undefined4 *)0x0)) {
                    (**(code **)*puVar7)(1);
                  }
                }
                else {
                  FUN_0054e9ab(*(undefined4 *)(local_20c + 0x670),iVar6 + -1);
                  iVar2 = local_20c;
                  FUN_0058f078(local_20c + 0x39a,&PTR_DAT_005b7da4,iVar6 + -1);
                  FUN_00421b8f(iVar2 + 0x39a,*(undefined4 *)(iVar2 + 900),0);
                }
                if (iVar6 < 2) {
                  _memset((void *)(DAT_013b71e8 + 0x7c8 + local_22c * 8),0,8);
                }
                if ((DAT_005ccf98 != 0) && (iVar6 = FUN_00429a6d(0x36), iVar6 != 0)) {
                  uVar8 = 0;
                  uVar5 = 0;
                  FUN_00429a6d(0x36);
                  FUN_0042ad2b(uVar5,uVar8);
                }
              }
              FUN_004431e4(0);
              *(uint *)(param_1 + 0x2746c) = local_8;
            }
            uVar5 = 1;
          }
          else {
            uVar5 = 1;
          }
        }
        else if ((param_2 < 0x60) || (0x69 < param_2)) {
          if (param_2 == 0x21) {
            if ((*(int *)(param_1 + 0x27c88) == 0) || (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
              local_240 = *(int *)(param_1 + 0x27ae8);
            }
            else {
              local_240 = *(int *)(param_1 + 0x27c8c);
            }
            if (((local_240 != 0) && (iVar6 = FUN_0040c0f0(), iVar6 == 1)) &&
               (*(int *)(local_240 + 0x20c) != 0)) {
              (**(code **)(**(int **)(local_240 + 0x20c) + 0x90))();
            }
            uVar5 = 1;
          }
          else if (param_2 == 0x22) {
            if ((*(int *)(param_1 + 0x27c88) == 0) || (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
              local_244 = *(int *)(param_1 + 0x27ae8);
            }
            else {
              local_244 = *(int *)(param_1 + 0x27c8c);
            }
            if (((local_244 != 0) && (iVar6 = FUN_0040c0f0(), iVar6 == 1)) &&
               (*(int *)(local_244 + 0x20c) != 0)) {
              (**(code **)(**(int **)(local_244 + 0x20c) + 0x94))();
            }
            uVar5 = 1;
          }
          else {
            uVar5 = 0;
          }
        }
        else {
          uVar5 = FUN_00455950(param_2);
        }
      }
      else {
        *(char *)(DAT_013b71e8 + 0xef5) = (char)param_2 + -0x70;
        iVar6 = FUN_0040c0f0();
        if (iVar6 == 1) {
          *(char *)(DAT_013b71e8 + 0xef5) = *(char *)(DAT_013b71e8 + 0xef5) + '\n';
        }
        if (*(char *)(DAT_013b71e8 + 0xef5) < '\n') {
          local_d0 = *(int **)(param_1 + 0x28718);
          for (local_cc = 0; local_cc < 10; local_cc = local_cc + 1) {
            local_d4 = (**(code **)(*local_d0 + 0xb4))(local_cc,0);
            if ((local_cc == *(char *)(DAT_013b71e8 + 0xef5)) && (local_d4 != 0)) {
              *(undefined4 *)(local_d4 + 0xa0) = 2;
              if (DAT_005b892c == 2) {
                *(undefined4 *)(local_d4 + 0xa0) = 200;
              }
            }
            else if ((local_d4 != 0) && (*(undefined4 *)(local_d4 + 0xa0) = 1, DAT_005b892c == 2)) {
              *(undefined4 *)(local_d4 + 0xa0) = 199;
            }
          }
        }
        else {
          local_dc = *(int **)(param_1 + 0x2871c);
          for (local_d8 = 0; local_d8 < 10; local_d8 = local_d8 + 1) {
            local_e0 = (**(code **)(*local_dc + 0xb4))(local_d8,0);
            if ((local_d8 + 10 == (int)*(char *)(DAT_013b71e8 + 0xef5)) && (local_e0 != 0)) {
              *(undefined4 *)(local_e0 + 0xa0) = 2;
              if (DAT_005b892c == 2) {
                *(undefined4 *)(local_e0 + 0xa0) = 200;
              }
            }
            else if ((local_e0 != 0) && (*(undefined4 *)(local_e0 + 0xa0) = 1, DAT_005b892c == 2)) {
              *(undefined4 *)(local_e0 + 0xa0) = 199;
            }
          }
        }
        local_c8 = *(int **)(param_1 + 0x27a90);
        local_c4 = *(char *)(DAT_013b71e8 + 0xef6 + (int)*(char *)(DAT_013b71e8 + 0xef5));
        if ((local_c4 < '\0') || ('g' < local_c4)) {
          local_178 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x450);
          FUN_0058f078(local_1f8,&DAT_005b7da0,0);
          if (local_c8 != (int *)0x0) {
            (**(code **)(*local_c8 + 0x80))(local_1f8,0);
          }
          if (local_178 != (int *)0x0) {
            (**(code **)(*local_178 + 0x80))(local_1f8,0);
          }
        }
        else {
          FUN_004431d9();
          local_e4 = (int)*(short *)(DAT_013b71e8 + 0x778);
          local_170 = DAT_013b71e8 + 0x6ec;
          local_174 = DAT_005d03e4;
          if (DAT_005d03e4 == 3) {
            local_174 = 2;
          }
          iVar6 = __ftol();
          if ((((iVar6 >> 7 < 0x1b) || (iVar6 = __ftol(), 0x1e < iVar6 >> 7)) ||
              (iVar6 = __ftol(), iVar6 >> 7 < 0x15)) || (iVar6 = __ftol(), 0x18 < iVar6 >> 7)) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            local_174 = 2;
          }
          uVar5 = FUN_00442ff7((int)*(short *)(DAT_013b71e8 + 0xc4 +
                                              *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
          local_16c = FUN_00557aff((int)local_c4,local_170,local_174,uVar5);
          local_e8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x450);
          if (local_16c < 0) {
            FUN_0058f078(local_168,&DAT_005b7d9c,-local_16c);
            if (local_c8 != (int *)0x0) {
              (**(code **)(*local_c8 + 0x80))(local_168,0);
            }
            if (local_e8 != (int *)0x0) {
              (**(code **)(*local_e8 + 0x80))(local_168,0);
            }
            if (local_c8 != (int *)0x0) {
              (**(code **)(*local_c8 + 0x84))(0xffaaffaa);
            }
            if (local_e8 != (int *)0x0) {
              (**(code **)(*local_e8 + 0x84))(0xffaaffaa);
            }
          }
          else {
            FUN_0058f078(local_168,&DAT_005b7d98,local_16c);
            if (local_c8 != (int *)0x0) {
              (**(code **)(*local_c8 + 0x80))(local_168,0);
            }
            if (local_e8 != (int *)0x0) {
              (**(code **)(*local_e8 + 0x80))(local_168,0);
            }
            if (local_c8 != (int *)0x0) {
              (**(code **)(*local_c8 + 0x84))(0xffbbbbff);
            }
            if (local_e8 != (int *)0x0) {
              (**(code **)(*local_e8 + 0x84))(0xffbbbbff);
            }
          }
        }
        uVar5 = 1;
      }
    }
  }
  return uVar5;
}

