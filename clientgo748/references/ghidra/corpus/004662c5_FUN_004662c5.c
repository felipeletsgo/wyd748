// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004662c5 | Name: FUN_004662c5


/* WARNING: Removing unreachable block (ram,0x0046920a) */
/* WARNING: Removing unreachable block (ram,0x0046913a) */
/* WARNING: Removing unreachable block (ram,0x004692e0) */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_004662c5(uint param_1,int param_2)

{
  POINT pt;
  undefined1 uVar1;
  short sVar2;
  undefined2 uVar3;
  uint uVar4;
  char *pcVar5;
  int iVar6;
  void *pvVar7;
  size_t sVar8;
  undefined4 uVar9;
  int *extraout_ECX;
  int *piVar10;
  int *piVar11;
  undefined4 local_1b14;
  undefined1 *local_1b04;
  undefined1 *local_1b00;
  undefined4 local_1afc;
  int local_1aec;
  int local_1ae8;
  int local_1ae4;
  int local_1ae0;
  int local_1adc;
  undefined1 local_1a78 [4];
  undefined2 local_1a74;
  undefined2 local_1a72;
  int local_1a6c;
  int local_1a68;
  int *local_1a64;
  int *local_1960;
  undefined4 local_195c;
  undefined4 local_1958;
  undefined4 local_1954;
  undefined4 local_1950;
  int local_194c;
  int *local_1948;
  int local_1944;
  int *local_1940;
  int local_193c;
  int *local_1938;
  int local_1934;
  undefined2 local_192c;
  int local_1894;
  undefined1 local_1890 [4];
  undefined2 local_188c;
  undefined2 local_188a;
  undefined4 local_1884;
  int local_1880;
  int local_187c;
  int local_1878;
  int local_1874;
  byte local_1870;
  undefined3 uStackY_186f;
  int local_186c;
  int local_1868;
  byte local_1864;
  undefined3 uStackY_1863;
  int local_1860;
  int local_185c;
  int local_1858;
  undefined1 local_1854 [4];
  undefined2 local_1850;
  undefined2 local_184e;
  undefined2 local_1848;
  undefined2 local_1844;
  undefined2 local_1842;
  undefined2 local_1840;
  undefined2 local_183e;
  short local_183c;
  undefined2 local_183a;
  undefined1 local_1838;
  undefined1 local_1837;
  undefined2 local_1828;
  undefined2 local_1826;
  undefined2 local_17ee;
  undefined1 local_17d6;
  byte local_17d5;
  undefined4 local_1794;
  int local_1790;
  int local_178c;
  uint local_1788;
  char local_1784;
  int local_1780;
  undefined1 local_16fc [4];
  undefined2 local_16f8;
  undefined2 local_16f6;
  undefined2 local_16f0;
  int local_16dc;
  int local_16d8;
  int *local_16d4;
  int local_16d0;
  undefined1 local_16cc [4];
  undefined2 local_16c8;
  undefined2 local_16c6;
  undefined2 local_16c0;
  undefined2 local_16be;
  int local_16b8;
  undefined1 local_16b4 [4];
  undefined2 local_16b0;
  undefined2 local_16ae;
  undefined2 local_16a8;
  undefined2 local_16a6;
  int local_16a0;
  undefined1 local_169c [4];
  undefined2 local_1698;
  undefined2 local_1696;
  undefined2 local_1690;
  undefined2 local_168e;
  int local_1688;
  undefined1 local_1684 [4];
  undefined2 local_1680;
  undefined2 local_167e;
  undefined2 local_1678;
  undefined2 local_1676;
  undefined1 local_1670 [4];
  undefined2 local_166c;
  undefined2 local_166a;
  undefined2 local_1664;
  undefined2 local_1662;
  undefined1 local_165c [4];
  undefined2 local_1658;
  undefined2 local_1656;
  undefined2 local_1650;
  undefined2 local_164e;
  undefined1 local_1648 [4];
  undefined2 local_1644;
  undefined2 local_1642;
  undefined2 local_163c;
  undefined2 local_163a;
  undefined1 local_1634 [4];
  undefined2 local_1630;
  undefined2 local_162e;
  undefined2 local_1628;
  undefined2 local_1626;
  undefined1 local_1620 [4];
  undefined2 local_161c;
  undefined2 local_161a;
  undefined4 local_1614;
  undefined4 local_1610;
  int local_160c;
  int local_1608;
  int local_1604;
  int local_1600;
  undefined1 local_15fc [4];
  undefined2 local_15f8;
  undefined2 local_15f6;
  undefined4 local_15f0;
  undefined4 local_15ec;
  LONG local_15e8;
  LONG local_15e4;
  int local_15e0;
  RECT local_15dc;
  BOOL local_15cc;
  int local_15c8;
  undefined1 local_15c4 [4];
  undefined2 local_15c0;
  undefined2 local_15be;
  char local_15b8;
  undefined1 local_15b7;
  undefined2 local_15b6;
  undefined2 local_15b4;
  undefined2 local_15b2;
  undefined2 local_15b0;
  uint local_159c;
  undefined2 local_1594;
  undefined2 local_1592;
  undefined4 local_158c;
  int local_1588;
  undefined4 *local_1584;
  int local_1580;
  undefined4 local_157c;
  undefined4 local_1578;
  undefined2 local_1570;
  undefined2 local_156e;
  undefined4 local_1568;
  int local_1564;
  int local_1560;
  int local_155c;
  int *local_1558;
  int local_1554;
  int local_1550;
  int local_154c;
  int local_1548;
  undefined2 local_1540;
  undefined2 local_153e;
  undefined4 local_1538;
  int local_1534;
  char local_1530 [128];
  undefined1 *local_14b0;
  size_t local_14ac;
  size_t local_14a8;
  size_t local_14a4;
  size_t local_14a0;
  int local_149c;
  char local_1498 [14];
  undefined1 local_148a;
  undefined1 *local_1418 [12];
  int local_13e8;
  undefined1 local_13e4 [4];
  undefined1 local_13e0 [4];
  undefined1 local_13dc [4];
  char local_13d8 [4];
  undefined1 local_13d4 [4];
  int local_13d0;
  undefined1 local_13cc [20];
  int local_13b8;
  int local_13b4;
  int local_13b0;
  undefined2 local_13a8;
  undefined2 local_13a6;
  undefined4 local_13a0;
  undefined1 local_139c [1024];
  int local_f9c;
  int local_f98;
  int local_f94;
  int *local_f90;
  int local_f8c;
  int local_f88 [10];
  uint local_f60;
  _SYSTEMTIME local_f5c;
  int local_f4c;
  undefined1 local_f48 [4];
  undefined1 local_f44 [4];
  char local_f40 [4];
  undefined1 local_f3c [4];
  int local_f38;
  undefined1 local_f34 [24];
  undefined2 local_f1c;
  undefined2 local_f1a;
  undefined4 local_f14;
  undefined1 local_f10 [4];
  undefined2 local_f0c;
  undefined2 local_f0a;
  undefined1 local_ea4 [4];
  undefined2 local_ea0;
  undefined2 local_e9e;
  undefined1 local_e38 [4];
  undefined2 local_e34;
  undefined2 local_e32;
  undefined1 local_dcc [4];
  undefined2 local_dc8;
  undefined2 local_dc6;
  int local_d60;
  int *local_d5c;
  int *local_d58;
  int *local_d54;
  int *local_d50;
  int local_d4c;
  int *local_d48;
  int *local_d44;
  int local_d40;
  undefined1 local_d3c [4];
  undefined2 local_d38;
  undefined2 local_d36;
  undefined4 local_d30;
  undefined1 local_d2c [4];
  undefined2 local_d28;
  undefined2 local_d26;
  undefined4 local_d20;
  int local_d1c;
  int local_d18;
  undefined1 local_d14 [4];
  undefined2 local_d10;
  undefined2 local_d0e;
  undefined4 local_c94;
  char local_c90 [14];
  undefined1 local_c82;
  undefined1 local_c81;
  char *local_c10;
  undefined1 local_c0c [4];
  undefined2 local_c08;
  undefined2 local_c06;
  undefined4 local_c00;
  undefined1 local_bfc [4];
  undefined2 local_bf8;
  undefined2 local_bf6;
  int local_bf0;
  undefined4 local_bec;
  undefined4 local_be8;
  void *local_be4;
  undefined1 local_be0 [128];
  int *local_b60;
  int local_b5c;
  int *local_b58;
  int local_b54;
  int *local_b50;
  int local_b4c;
  int *local_ac8;
  int local_ac4;
  int *local_ac0;
  undefined4 *local_abc;
  int *local_ab8;
  int *local_ab4;
  int local_ab0;
  int *local_aac;
  int *local_aa8;
  int local_aa4;
  int *local_aa0;
  int local_a9c;
  int local_a98 [32];
  undefined1 local_a18 [4];
  undefined2 local_a14;
  undefined2 local_a12;
  undefined4 local_a0c;
  undefined1 local_a08 [4];
  undefined2 local_a04;
  undefined2 local_a02;
  undefined4 local_9fc;
  undefined4 local_8f8;
  undefined4 local_7f4;
  undefined4 local_7f0;
  undefined4 local_7ec;
  size_t local_768;
  size_t local_764;
  int *local_760;
  int local_75c;
  longlong local_758;
  char *local_750;
  int local_74c;
  int *local_748;
  int *local_744;
  int *local_740;
  int local_73c;
  int local_738;
  int local_734;
  undefined4 local_730;
  int *local_72c;
  int *local_728;
  int local_724;
  int local_720;
  undefined4 local_71c;
  undefined1 local_718 [160];
  undefined1 local_678 [4];
  undefined2 local_674;
  undefined2 local_672;
  char local_66c;
  int local_60c;
  int local_608;
  int local_604;
  int local_600;
  undefined2 local_5fc;
  undefined2 local_5fa;
  char local_5e4 [100];
  int local_580;
  char local_57c [14];
  undefined1 local_56e;
  undefined1 local_56d;
  char *local_4fc;
  undefined1 local_4f8 [4];
  undefined2 local_4f4;
  undefined2 local_4f2;
  undefined2 local_47c;
  int local_478;
  undefined1 local_474 [4];
  undefined2 local_470;
  undefined2 local_46e;
  undefined2 local_3f8;
  int local_3f4;
  int local_370;
  int local_2ec;
  int local_2e8;
  int local_2e4;
  int local_2e0;
  char local_2dc;
  char local_2db;
  undefined1 local_25c [128];
  int local_1dc;
  char *local_1d8;
  int local_1d4;
  int *local_1d0;
  int local_1cc;
  int local_1c8;
  int *local_1c4;
  int local_1c0;
  int local_1bc;
  int *local_1b8;
  undefined1 local_1b4 [4];
  undefined2 local_1b0;
  undefined2 local_1ae;
  undefined1 local_1a8;
  int local_1a4;
  int *local_1a0;
  int *local_15c;
  int local_158;
  int local_154;
  undefined4 local_150;
  char *local_14c;
  int local_148;
  undefined4 local_144;
  int local_c0;
  int local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  undefined4 uStackY_58;
  undefined1 *puStackY_54;
  char *pcStackY_50;
  int *piStackY_4c;
  undefined1 *puStackY_48;
  undefined1 *puStackY_44;
  char *pcStackY_40;
  undefined1 *puStackY_3c;
  int *piStackY_38;
  undefined1 *puStackY_34;
  char *pcVar12;
  undefined **ppuVar13;
  int iVar14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f733;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0058f730();
  uVar4 = (**(code **)(*DAT_0092e654 + 8))();
  if (uVar4 < DAT_013b7220 + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c4e] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c4f] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c50] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c52] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c51] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c53] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  if (uVar4 < extraout_ECX[0x9c54] + 6000U) {
    ExceptionList = local_10;
    return 1;
  }
  iVar14 = 0;
  if ((DAT_013b71e8 != (int *)0xfffff914) && ((DAT_013b71e8[0x272] & 0x20000000U) != 0)) {
    iVar14 = 1;
  }
  if (param_1 == 0x1214) {
    if (((*(int *)(extraout_ECX[10] + 300) != 0) &&
        (*(int *)(*(int *)(extraout_ECX[10] + 300) + 0x40) == 0xd)) &&
       (*(int *)(*(int *)(extraout_ECX[10] + 300) + 0x28) != 0)) {
      iVar14 = FUN_004aea2c();
      if (iVar14 == 0) {
        FUN_004aea8a();
      }
      else {
        FUN_004aeaeb();
      }
    }
  }
  else {
    if ((param_1 == 0x1403) && (param_2 == 8)) {
      ExceptionList = local_10;
      return 1;
    }
    if ((param_1 < 0x80b) || (0x837 < param_1)) {
      if (param_1 == 0x840) {
        FUN_005900c1();
        for (local_ac = 0; local_ac < 6; local_ac = local_ac + 1) {
          iVar14 = _rand();
          local_b0 = iVar14 % 0x2d + 1;
          local_b8 = 0;
          local_b4 = 0;
          while( true ) {
            if ((5 < local_b4) ||
               (local_bc = (**(code **)(*(int *)extraout_ECX[local_b4 + 0x9e2e] + 0x88))(),
               local_bc == 0)) goto LAB_0046670c;
            local_c0 = FUN_0059115a();
            if (local_b0 == local_c0) break;
            local_b4 = local_b4 + 1;
          }
          if (0 < local_ac) {
            local_ac = local_ac + -1;
          }
          local_b8 = 1;
LAB_0046670c:
          if (local_b8 == 0) {
            local_144 = (**(code **)(*(int *)extraout_ECX[local_ac + 0x9e2e] + 0x88))();
            FUN_0058f078();
            (**(code **)(*(int *)extraout_ECX[local_ac + 0x9e2e] + 0x80))();
          }
        }
      }
      else if (param_1 == 0x838) {
        for (local_148 = 0; local_148 < 6; local_148 = local_148 + 1) {
          local_14c = (char *)(**(code **)(*(int *)extraout_ECX[local_148 + 0x9e2e] + 0x88))();
          if ((local_14c != (char *)0x0) && (*local_14c == '\0')) {
            FUN_00403df2();
            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
            ExceptionList = local_10;
            return 1;
          }
        }
        for (local_148 = 0; local_148 < 6; local_148 = local_148 + 1) {
          local_150 = (**(code **)(*(int *)extraout_ECX[local_148 + 0x9e2e] + 0x88))();
          uVar1 = FUN_0059115a();
          *(undefined1 *)((int)extraout_ECX + local_148 + 0x26e6c) = uVar1;
        }
        FUN_0055f2dd();
        (**(code **)(*(int *)extraout_ECX[0x9e2d] + 0x60))();
        (**(code **)(*(int *)extraout_ECX[0xa175] + 0x60))();
      }
      else {
        if (param_1 == 0x839) {
          _memset(extraout_ECX + 0x9b95,0,0x20);
          (**(code **)(*(int *)extraout_ECX[0x9e2d] + 0x60))();
          FUN_00447691();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1830) {
          iVar14 = FUN_0040c0f0();
          if (iVar14 == 0) {
            (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
            *(int *)(extraout_ECX[0x23] + 0x1e8) = extraout_ECX[0x9d76];
            (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
            ExceptionList = local_10;
            return 1;
          }
          ExceptionList = local_10;
          return 1;
        }
        if ((((param_1 == 0x1831) || (param_1 == 0x1832)) || (param_1 == 0x1833)) ||
           (param_1 == 0x1834)) {
          if (param_1 == 0x1831) {
            if (extraout_ECX[0x9ef8] != 0) {
              FUN_0049e50f();
            }
          }
          else if (param_1 == 0x1832) {
            if (extraout_ECX[0x9ef8] != 0) {
              FUN_0049e50f();
            }
          }
          else if (param_1 == 0x1833) {
            if (extraout_ECX[0x9ef8] != 0) {
              FUN_0049e50f();
            }
          }
          else if ((param_1 == 0x1834) && (extraout_ECX[0x9ef8] != 0)) {
            FUN_0049e50f();
          }
          extraout_ECX[0x9d77] = param_1;
          local_158 = extraout_ECX[0x9ef7];
          local_15c = (int *)0x0;
          for (local_154 = 0; local_154 < 4; local_154 = local_154 + 1) {
            local_15c = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            if (local_15c != (int *)0x0) {
              if (local_154 + 0x1831U == param_1) {
                (**(code **)(*local_15c + 0x8c))();
              }
              else {
                (**(code **)(*local_15c + 0x8c))();
              }
            }
          }
          ExceptionList = local_10;
          return 1;
        }
        if (((param_1 == 0x1902) || (param_1 == 0x1903)) ||
           ((param_1 == 0x1904 || (param_1 == 0x1905)))) {
          if (param_1 == 0x1902) {
            extraout_ECX[0x9dac] = extraout_ECX[0x9dac] + -10000;
            if (extraout_ECX[0x9dac] < 1000) {
              extraout_ECX[0x9dac] = 1000;
            }
          }
          else if (param_1 == 0x1903) {
            extraout_ECX[0x9dac] = extraout_ECX[0x9dac] + -1000;
            if (extraout_ECX[0x9dac] < 1000) {
              extraout_ECX[0x9dac] = 1000;
            }
          }
          else if (param_1 == 0x1904) {
            if (DAT_013b71e8[0x1c1] - extraout_ECX[0x9dac] < 10000) {
              ExceptionList = local_10;
              return 1;
            }
            extraout_ECX[0x9dac] = extraout_ECX[0x9dac] + 10000;
            if (100000 < extraout_ECX[0x9dac]) {
              extraout_ECX[0x9dac] = 100000;
            }
          }
          else if (param_1 == 0x1905) {
            if (DAT_013b71e8[0x1c1] - extraout_ECX[0x9dac] < 1000) {
              ExceptionList = local_10;
              return 1;
            }
            extraout_ECX[0x9dac] = extraout_ECX[0x9dac] + 1000;
            if (100000 < extraout_ECX[0x9dac]) {
              extraout_ECX[0x9dac] = 100000;
            }
          }
          local_1a0 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
          FUN_0058f078();
          (**(code **)(*local_1a0 + 0x80))();
          puStackY_34 = (undefined1 *)0x466e66;
          FUN_0058f078();
          *(undefined1 *)(extraout_ECX[0x9e4a] + 0xcdd) = 1;
          (**(code **)(*(int *)extraout_ECX[0x9e4a] + 0x80))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1901) {
          if ((*(int *)(extraout_ECX[0xa268] + 0x388) != 0) ||
             (*(char *)(extraout_ECX[0xa268] + 0x369) == '\x01')) {
            ExceptionList = local_10;
            return 1;
          }
          if ((*(int *)(extraout_ECX[0xa267] + 0x388) == 0) &&
             (*(char *)(extraout_ECX[0xa267] + 0x369) != '\x01')) {
            local_1c4 = (int *)extraout_ECX[0xa1cf];
            (**(code **)(*local_1c4 + 0x98))();
            if ((-1 < local_1c0) && (-1 < local_1bc)) {
              local_1b8 = DAT_013b71e8 + 0x1bb;
              if (DAT_013b71e8[0x1c1] - extraout_ECX[0x9dac] < 0) {
                ExceptionList = local_10;
                return 1;
              }
              puStackY_34 = (undefined1 *)0x4670a1;
              _memset(local_1b4,0,0x14);
              local_1ae = (undefined2)DAT_013b71e8[0x317];
              local_1b0 = 0x2be;
              local_1a8 = 1;
              local_1a4 = extraout_ECX[0x9dac];
              FUN_0055f2dd();
              *(int *)(extraout_ECX[0xa267] + 0x3b0) = extraout_ECX[0x9dac];
              *(int *)(extraout_ECX[0xa268] + 0x3b0) = extraout_ECX[0x9dac];
              puStackY_34 = (undefined1 *)0x0;
              piStackY_38 = (int *)0x1;
              puStackY_3c = (undefined1 *)0x467145;
              FUN_0040a8de();
              puStackY_34 = (undefined1 *)0x0;
              piStackY_38 = (int *)0x1;
              puStackY_3c = (undefined1 *)0x467163;
              FUN_0040a8de();
              DAT_013b71e8[0x1c1] = DAT_013b71e8[0x1c1] - extraout_ECX[0x9dac];
              if ((DAT_005ccf98 != 0) && (iVar14 = FUN_00429a6d(), iVar14 != 0)) {
                puStackY_34 = (undefined1 *)0x4671b8;
                FUN_00429a6d();
                FUN_0042ad2b();
              }
              FUN_004431e4();
              ExceptionList = local_10;
              return 1;
            }
            local_1c8 = extraout_ECX[0x9eba];
            pvVar7 = operator_new(0xe50);
            local_8 = 0;
            if (pvVar7 == (void *)0x0) {
              local_1adc = 0;
            }
            else {
              puStackY_34 = (undefined1 *)0x0;
              piStackY_38 = (int *)0x41800000;
              puStackY_3c = (undefined1 *)0x43960000;
              pcStackY_40 = (char *)0x0;
              puStackY_44 = (undefined1 *)0x0;
              puStackY_48 = (undefined1 *)0xffffaaaa;
              piStackY_4c = (int *)&DAT_00a341f8;
              pcStackY_50 = (char *)0x466fd6;
              local_1adc = FUN_00407203();
            }
            local_8 = 0xffffffff;
            local_1cc = local_1adc;
            if (local_1adc != 0) {
              FUN_00408d33();
            }
            if ((DAT_005ccf98 != 0) && (iVar14 = FUN_00429a6d(), iVar14 != 0)) {
              puStackY_34 = (undefined1 *)0x46704d;
              FUN_00429a6d();
              FUN_0042ad2b();
            }
            ExceptionList = local_10;
            return 1;
          }
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1915) {
          FUN_0044a745();
        }
        else if ((param_1 == 0x1403) && (param_2 == 0)) {
          local_1d0 = (int *)extraout_ECX[0x9ecb];
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          sVar8 = _strlen(pcVar5);
          if (sVar8 == 0) {
            if (DAT_013b722c == 0) {
              (**(code **)(*local_1d0 + 0x60))();
              if (DAT_005b892c == 2) {
                (**(code **)(*(int *)extraout_ECX[0x9eca] + 0x60))();
              }
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            }
            ExceptionList = local_10;
            return 1;
          }
          local_1d4 = extraout_ECX[0x9d33];
          extraout_ECX[0x9d33] = extraout_ECX[0x9d32];
          extraout_ECX[0x9d32] = extraout_ECX[0x9d31];
          extraout_ECX[0x9d31] = extraout_ECX[0x9d30];
          extraout_ECX[0x9d30] = uVar4;
          if (uVar4 - local_1d4 < 4000) {
            FUN_00403df2();
            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
            (**(code **)(*local_1d0 + 0x90))();
            (**(code **)(*local_1d0 + 0x60))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            ExceptionList = local_10;
            return 1;
          }
          if ((char)extraout_ECX[0x9bbc] == '\0') {
            for (local_2e8 = 0; local_2e8 < 5; local_2e8 = local_2e8 + 1) {
              (**(code **)(*local_1d0 + 0x88))();
              FUN_0058f078();
            }
          }
          else {
            pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
            iVar14 = _strcmp((char *)(extraout_ECX + 0x9bbc),pcVar5);
            if (iVar14 != 0) {
              for (local_2ec = 4; 0 < local_2ec; local_2ec = local_2ec + -1) {
                FUN_0058f220();
              }
              (**(code **)(*local_1d0 + 0x88))();
              FUN_0058f078();
            }
          }
          *(undefined2 *)((int)extraout_ECX + 0x26eee) = 0;
          pcVar12 = &DAT_00a3a0f8;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,pcVar12);
          if (iVar14 == 0) {
            FUN_0044fc4b();
            (**(code **)(*local_1d0 + 0x90))();
            (**(code **)(*local_1d0 + 0x60))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            ExceptionList = local_10;
            return 1;
          }
          pcVar12 = s__help_005b7e5c;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,pcVar12);
          if (iVar14 == 0) {
            FUN_004524ab();
            (**(code **)(*local_1d0 + 0x90))();
            ExceptionList = local_10;
            return 1;
          }
          pcVar12 = s_effects_005b7e64;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,pcVar12);
          if (iVar14 == 0) {
            DAT_0092e664 = (uint)(DAT_0092e660 != 0);
            DAT_0092e660 = (uint)(DAT_0092e660 == 0);
            (**(code **)(*local_1d0 + 0x90))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            ExceptionList = local_10;
            return 1;
          }
          pcVar12 = s_kpoint_005b7e6c;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,pcVar12);
          if (iVar14 == 0) {
            local_370 = DAT_0067cf38;
            FUN_0058f078();
            FUN_00493aa1();
            ExceptionList = local_10;
            return 1;
          }
          ppuVar13 = &PTR_DAT_005b7e88;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,(char *)ppuVar13);
          if (iVar14 == 0) {
            *(uint *)(DAT_005ccf9c + 0x2b050) = (uint)(*(int *)(DAT_005ccf9c + 0x2b050) == 0);
            (**(code **)(*local_1d0 + 0x90))();
            (**(code **)(*local_1d0 + 0x60))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            FUN_004431e4();
            ExceptionList = local_10;
            return 1;
          }
          pcVar12 = s_effect_005b7e8c;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,pcVar12);
          if (iVar14 == 0) {
            *(uint *)(DAT_005ccf9c + 0x2b03c) = (uint)(*(int *)(DAT_005ccf9c + 0x2b03c) == 0);
            (**(code **)(*local_1d0 + 0x90))();
            (**(code **)(*local_1d0 + 0x60))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            ExceptionList = local_10;
            return 1;
          }
          ppuVar13 = &PTR_DAT_005b7e94;
          pcVar5 = (char *)(**(code **)(*local_1d0 + 0x88))();
          iVar14 = _strcmp(pcVar5,(char *)ppuVar13);
          if (iVar14 == 0) {
            extraout_ECX[0xa3b2] = (uint)(extraout_ECX[0xa3b2] == 0);
            if (extraout_ECX[0xa3b2] == 0) {
              FUN_0058f078();
            }
            else {
              FUN_0058f078();
            }
            FUN_00493aa1();
            ExceptionList = local_10;
            return 1;
          }
          local_1d8 = (char *)0xffffaaaa;
          local_1dc = 0;
          (**(code **)(*local_1d0 + 0x88))();
          FUN_0058f078();
          local_2e4 = extraout_ECX[0xa1cb];
          local_2e0 = extraout_ECX[0x9eba];
          if (local_2dc == '-') {
            local_1d8 = (char *)0xffaaffff;
            local_1dc = 1;
            if (local_2db == '-') {
              local_1d8 = (char *)0xff00ffff;
              local_1dc = 2;
            }
            _memset(local_474,0,0x80);
            local_46e = (undefined2)DAT_013b71e8[0x317];
            local_470 = 0x334;
            local_3f8 = 3;
            FUN_0058f078();
            (**(code **)(*local_1d0 + 0x88))();
            FUN_0058f078();
            FUN_004b17e2();
            (**(code **)(*local_1d0 + 0x90))();
            FUN_0055f2dd();
            FUN_0058f078();
            pvVar7 = operator_new(0xe50);
            local_8 = 1;
            if (pvVar7 == (void *)0x0) {
              local_1ae0 = 0;
            }
            else {
              puStackY_34 = (undefined1 *)0x43960000;
              piStackY_38 = (int *)0x0;
              puStackY_3c = (undefined1 *)0x0;
              pcStackY_40 = local_1d8;
              puStackY_44 = local_25c;
              puStackY_48 = (undefined1 *)0x467a8b;
              local_1ae0 = FUN_00407203();
            }
            local_8 = 0xffffffff;
            local_3f4 = local_1ae0;
            if (local_1ae0 != 0) {
              FUN_00408d33();
            }
          }
          else if ((local_2dc == '=') && (1 < *(int *)(local_2e4 + 0x210))) {
            local_1d8 = (char *)0xffff99ff;
            local_1dc = 2;
            _memset(local_4f8,0,0x80);
            local_4f2 = (undefined2)DAT_013b71e8[0x317];
            local_4f4 = 0x334;
            local_47c = 1;
            FUN_0058f078();
            (**(code **)(*local_1d0 + 0x88))();
            FUN_0058f078();
            FUN_004b17e2();
            (**(code **)(*local_1d0 + 0x90))();
            FUN_0055f2dd();
            FUN_0058f078();
            pvVar7 = operator_new(0xe50);
            local_8 = 2;
            if (pvVar7 == (void *)0x0) {
              local_1ae4 = 0;
            }
            else {
              puStackY_34 = (undefined1 *)0x43960000;
              piStackY_38 = (int *)0x0;
              puStackY_3c = (undefined1 *)0x0;
              pcStackY_40 = local_1d8;
              puStackY_44 = local_25c;
              puStackY_48 = (undefined1 *)0x467c43;
              local_1ae4 = FUN_00407203();
            }
            local_8 = 0xffffffff;
            local_478 = local_1ae4;
            if (local_1ae4 != 0) {
              FUN_00408d33();
            }
          }
          else if (local_2dc == '/') {
            local_1dc = 3;
            _memset(local_57c,0,0x80);
            FUN_005909d2();
            iVar14 = _strcmp(local_57c,&DAT_005b7ee0);
            if ((((iVar14 == 0) || (iVar14 = _strcmp(local_57c,&DAT_00a3b678), iVar14 == 0)) ||
                (iVar14 = _strcmp(local_57c,s_Relocate_005b7ee8), iVar14 == 0)) ||
               (iVar14 = _strcmp(local_57c,s_relocate_005b7ef4), iVar14 == 0)) {
              iVar14 = FUN_0040c0f0();
              if (iVar14 != 0) {
                ExceptionList = local_10;
                return 1;
              }
              FUN_005909d2();
              *(undefined1 *)((int)extraout_ECX + 0x271d2) = 2;
              iVar14 = (**(code **)(*DAT_0092e654 + 8))();
              extraout_ECX[0x9c53] = iVar14;
              (**(code **)(*local_1d0 + 0x90))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              if (DAT_005b892c == 2) {
                (**(code **)(*(int *)extraout_ECX[0x9eca] + 0x60))();
              }
              ExceptionList = local_10;
              return 1;
            }
            local_1d8 = (char *)0xffffff00;
            _memset(&local_600,0,0x80);
            local_5fa = (undefined2)DAT_013b71e8[0x317];
            local_5fc = 0x334;
            FUN_0058f078();
            sVar8 = _strlen(local_57c);
            if (0xf < sVar8) {
              local_56d = 0;
              local_56e = 0;
            }
            if ((char)extraout_ECX[0x9c34] == '\0') {
              for (local_604 = 0; local_604 < 5; local_604 = local_604 + 1) {
                FUN_0058f078();
              }
            }
            else {
              iVar14 = _strcmp((char *)(extraout_ECX + 0x9c34),local_57c);
              if (iVar14 != 0) {
                for (local_608 = 4; 0 < local_608; local_608 = local_608 + -1) {
                  FUN_0058f220();
                }
                FUN_0058f078();
              }
            }
            *(undefined2 *)(extraout_ECX + 0x9bbb) = 0;
            iVar14 = (**(code **)(*local_1d0 + 0x88))();
            sVar8 = _strlen(local_57c);
            local_4fc = (char *)(iVar14 + 1 + sVar8);
            if (*local_4fc != '\0') {
              (**(code **)(*local_1d0 + 0x88))();
              _strlen(local_57c);
              FUN_0058f078();
            }
            FUN_004b17e2();
            (**(code **)(*local_1d0 + 0x90))();
            iVar14 = _strcmp(local_57c,&DAT_005b7f18);
            if ((iVar14 == 0) || (iVar14 = _strcmp(local_57c,s_summonguild_005b7f24), iVar14 == 0))
            {
              if ((*(float *)(extraout_ECX[0x13] + 0x230) < _DAT_005a40f4) &&
                 (_DAT_005a3534 < *(float *)(extraout_ECX[0x13] + 0x230))) {
                ExceptionList = local_10;
                return 1;
              }
              FUN_0055f2dd();
            }
            else {
              iVar14 = _strcmp(local_57c,&DAT_005b7f30);
              if (((iVar14 == 0) || (iVar14 = _strcmp(local_57c,&DAT_005b7f38), iVar14 == 0)) ||
                 ((iVar14 = _strcmp(local_57c,&DAT_005b7f40), iVar14 == 0 ||
                  (((iVar14 = _strcmp(local_57c,s_kingdom_005b7f48), iVar14 == 0 ||
                    (iVar14 = _strcmp(local_57c,&DAT_005b7f50), iVar14 == 0)) ||
                   (iVar14 = _strcmp(local_57c,s_Kingdom_005b7f58), iVar14 == 0)))))) {
                if ((*(float *)(extraout_ECX[0x13] + 0x230) < _DAT_005a40f4) &&
                   (_DAT_005a3534 < *(float *)(extraout_ECX[0x13] + 0x230))) {
                  ExceptionList = local_10;
                  return 1;
                }
                piVar10 = &local_600;
                piVar11 = extraout_ECX + 0x9c75;
                for (iVar14 = 0x20; iVar14 != 0; iVar14 = iVar14 + -1) {
                  *piVar11 = *piVar10;
                  piVar10 = piVar10 + 1;
                  piVar11 = piVar11 + 1;
                }
                *(undefined1 *)((int)extraout_ECX + 0x271d3) = 1;
                iVar14 = (**(code **)(*DAT_0092e654 + 8))();
                extraout_ECX[0x9c54] = iVar14;
              }
              else {
                iVar14 = _strcmp(local_57c,&DAT_00a403f8);
                if (iVar14 == 0) {
                  FUN_0058f078();
                  FUN_0055f2dd();
                }
                else {
                  iVar14 = _strcmp(local_57c,&DAT_00a40278);
                  if (iVar14 == 0) {
                    if (*(short *)(extraout_ECX[0x13] + 0x682) != 0) {
                      FUN_00403df2();
                      (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                      ExceptionList = local_10;
                      return 1;
                    }
                    FUN_0058f078();
                    FUN_0055f2dd();
                  }
                  else {
                    iVar14 = _strcmp(local_57c,&DAT_00a402f8);
                    if (iVar14 == 0) {
                      if ((*(short *)(extraout_ECX[0x13] + 0x682) != 0x1fd) &&
                         ((*(short *)(extraout_ECX[0x13] + 0x682) < 0x20e ||
                          (0x213 < *(short *)(extraout_ECX[0x13] + 0x682))))) {
                        FUN_00403df2();
                        (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                        ExceptionList = local_10;
                        return 1;
                      }
                      FUN_0058f078();
                      FUN_0055f2dd();
                    }
                    else {
                      iVar14 = _strcmp(local_57c,&DAT_00a404f8);
                      if (iVar14 == 0) {
                        FUN_0058f078();
                        FUN_0055f2dd();
                      }
                      else {
                        iVar14 = _strcmp(local_57c,&DAT_00a40478);
                        if (iVar14 == 0) {
                          FUN_0058f078();
                          FUN_0055f2dd();
                        }
                        else {
                          iVar14 = _strcmp(local_57c,&DAT_005b7f98);
                          if (iVar14 == 0) {
                            FUN_0058f078();
                            FUN_0055f2dd();
                          }
                          else {
                            iVar14 = _strcmp(local_57c,(char *)&PTR_DAT_005b7fac);
                            if (iVar14 == 0) {
                              ExceptionList = local_10;
                              return 1;
                            }
                            iVar14 = _strcmp(local_57c,(char *)&PTR_DAT_005b7fb0);
                            if (iVar14 == 0) {
                              if ((*(float *)(extraout_ECX[0x13] + 0x230) < _DAT_005a40f4) &&
                                 (_DAT_005a3534 < *(float *)(extraout_ECX[0x13] + 0x230))) {
                                ExceptionList = local_10;
                                return 1;
                              }
                              iVar14 = _strcmp((char *)(extraout_ECX[0x13] + 0x178),local_5e4);
                              if (iVar14 == 0) {
                                ExceptionList = local_10;
                                return 1;
                              }
                              FUN_0058f078();
                              FUN_0058f078();
                            }
                            FUN_0055f2dd();
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            iVar14 = _strcmp(local_57c,&DAT_005b7fbc);
            if ((((iVar14 == 0) || (iVar14 = _strcmp(local_57c,&DAT_005b7fc0), iVar14 == 0)) ||
                (iVar14 = _strcmp(local_57c,&DAT_005b7fc4), iVar14 == 0)) ||
               (iVar14 = _strcmp(local_57c,&DAT_005b7fc8), iVar14 == 0)) {
              FUN_0058f078();
              puStackY_34 = (undefined1 *)0x46863f;
              FUN_0058f078();
            }
            else {
              puStackY_34 = (undefined1 *)0x468670;
              FUN_0058f078();
            }
            pvVar7 = operator_new(0xe50);
            local_8 = 3;
            if (pvVar7 == (void *)0x0) {
              local_1ae8 = 0;
            }
            else {
              puStackY_34 = (undefined1 *)0x43960000;
              piStackY_38 = (int *)0x0;
              puStackY_3c = (undefined1 *)0x0;
              pcStackY_40 = local_1d8;
              puStackY_44 = local_25c;
              puStackY_48 = (undefined1 *)0x4686c8;
              local_1ae8 = FUN_00407203();
            }
            local_8 = 0xffffffff;
            local_580 = local_1ae8;
            if (local_1ae8 != 0) {
              FUN_00408d33();
            }
          }
          else {
            local_1dc = 0;
            _memset(local_678,0,0x6c);
            local_672 = (undefined2)DAT_013b71e8[0x317];
            local_674 = 0x333;
            (**(code **)(*local_1d0 + 0x88))();
            FUN_0058f078();
            FUN_004b17e2();
            (**(code **)(*local_1d0 + 0x90))();
            FUN_0055f2dd();
            FUN_0058f078();
            pvVar7 = operator_new(0xe50);
            local_8 = 4;
            if (pvVar7 == (void *)0x0) {
              local_1aec = 0;
            }
            else {
              puStackY_34 = (undefined1 *)0x43960000;
              piStackY_38 = (int *)0x0;
              puStackY_3c = (undefined1 *)0x0;
              pcStackY_40 = local_1d8;
              puStackY_44 = local_25c;
              puStackY_48 = (undefined1 *)0x468831;
              local_1aec = FUN_00407203();
            }
            local_8 = 0xffffffff;
            local_60c = local_1aec;
            if ((local_1aec != 0) && (local_2e0 != 0)) {
              FUN_00408d33();
            }
            FUN_0058f078();
            if (local_66c == '*') {
              *(undefined4 *)(extraout_ECX[0x13] + 0x61c) = 10000;
              FUN_0058f078();
            }
            else {
              *(undefined4 *)(extraout_ECX[0x13] + 0x61c) = 3000;
            }
          }
          iVar14 = (**(code **)(*DAT_0092e654 + 8))();
          extraout_ECX[0x9d19] = iVar14;
          iVar14 = _strncmp(&local_2dc,s__set_005b8004,4);
          if ((iVar14 != 0) && (local_1dc == 0)) {
            FUN_00504748();
          }
        }
        else if ((param_1 == 0x1403) && ((param_2 == 2 || (param_2 == 3)))) {
          FUN_0058f078();
          (**(code **)(*(int *)extraout_ECX[0x9ecb] + 0x90))();
          *(short *)((int)extraout_ECX + 0x26eee) = *(short *)((int)extraout_ECX + 0x26eee) + 1;
          *(short *)((int)extraout_ECX + 0x26eee) = *(short *)((int)extraout_ECX + 0x26eee) % 5;
        }
        else if ((param_1 == 0x1403) && ((param_2 == 4 || (param_2 == 5)))) {
          FUN_0058f078();
          (**(code **)(*(int *)extraout_ECX[0x9ecb] + 0x90))();
          *(short *)(extraout_ECX + 0x9bbb) = (short)extraout_ECX[0x9bbb] + 1;
          *(short *)(extraout_ECX + 0x9bbb) = (short)extraout_ECX[0x9bbb] % 5;
        }
        else if ((param_1 == 0x1403) && (param_2 == 6)) {
          _memset(local_718,0,0x20);
          (**(code **)(*(int *)extraout_ECX[0x9ecb] + 0x90))();
        }
        else if (param_1 == 0x171) {
          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
            ExceptionList = local_10;
            return 1;
          }
          local_728 = (int *)extraout_ECX[0x9f1e];
          local_72c = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
          local_71c = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
          local_720 = extraout_ECX[0xa176];
          local_724 = extraout_ECX[0xa17c];
          iVar14 = FUN_0040c0f0();
          if (iVar14 == 1) {
            extraout_ECX[0x9bb8] = 0;
            (**(code **)(*local_72c + 0x80))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            puStackY_34 = (undefined1 *)0x468c1a;
            (**(code **)(*local_728 + 0x60))();
          }
          else {
            iVar14 = FUN_0040c0f0();
            if (iVar14 == 1) {
              if (0 < DAT_013b71e8[0x33d]) {
                ExceptionList = local_10;
                return 1;
              }
              extraout_ECX[0x9bb8] = 1;
              (**(code **)(*local_72c + 0x80))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              puStackY_34 = (undefined1 *)0x468ca3;
              (**(code **)(*local_728 + 0x60))();
            }
            else {
              (**(code **)(*local_728 + 0x60))();
            }
          }
        }
        else if (param_1 == 0x72a) {
          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
            ExceptionList = local_10;
            return 1;
          }
          local_734 = extraout_ECX[0x9ece];
          if ((local_734 != 0) && (iVar14 = FUN_0040c0f0(), iVar14 == 1)) {
            ExceptionList = local_10;
            return 1;
          }
          local_740 = (int *)extraout_ECX[0x9f1e];
          local_744 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
          local_730 = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
          local_738 = extraout_ECX[0xa176];
          local_73c = extraout_ECX[0xa17c];
          iVar14 = FUN_0040c0f0();
          if (iVar14 == 1) {
            extraout_ECX[0x9bb8] = 2;
            (**(code **)(*local_744 + 0x80))();
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            (**(code **)(*local_740 + 0x60))();
          }
        }
        else {
          if (param_1 == 0x274) {
            if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
              ExceptionList = local_10;
              return 1;
            }
            local_748 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            local_750 = (char *)(**(code **)(*local_748 + 0x88))();
            local_764 = _strlen(local_750);
            iVar14 = _strcmp(local_750,(char *)&PTR_DAT_005b8014);
            if (iVar14 == 0) {
              if (((extraout_ECX[0x9bb8] == 0) || (extraout_ECX[0x9bb8] == 1)) ||
                 (extraout_ECX[0x9bb8] == 7)) {
                FUN_0058f078();
                (**(code **)(*local_748 + 0x90))();
              }
              else if (extraout_ECX[0x9bb8] == 2) {
                FUN_0058f078();
                (**(code **)(*local_748 + 0x90))();
              }
              ExceptionList = local_10;
              return 1;
            }
            local_74c = 0;
            for (local_75c = 0; local_75c < (int)local_764; local_75c = local_75c + 1) {
              if ((local_750[local_75c] < '0') || ('9' < local_750[local_75c])) {
                local_74c = 1;
                break;
              }
            }
            for (local_75c = 0; local_75c < (int)local_764; local_75c = local_75c + 1) {
              if (local_750[local_75c] == '%') {
                local_750[local_75c] = '!';
              }
            }
            (**(code **)(*local_748 + 0x88))();
            local_758 = FUN_00591165();
            pcVar5 = (char *)(**(code **)(*local_748 + 0x88))();
            local_768 = _strlen(pcVar5);
            if ((int)local_768 < 1) {
              local_7ec = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              ExceptionList = local_10;
              return 1;
            }
            if ((((extraout_ECX[0x9bb8] != 8) && (extraout_ECX[0x9bb8] != 3)) &&
                (extraout_ECX[0x9bb8] != 6)) &&
               (((local_74c == 1 || (local_758 < 0)) ||
                ((DAT_013b71e8[0x1c1] < local_758 && (extraout_ECX[0x9bb8] == 0)))))) {
              FUN_00403df2();
              (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
              local_7f0 = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              ExceptionList = local_10;
              return 1;
            }
            if ((extraout_ECX[0x9bb8] == 4) && (999999999 < local_758)) {
              FUN_0058f078();
              FUN_00403df2();
              (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
              local_7f4 = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              ExceptionList = local_10;
              return 1;
            }
            if ((extraout_ECX[0x9bb8] == 5) && (1999999999 < local_758)) {
              FUN_0058f078();
              FUN_00403df2();
              (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
              local_8f8 = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              ExceptionList = local_10;
              return 1;
            }
            if (extraout_ECX[0x9bb8] == 0) {
              _memset(local_a08,0,0x10);
              local_a04 = 0x388;
              local_a02 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
              local_9fc = (int)local_758;
              FUN_0055f2dd();
            }
            else if (extraout_ECX[0x9bb8] == 2) {
              _memset(local_a18,0,0x10);
              local_a14 = 0x387;
              local_a12 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
              local_a0c = (int)local_758;
              FUN_0055f2dd();
            }
            else if (extraout_ECX[0x9bb8] == 1) {
              local_a9c = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              local_aa4 = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              *(undefined4 *)(local_a9c + 0x1e8) = 0;
              *(undefined4 *)(local_aa4 + 0x1e8) = 0;
              iVar14 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
              extraout_ECX[0x9d34] = iVar14;
              *(undefined1 *)(DAT_013b71e8 + 0x33e) = *(undefined1 *)(local_a9c + 0x1e8);
              DAT_013b71e8[0x33d] = (int)local_758;
              *(undefined2 *)(DAT_013b71e8 + 0x319) = 899;
              FUN_0055f2dd();
              local_aa0 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              puStackY_34 = &DAT_005b8020;
              piStackY_38 = local_a98;
              puStackY_3c = (undefined1 *)0x469588;
              FUN_0058f078();
              puStackY_34 = (undefined1 *)0x4695a8;
              (**(code **)(*local_aa0 + 0x80))();
            }
            else if (extraout_ECX[0x9bb8] == 3) {
              local_ab4 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              local_aac = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              if (local_ab4 != (int *)0x0) {
                (**(code **)(*local_748 + 0x88))();
                (**(code **)(*local_ab4 + 0x80))();
              }
              (**(code **)(*local_aac + 0x80))();
              *(undefined2 *)((int)extraout_ECX + 0x275ae) =
                   *(undefined2 *)(extraout_ECX[0x13] + 0x20);
              (**(code **)(*local_748 + 0x88))();
              puStackY_34 = &DAT_005b8028;
              piStackY_38 = extraout_ECX + 0x9d3e;
              puStackY_3c = (undefined1 *)0x4696a4;
              FUN_0058f078();
              local_ab8 = (int *)extraout_ECX[0x9ecc];
              if (local_ab8 != (int *)0x0) {
                puStackY_34 = (undefined1 *)0x4696d5;
                (**(code **)(*local_ab8 + 0x60))();
              }
              if (DAT_005b892c != 2) {
                puStackY_34 = (undefined1 *)0x42c40000;
                piStackY_38 = (int *)0x4696fb;
                (**(code **)(*local_748 + 0x70))();
              }
              local_748[0x414] = 10;
              puStackY_34 = (undefined1 *)0x469727;
              local_aa8 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
              for (local_ab0 = 0; local_ab0 < 0xc; local_ab0 = local_ab0 + 1) {
                FUN_0058f078();
                local_ac8 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
                *(undefined1 *)((int)local_ac8 + 0xcdd) = 1;
                (**(code **)(*local_ac8 + 0x80))();
                local_ac0 = (int *)extraout_ECX[local_ab0 + 0x9ecf];
                puStackY_34 = (undefined1 *)0x4697f0;
                local_abc = (undefined4 *)(**(code **)(*local_ac0 + 0xa8))();
                puStackY_34 = (undefined1 *)
                              ((int)*(char *)((int)extraout_ECX + local_ab0 + 0x27570) / 9);
                piStackY_38 = (int *)((int)*(char *)((int)extraout_ECX + local_ab0 + 0x27570) % 9);
                puStackY_3c = (undefined1 *)0x469842;
                local_ac4 = (**(code **)(*local_aa8 + 0xb8))();
                if (local_ac4 != 0) {
                  *(undefined4 *)(local_ac4 + 0x94) = 0xffffffff;
                }
                if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_abc)) {
                  DAT_005ccec0[0x7a] = 0;
                }
                if (local_abc != (undefined4 *)0x0) {
                  if (local_abc != (undefined4 *)0x0) {
                    (**(code **)*local_abc)();
                  }
                  local_abc = (undefined4 *)0x0;
                }
              }
              FUN_0044ae38();
            }
            else if (extraout_ECX[0x9bb8] == 4) {
              if (-1 < extraout_ECX[0x9bb9]) {
                for (local_b4c = 0; local_b4c < 0xc; local_b4c = local_b4c + 1) {
                  local_b58 = (int *)extraout_ECX[local_b4c + 0x9ecf];
                  local_b54 = (**(code **)(*local_b58 + 0xb8))();
                  local_b50 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
                  puStackY_34 = (undefined1 *)0x4699e9;
                  local_b5c = (**(code **)(*local_b50 + 0xb8))();
                  if (local_b54 == 0) {
                    if (local_b5c == 0) {
                      ExceptionList = local_10;
                      return 1;
                    }
                    local_be4 = operator_new(8);
                    FUN_0058f220();
                    pvVar7 = operator_new(0x678);
                    local_8 = 5;
                    if (pvVar7 == (void *)0x0) {
                      local_1afc = 0;
                    }
                    else {
                      local_1afc = FUN_0040d13e();
                    }
                    local_8 = 0xffffffff;
                    local_be8 = local_1afc;
                    (**(code **)(*local_b58 + 0x8c))();
                    *(undefined4 *)(local_b5c + 0x94) = 0xffff0000;
                    local_b60 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
                    *(undefined1 *)((int)local_b60 + 0xcdd) = 1;
                    puStackY_34 = (undefined1 *)(int)local_758;
                    piStackY_38 = (int *)&DAT_005b8034;
                    puStackY_3c = local_be0;
                    pcStackY_40 = (char *)0x469b3f;
                    FUN_0058f078();
                    puStackY_34 = local_be0;
                    piStackY_38 = (int *)0x469b5f;
                    (**(code **)(*local_b60 + 0x80))();
                    *(char *)((int)extraout_ECX + local_b4c + 0x27570) = (char)extraout_ECX[0x9bb9];
                    extraout_ECX[local_b4c + 0x9d5f] = (int)local_758;
                    FUN_0058f220();
                    local_b58[0x163] = (int)local_758;
                    break;
                  }
                  if ((local_b54 != 0) && (local_b4c == 0xb)) {
                    FUN_00403df2();
                    (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                    *(undefined4 *)(local_b5c + 0x94) = 0xffffffff;
                  }
                }
                extraout_ECX[0x9bb9] = -1;
              }
            }
            else if (extraout_ECX[0x9bb8] == 5) {
              _memset(local_bfc,0,0x14);
              local_bf6 = (undefined2)DAT_013b71e8[0x317];
              local_bf8 = 0x28f;
              local_bf0 = extraout_ECX[0x9bb7];
              local_bec = (int)local_758;
              FUN_0055f2dd();
            }
            else if (extraout_ECX[0x9bb8] == 6) {
              FUN_00465b05();
            }
            else if (extraout_ECX[0x9bb8] == 7) {
              _memset(local_c0c,0,0x10);
              local_c08 = 0x3d1;
              local_c06 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
              local_c00 = (int)local_758;
              FUN_0055f2dd();
            }
            else if (extraout_ECX[0x9bb8] == 8) {
              local_c94 = 3;
              _memset(local_c90,0,0x80);
              FUN_0058f078();
              _memset(local_d14,0,0x80);
              local_d0e = (undefined2)DAT_013b71e8[0x317];
              local_d10 = 0x334;
              FUN_0058f078();
              FUN_0058f078();
              sVar8 = _strlen(local_c90);
              if (0xf < sVar8) {
                local_c81 = 0;
                local_c82 = 0;
              }
              if ((char)extraout_ECX[0x9c34] == '\0') {
                for (local_d18 = 0; local_d18 < 5; local_d18 = local_d18 + 1) {
                  FUN_0058f078();
                }
              }
              else {
                iVar14 = _strcmp((char *)(extraout_ECX + 0x9c34),local_c90);
                if (iVar14 != 0) {
                  for (local_d1c = 4; 0 < local_d1c; local_d1c = local_d1c + -1) {
                    FUN_0058f220();
                  }
                  FUN_0058f078();
                }
              }
              *(undefined2 *)(extraout_ECX + 0x9bbb) = 0;
              iVar14 = (**(code **)(*local_748 + 0x88))();
              sVar8 = _strlen(local_c90);
              local_c10 = (char *)(iVar14 + 1 + sVar8);
              if (*local_c10 != '\0') {
                (**(code **)(*local_748 + 0x88))();
                _strlen(local_c90);
                FUN_0058f078();
              }
              FUN_004b17e2();
              (**(code **)(*local_748 + 0x90))();
              FUN_0055f2dd();
            }
            else if (extraout_ECX[0x9bb8] == 9) {
              _memset(local_d2c,0,0x10);
              local_d28 = 0xed7;
              local_d26 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
              local_d20 = (int)local_758;
              FUN_0055f2dd();
            }
            else if (extraout_ECX[0x9bb8] == 10) {
              _memset(local_d3c,0,0x10);
              local_d38 = 0xed8;
              local_d36 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
              local_d30 = (int)local_758;
              FUN_0055f2dd();
            }
            local_760 = (int *)extraout_ECX[0x9f1e];
            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            (**(code **)(*local_760 + 0x60))();
            (**(code **)(*local_748 + 0x90))();
            if (DAT_013b722c == 1) {
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            }
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x273) {
            if ((*(char *)(extraout_ECX[0x13] + 0x245) != '\x01') && (extraout_ECX[0x9bb8] != 5)) {
              (**(code **)(*extraout_ECX + 0x58))();
              ExceptionList = local_10;
              return 1;
            }
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x275) {
            FUN_00447594();
          }
          else if (param_1 == 0x170) {
            FUN_00447691();
            local_d40 = extraout_ECX[0x9ecd];
            local_d44 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            (**(code **)(*local_d44 + 0x8c))();
            if (DAT_013b722c == 1) {
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            }
          }
          else if (param_1 == 0x215) {
            FUN_00447c73();
            local_d4c = extraout_ECX[0x9f1f];
            local_d48 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            (**(code **)(*local_d48 + 0x8c))();
            if (DAT_013b722c == 1) {
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            }
          }
          else if (param_1 == 0x779) {
            FUN_0044c53f();
            if (DAT_013b722c == 1) {
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
            }
          }
          else if (param_1 == 0x125) {
            iVar14 = FUN_0040c0f0();
            if (iVar14 == 1) {
              ExceptionList = local_10;
              return 1;
            }
            iVar14 = FUN_0040c0f0();
            if (iVar14 == 1) {
              ExceptionList = local_10;
              return 1;
            }
            local_d50 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            (**(code **)(*local_d50 + 0x8c))();
            FUN_00447c73();
          }
          else if (param_1 == 0x126) {
            local_d54 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            (**(code **)(*local_d54 + 0x8c))();
            FUN_00447691();
          }
          else if (param_1 == 0x127) {
            local_d58 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
            (**(code **)(*local_d58 + 0x8c))();
            FUN_0044c53f();
          }
          else if (param_1 == 0x128) {
            FUN_0044ca65();
          }
          else if (param_1 == 0x129) {
            FUN_0044da6f();
          }
          else {
            if (param_1 == 0x130) {
              local_d5c = (int *)extraout_ECX[0x9f1c];
              (**(code **)(*local_d5c + 0x60))();
              (**(code **)(*DAT_005ccec0 + 0x98))();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x134) {
              FUN_0044e937();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x132) {
              FUN_0044ecae();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x133) {
              FUN_0044ed63();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x138) {
              FUN_0044eecf();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x415) {
              FUN_0044effc();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x416) {
              FUN_0044f46b();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x417) {
              FUN_0044f88f();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x29f) {
              (**(code **)(*(int *)extraout_ECX[0x9f22] + 0x60))();
              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x139) {
              iVar14 = (**(code **)(*(int *)extraout_ECX[0x13] + 0x50))();
              if (iVar14 != 0) {
                local_d60 = extraout_ECX[0x9ece];
                if ((local_d60 == 0) || (iVar14 = FUN_0040c0f0(), iVar14 != 1)) {
                  FUN_004656af();
                }
                else {
                  FUN_0044ae38();
                }
                ExceptionList = local_10;
                return 1;
              }
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x137) {
              _memset(local_dcc,0,0x6c);
              local_dc6 = (undefined2)DAT_013b71e8[0x317];
              local_dc8 = 0x333;
              FUN_0058f078();
              FUN_0055f2dd();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x136) {
              _memset(local_e38,0,0x6c);
              local_e32 = (undefined2)DAT_013b71e8[0x317];
              local_e34 = 0x333;
              FUN_0058f078();
              FUN_0055f2dd();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x135) {
              _memset(local_ea4,0,0x6c);
              local_e9e = (undefined2)DAT_013b71e8[0x317];
              local_ea0 = 0x333;
              FUN_0058f078();
              FUN_0055f2dd();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 299) {
              _memset(local_f10,0,0x6c);
              local_f0a = (undefined2)DAT_013b71e8[0x317];
              local_f0c = 0x333;
              if (extraout_ECX[0xa1fd] == 0) {
                *(char *)(extraout_ECX + 0x9bb0) = '\x01' - ((char)extraout_ECX[0x9bb0] != '\0');
                if ((char)extraout_ECX[0x9bb0] == '\x01') {
                  FUN_0058f078();
                }
                else {
                  FUN_0058f078();
                }
              }
              else if (*(int *)(extraout_ECX[0xa1fd] + 0x1e8) == 1) {
                FUN_0058f078();
              }
              else {
                FUN_0058f078();
              }
              FUN_0055f2dd();
              ExceptionList = local_10;
              return 1;
            }
            if (param_1 == 0x12a) {
              FUN_00450962();
            }
            else {
              if (param_1 == 0x27b) {
                if (DAT_013b7220 == 0) {
                  DAT_013b7220 = (**(code **)(*DAT_0092e654 + 8))();
                  local_f1a = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                  local_f1c = 0x3ae;
                  local_f14 = 0;
                  FUN_0042550e();
                  ExceptionList = local_10;
                  return 0;
                }
                ExceptionList = local_10;
                return 1;
              }
              if (param_1 == 0x1670) {
                if (*(int *)(extraout_ECX[0xa1ff] + 0x28) == 0) {
                  (**(code **)(*(int *)extraout_ECX[0xa1ff] + 0x60))();
                }
                else {
                  (**(code **)(*(int *)extraout_ECX[0xa1ff] + 0x60))();
                }
                if (extraout_ECX[0x9ec6] != 0) {
                  *(undefined4 *)(extraout_ECX[0x9ec6] + 0x1e8) =
                       *(undefined4 *)(extraout_ECX[0xa1ff] + 0x28);
                }
              }
              else if (param_1 == 0x279) {
                if (extraout_ECX[0xa2e4] == 0) {
                  iVar14 = (**(code **)(*DAT_0092e654 + 8))();
                  extraout_ECX[0x9c4f] = iVar14;
                  local_13a6 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                  local_13a8 = 0x3ae;
                  local_13a0 = 0;
                  FUN_0055f2dd();
                  ExceptionList = local_10;
                  return 1;
                }
                (**(code **)(*(int *)extraout_ECX[0x9f1c] + 0x60))();
                for (local_f94 = 0; local_f94 < 0xb; local_f94 = local_f94 + 1) {
                  *(undefined4 *)(local_f48 + local_f94 * 4 + -4) = 0xffffffff;
                }
                FUN_00403df2();
                (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                local_f98 = 0;
                for (local_f8c = 0; local_f8c < 0xb; local_f8c = local_f8c + 1) {
                  if ((&DAT_009325f8)[local_f8c * 0x2c0] == '\0') {
                    local_f98 = local_f8c + -1;
                    break;
                  }
                }
                puStackY_34 = (undefined1 *)0x46ab7f;
                _memset(local_f88,0,0x28);
                puStackY_34 = (undefined1 *)0x46ab92;
                _memset(&local_f5c,0,0x10);
                GetLocalTime(&local_f5c);
                local_f60 = (local_f5c._6_4_ & 0xffff) % 10;
                if (local_f60 == 0) {
                  local_f60 = 10;
                }
                for (local_f8c = 0; local_f8c < 10; local_f8c = local_f8c + 1) {
                  for (local_13b0 = 1; local_13b0 < 0xb; local_13b0 = local_13b0 + 1) {
                    if ((&DAT_009325f8)[local_13b0 * 0x40 + local_f8c * 0x2c0] != '\0') {
                      local_f88[local_f8c] = local_f88[local_f8c] + 1;
                    }
                  }
                  if (local_f88[local_f8c] != 0) {
                    local_13b4 = (int)((longlong)(ulonglong)(local_f5c._6_4_ & 0xffff) %
                                      (longlong)local_f88[local_f8c]);
                    if (local_13b4 == 0) {
                      local_13b4 = local_f88[local_f8c];
                    }
                    local_f88[local_f8c] = local_13b4;
                  }
                }
                local_f9c = DAT_013b71e8[0x6bd3];
                if (local_f9c == local_f98) {
                  for (local_13b8 = local_f98; local_13b8 < 10; local_13b8 = local_13b8 + 1) {
                    (&DAT_009325f8)[local_13b8 * 0x40 + local_f9c * 0x2c0] = 0;
                  }
                  for (local_13b8 = 0; local_13b8 < local_f98; local_13b8 = local_13b8 + 1) {
                    puStackY_34 = (undefined1 *)0x46ad62;
                    _memset(local_13e4,-1,0x2c);
                    puStackY_34 = (undefined1 *)0x46ad89;
                    FUN_00557a41();
                    puStackY_34 = local_13cc;
                    piStackY_38 = &local_13d0;
                    puStackY_3c = local_13d4;
                    pcStackY_40 = local_13d8;
                    puStackY_44 = local_13dc;
                    puStackY_48 = local_13e0;
                    piStackY_4c = (int *)local_13e4;
                    pcStackY_50 = s__d__d__d__d__d__d__d__d__d__d_005b80a8;
                    puStackY_54 = local_139c;
                    uStackY_58 = 0x46ade3;
                    FUN_005909d2();
                    *(int *)((int)&local_f5c + (local_f98 - local_13b8) * 4 + 0xc) =
                         (&local_13e8)[local_f88[local_13b8]];
                    puStackY_34 = (undefined1 *)0x46ae6a;
                    FUN_0058f078();
                  }
                }
                else {
                  puStackY_34 = (undefined1 *)0x46ae9b;
                  FUN_00557a41();
                  puStackY_34 = local_f34;
                  piStackY_38 = &local_f38;
                  puStackY_3c = local_f3c;
                  pcStackY_40 = local_f40;
                  puStackY_44 = local_f44;
                  puStackY_48 = local_f48;
                  piStackY_4c = &local_f4c;
                  pcStackY_50 = s__d__d__d__d__d__d__d__d__d__d_005b80cc;
                  puStackY_54 = local_139c;
                  uStackY_58 = 0x46aef5;
                  FUN_005909d2();
                }
                (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                local_f90 = (int *)extraout_ECX[0xa2ee];
                if (local_f90 != (int *)0x0) {
                  FUN_004091ab();
                  for (local_13e8 = 1; local_13e8 < 0xb; local_13e8 = local_13e8 + 1) {
                    if ((&DAT_009325f8)[local_13e8 * 0x40 + local_f9c * 0x2c0] == '\0') {
                      if ((local_f9c == local_f98) && (local_13e8 < local_f98)) {
                        FUN_0058f078();
                        local_1534 = *(int *)(&local_f5c.wSecond + local_13e8 * 2);
                        if (local_1534 < 0) {
                          local_1534 = 0;
                        }
                        local_14b0 = (undefined1 *)0x6;
                        pvVar7 = operator_new(0xe68);
                        local_8 = 7;
                        if (pvVar7 == (void *)0x0) {
                          local_1b04 = (undefined1 *)0x0;
                        }
                        else {
                          puStackY_34 = (undefined1 *)0x0;
                          piStackY_38 = (int *)0x0;
                          puStackY_3c = (undefined1 *)0xffffffff;
                          pcStackY_40 = local_1530;
                          puStackY_44 = local_14b0;
                          puStackY_48 = (undefined1 *)0x46b494;
                          local_1b04 = (undefined1 *)FUN_004072f2();
                        }
                        local_8 = 0xffffffff;
                        local_1418[local_13e8] = local_1b04;
                        if (*(int *)(&local_f5c.wSecond + local_13e8 * 2) < 0) {
                          local_1418[local_13e8][0xe64] = 0;
                        }
                        FUN_00408d33();
                      }
                    }
                    else {
                      if (local_f9c == local_f98) {
                        if (local_f98 - local_13e8 < 0) goto LAB_0046af52;
                        if ((&DAT_013b6dc8)
                            [(local_f88[local_f98 - local_13e8] + -1) * 9 +
                             (local_f98 - local_13e8) * 0x5a] == '\0') {
                          FUN_0058f078();
                          if (700 < *(int *)(&local_f5c.wSecond + local_13e8 * 2)) {
                            local_14a0 = _strlen(local_1498);
                            sVar8 = local_14a0;
                            if ((int)local_14a0 < 0xe) {
                              while (local_14a4 = sVar8, (int)local_14a4 < 0xe) {
                                local_1498[local_14a4] = ' ';
                                sVar8 = local_14a4 + 1;
                              }
                            }
                            local_148a = 0;
                            FUN_0058ee30();
                          }
                        }
                        else {
                          FUN_0058f078();
                        }
                      }
                      else if ((&DAT_013b714c)[local_f9c * 9] == '\0') {
                        FUN_0058f078();
                      }
                      else if ((&DAT_013b6dc8)[(local_13e8 + -1) * 9 + local_f9c * 0x5a] == '\0') {
                        FUN_0058f078();
                        if (700 < *(int *)(&local_f5c.wSecond + local_13e8 * 2)) {
                          local_14a8 = _strlen(local_1498);
                          sVar8 = local_14a8;
                          if ((int)local_14a8 < 0xe) {
                            while (local_14ac = sVar8, (int)local_14ac < 0xe) {
                              local_1498[local_14ac] = ' ';
                              sVar8 = local_14ac + 1;
                            }
                          }
                          local_148a = 0;
                          FUN_0058ee30();
                        }
                      }
                      else {
                        FUN_0058f078();
                      }
                      if (*(int *)(&local_f5c.wSecond + local_13e8 * 2) < 0) {
                        FUN_0058f078();
                      }
                      local_149c = *(int *)(&local_f5c.wSecond + local_13e8 * 2);
                      if (local_149c < 0) {
                        local_149c = 0;
                      }
                      local_1418[0] = (undefined1 *)0x6;
                      if (local_13e8 == local_f88[local_f9c]) {
                        local_1418[0] = (undefined1 *)0xfffffffe;
                      }
                      if (local_f9c == local_f98) {
                        local_1418[0] = (undefined1 *)0xfffffffe;
                      }
                      pvVar7 = operator_new(0xe68);
                      local_8 = 6;
                      if (pvVar7 == (void *)0x0) {
                        local_1b00 = (undefined1 *)0x0;
                      }
                      else {
                        puStackY_34 = (undefined1 *)0x0;
                        piStackY_38 = (int *)0x0;
                        puStackY_3c = (undefined1 *)0xffffffff;
                        pcStackY_40 = local_1498;
                        puStackY_44 = local_1418[0];
                        puStackY_48 = (undefined1 *)0x46b34f;
                        local_1b00 = (undefined1 *)FUN_004072f2();
                      }
                      local_8 = 0xffffffff;
                      local_1418[local_13e8] = local_1b00;
                      if (*(int *)(&local_f5c.wSecond + local_13e8 * 2) < 0) {
                        local_1418[local_13e8][0xe64] = 0;
                      }
                      FUN_00408d33();
                    }
LAB_0046af52:
                  }
                  (**(code **)(*local_f90 + 0x60))();
                  if (extraout_ECX[0xa2e4] != 0) {
                    (**(code **)(*(int *)extraout_ECX[0xa2e4] + 0x60))();
                  }
                }
              }
              else {
                if (param_1 == 0x3003) {
                  iVar14 = (**(code **)(*DAT_0092e654 + 8))();
                  extraout_ECX[0x9c4f] = iVar14;
                  local_153e = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                  local_1540 = 0x3ae;
                  local_1538 = 0;
                  FUN_0055f2dd();
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x3001) {
                  local_154c = extraout_ECX[0xa2ee];
                  local_1548 = FUN_00409046();
                  if (699 < *(int *)(local_1548 + 0xe60)) {
                    FUN_00403df2();
                    (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                    ExceptionList = local_10;
                    return 1;
                  }
                  extraout_ECX[0xa303] = param_2 + 1;
                  extraout_ECX[0x9c51] = uVar4;
                  *(undefined1 *)((int)extraout_ECX + 0x271d1) = 1;
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x3101) {
                  if (extraout_ECX[0x9b8f] == 1) {
                    ExceptionList = local_10;
                    return 1;
                  }
                  if (*(short *)((int)extraout_ECX + 0x2717e) == 0) {
                    FUN_0044a9e9();
                  }
                  else {
                    *(short *)(extraout_ECX + 0x9c66) = (short)param_2 + 1;
                  }
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x3102) {
                  if (extraout_ECX[0x9b8f] == 1) {
                    local_1550 = FUN_00493cb0();
                    local_1550 = local_1550 + 1;
                    if ((local_1550 != 0) && (extraout_ECX[0x9b87] == 0)) {
                      FUN_0047f7da();
                    }
                    FUN_0047fcd7();
                    ExceptionList = local_10;
                    return 1;
                  }
                  if ((short)extraout_ECX[0x9c66] == 0) {
                    local_1554 = extraout_ECX[0xa305];
                    sVar2 = FUN_00493cb0();
                    *(short *)(extraout_ECX + 0x9c66) = sVar2 + 1;
                  }
                  if (*(short *)((int)extraout_ECX + 0x2717e) != 0) {
                    local_1558 = (int *)extraout_ECX[0xa1cf];
                    local_1564 = 0;
                    local_155c = extraout_ECX[0x9c62] % 9;
                    local_1560 = extraout_ECX[0x9c62] / 9;
                    local_1564 = (**(code **)(*local_1558 + 0xb4))();
                    if (local_1564 != 0) {
                      local_1580 = FUN_0054e93b();
                      if (local_1580 < 2) {
                        local_1584 = (undefined4 *)(**(code **)(*local_1558 + 0xa4))();
                        if ((DAT_005ccec0[0x7a] != 0) &&
                           ((undefined4 *)DAT_005ccec0[0x7a] == local_1584)) {
                          DAT_005ccec0[0x7a] = 0;
                        }
                        if (local_1584 != (undefined4 *)0x0) {
                          if (local_1584 != (undefined4 *)0x0) {
                            (**(code **)*local_1584)();
                          }
                          local_1584 = (undefined4 *)0x0;
                        }
                      }
                      else {
                        FUN_0054e9ab();
                        local_1588 = local_1564;
                        FUN_0058f078();
                        FUN_00421b8f();
                      }
                      if (local_1580 < 2) {
                        if (extraout_ECX[0x9c61] == 0) {
                          _memset(DAT_013b71e8 + extraout_ECX[0x9c62] * 2 + 0x1d2,0,8);
                        }
                        else if (extraout_ECX[0x9c61] == 1) {
                          _memset(DAT_013b71e8 + extraout_ECX[0x9c62] * 2 + 0x1f2,0,8);
                        }
                        else if (extraout_ECX[0x9c61] == 2) {
                          _memset(DAT_013b71e8 + extraout_ECX[0x9c62] * 2 + 0xbb,0,8);
                        }
                      }
                      iVar14 = (**(code **)(*DAT_0092e654 + 8))();
                      extraout_ECX[0x9d1d] = iVar14;
                      extraout_ECX[0x9c51] = extraout_ECX[0x9d1d];
                      *(undefined1 *)((int)extraout_ECX + 0x271d1) = 1;
                      local_157c = *(undefined4 *)(extraout_ECX[0x13] + 0x28);
                      local_1578 = *(undefined4 *)(extraout_ECX[0x13] + 0x2c);
                      _memset(extraout_ECX + 0x9c55,0,0x24);
                      *(short *)((int)extraout_ECX + 0x2715a) = (short)DAT_013b71e8[0x317];
                      *(short *)(extraout_ECX + 0x9c56) = (short)extraout_ECX[0x9c5f];
                      extraout_ECX[0x9c58] = extraout_ECX[0x9c61];
                      extraout_ECX[0x9c59] = extraout_ECX[0x9c62];
                      *(short *)(extraout_ECX + 0x9c5d) = (short)extraout_ECX[0x9c66];
                      uVar3 = __ftol();
                      *(undefined2 *)(extraout_ECX + 0x9c5c) = uVar3;
                      uVar3 = __ftol();
                      *(undefined2 *)((int)extraout_ECX + 0x27172) = uVar3;
                      _memset(extraout_ECX + 0x9c5e,0,0x24);
                      local_156e = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                      local_1570 = 0x3ae;
                      local_1568 = 1;
                      FUN_0055f2dd();
                    }
                  }
                  FUN_0044a9e9();
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x3103) {
                  if (extraout_ECX[0x9b8f] == 1) {
                    FUN_0047fcd7();
                  }
                  _memset(extraout_ECX + 0x9c5e,0,0x24);
                  FUN_0044a9e9();
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x27a) {
                  iVar14 = (**(code **)(*DAT_0092e654 + 8))();
                  extraout_ECX[0x9c4e] = iVar14;
                  local_1592 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                  local_1594 = 0x3ae;
                  local_158c = 0;
                  FUN_0055f2dd();
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x27c) {
                  (**(code **)(*(int *)extraout_ECX[0x9f1c] + 0x60))();
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == 0x281) {
                  local_15c8 = (**(code **)(*DAT_013b71e8 + 0x34))();
                  if (local_15c8 == 0) {
                    extraout_ECX[0x9ba5] = 0;
                    ExceptionList = local_10;
                    return 1;
                  }
                  if (*(char *)(local_15c8 + 0x248) != '\0') {
                    extraout_ECX[0x9ba5] = 0;
                    ExceptionList = local_10;
                    return 1;
                  }
                  if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                    extraout_ECX[0x9ba5] = 0;
                    ExceptionList = local_10;
                    return 1;
                  }
                  if ((*(int *)(extraout_ECX[0xa1cb] + 0x214) != 0) &&
                     (*(int *)(*(int *)(extraout_ECX[0xa1cb] + 0x214) + 0xe5c) == 1)) {
                    FUN_00403df2();
                    extraout_ECX[0x9ba5] = 0;
                    ExceptionList = local_10;
                    return 1;
                  }
                  _memset(local_15c4,0,0x2c);
                  local_15c0 = 0x37f;
                  local_15be = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                  local_15b8 = *(char *)(extraout_ECX[0x13] + 0x5c) + -1;
                  local_15b7 = 0;
                  local_15b6 = *(undefined2 *)(extraout_ECX[0x13] + 0x444);
                  local_15b2 = *(undefined2 *)(extraout_ECX[0x13] + 0x450);
                  local_15b4 = *(undefined2 *)(extraout_ECX[0x13] + 0x44c);
                  local_15b0 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                  FUN_0058f078();
                  local_159c = (uint)*(ushort *)(extraout_ECX + 0x9bb6);
                  FUN_0055f2dd();
                  extraout_ECX[0x9bb6] = 0;
                  (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                  extraout_ECX[0x9ba5] = 0;
                }
                else {
                  if (param_1 == 0x283) {
                    if (extraout_ECX[0x9ba5] != 0) {
                      __ftol();
                      __ftol();
                      __ftol();
                      __ftol();
                      iVar14 = FUN_005563a0();
                      if ((float)iVar14 <=
                          (*(float *)(DAT_013b71e8[0x6c21] + 0xc0) - _DAT_005a4394) + _DAT_005a3794)
                      {
                        extraout_ECX[0x9ba5] = 0;
                        if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                          ExceptionList = local_10;
                          return 1;
                        }
                        local_15dc.left = 0xa29;
                        local_15dc.top = 0x6a6;
                        local_15dc.right = 0xa5c;
                        local_15dc.bottom = 0x6d6;
                        local_15e8 = __ftol();
                        local_15e4 = __ftol();
                        pt.y = local_15e4;
                        pt.x = local_15e8;
                        local_15cc = PtInRect(&local_15dc,pt);
                        if (local_15cc == 1) {
                          ExceptionList = local_10;
                          return 1;
                        }
                        local_15e0 = extraout_ECX[0xa17c];
                        if (((*(short *)((int)DAT_013b71e8 + 0xcfa) != 0) && (local_15e0 != 0)) &&
                           (iVar14 = FUN_0040c0f0(), iVar14 == 1)) {
                          FUN_00403df2();
                          (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                          ExceptionList = local_10;
                          return 1;
                        }
                        *(undefined2 *)(DAT_013b71e8 + 0x319) = 899;
                        *(undefined2 *)((int)DAT_013b71e8 + 0xc66) =
                             *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                        *(short *)((int)DAT_013b71e8 + 0xcfa) = (short)extraout_ECX[0x9bb6];
                        FUN_0055f2dd();
                        extraout_ECX[0x9bb6] = 0;
                        (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                        goto LAB_0046ea98;
                      }
                    }
                    ExceptionList = local_10;
                    return 1;
                  }
                  if (param_1 == 0x282) {
                    (**(code **)(*(int *)extraout_ECX[0xa1ee] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1ed] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1ef] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1f0] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1f1] + 0x60))();
                    puStackY_34 = (undefined1 *)0x46c24d;
                    (**(code **)(*(int *)extraout_ECX[0xa1f2] + 0x60))();
                    puStackY_34 = (undefined1 *)0x0;
                    piStackY_38 = (int *)0x46c26c;
                    (**(code **)(*(int *)extraout_ECX[0xa1f3] + 0x60))();
                    piStackY_38 = (int *)0x0;
                    puStackY_3c = (undefined1 *)0x46c28b;
                    (**(code **)(*(int *)extraout_ECX[0xa1f4] + 0x60))();
                    puStackY_3c = (undefined1 *)0x1;
                    pcStackY_40 = (char *)0x46c2aa;
                    (**(code **)(*(int *)extraout_ECX[0xa1f5] + 0x60))();
                    pcStackY_40 = (char *)0x1;
                    puStackY_44 = (undefined1 *)0x46c2c9;
                    (**(code **)(*(int *)extraout_ECX[0xa1f6] + 0x60))();
                    puStackY_44 = (undefined1 *)0x1;
                    puStackY_48 = (undefined1 *)0x46c2e8;
                    (**(code **)(*(int *)extraout_ECX[0xa1f7] + 0x60))();
                    if ((((extraout_ECX[0x13] != 0) &&
                         (((0x20d < *(short *)(extraout_ECX[0x13] + 0x682) &&
                           (*(short *)(extraout_ECX[0x13] + 0x682) < 0x214)) ||
                          (*(short *)(extraout_ECX[0x13] + 0x682) == 0x1fd)))) &&
                        ((*(short *)(extraout_ECX[0x9ba5] + 0x680) == 0 ||
                         ((*(short *)(extraout_ECX[0x9ba5] + 0x680) ==
                           *(short *)(extraout_ECX[0x13] + 0x680) &&
                          ((*(short *)(extraout_ECX[0x9ba5] + 0x682) == 0x1fc ||
                           (*(short *)(extraout_ECX[0x9ba5] + 0x682) == 0x20a)))))))) &&
                       (*(short *)(extraout_ECX[0x9ba5] + 0x682) != 0x1fd)) {
                      puStackY_48 = (undefined1 *)0x1;
                      piStackY_4c = (int *)0x46c3f5;
                      (**(code **)(*(int *)extraout_ECX[0xa1f8] + 0x60))();
                    }
                    puStackY_48 = (undefined1 *)0x0;
                    piStackY_4c = (int *)0x46c414;
                    (**(code **)(*(int *)extraout_ECX[0xa1f9] + 0x60))();
                    piStackY_4c = (int *)0x0;
                    pcStackY_50 = "j";
                    (**(code **)(*(int *)extraout_ECX[0xa1fa] + 0x60))();
                    pcStackY_50 = (char *)0x0;
                    puStackY_54 = (undefined1 *)0x46c452;
                    (**(code **)(*(int *)extraout_ECX[0xa1fb] + 0x60))();
                    puStackY_54 = (undefined1 *)0x0;
                    uStackY_58 = 0x46c471;
                    (**(code **)(*(int *)extraout_ECX[0xa1fc] + 0x60))();
                  }
                  else if (param_1 == 0x35f) {
                    (**(code **)(*(int *)extraout_ECX[0xa1f5] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1f6] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1f7] + 0x60))();
                    (**(code **)(*(int *)extraout_ECX[0xa1f8] + 0x60))();
                    if (extraout_ECX[0x13] != 0) {
                      if (*(short *)(extraout_ECX[0x9ba5] + 0x682) != 0x1fc) {
                        (**(code **)(*(int *)extraout_ECX[0xa1f9] + 0x60))();
                      }
                      if ((*(short *)(extraout_ECX[0x13] + 0x682) == 0x1fd) &&
                         (((*(short *)(extraout_ECX[0x9ba5] + 0x682) < 0x20e ||
                           (0x213 < *(short *)(extraout_ECX[0x9ba5] + 0x682))) &&
                          (*(short *)(extraout_ECX[0x13] + 0x680) ==
                           *(short *)(extraout_ECX[0x9ba5] + 0x680))))) {
                        (**(code **)(*(int *)extraout_ECX[0xa1fa] + 0x60))();
                        (**(code **)(*(int *)extraout_ECX[0xa1fb] + 0x60))();
                        (**(code **)(*(int *)extraout_ECX[0xa1fc] + 0x60))();
                      }
                    }
                  }
                  else if (param_1 == 0x390) {
                    if (extraout_ECX[0x9ba5] != 0) {
                      local_1600 = DAT_0067cf38;
                      if (DAT_0067cf38 == 0) {
                        ExceptionList = local_10;
                        return 0;
                      }
                      if (*(short *)(extraout_ECX[0x9ba5] + 0x682) != 0) {
                        FUN_00403df2();
                        (**(code **)(**(int **)(local_1600 + 0x84) + 0x88))();
                        ExceptionList = local_10;
                        return 0;
                      }
                      if (DAT_013b71e8[0x1c1] < 1000000) {
                        FUN_00403df2();
                        (**(code **)(**(int **)(local_1600 + 0x84) + 0x88))();
                        ExceptionList = local_10;
                        return 0;
                      }
                      _memset(local_15fc,0,0x14);
                      local_15f8 = 0x3d5;
                      local_15f6 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                      local_15f0 = *(undefined4 *)(extraout_ECX[0x9ba5] + 0x20);
                      local_15ec = 0;
                      FUN_0055f2dd();
                      (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                      extraout_ECX[0x9ba5] = 0;
                    }
                  }
                  else if ((param_1 < 0x391) || (0x393 < param_1)) {
                    if (param_1 == 0x330) {
                      extraout_ECX[0x9ba5] = 0;
                      iVar14 = FUN_0040c0f0();
                      if (iVar14 == 0) {
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
                        *(int *)(extraout_ECX[0x23] + 0x1e8) = extraout_ECX[0x9bb6];
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
                        ExceptionList = local_10;
                        return 1;
                      }
                      (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                    }
                    else if (param_1 == 0x331) {
                      iVar14 = FUN_0040c0f0();
                      if (iVar14 == 0) {
                        if (extraout_ECX[0x9ba5] == 0) {
                          ExceptionList = local_10;
                          return 1;
                        }
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
                        *(uint *)(extraout_ECX[0x23] + 0x1e8) =
                             (uint)*(ushort *)(extraout_ECX[0x9ba5] + 0x680);
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
                        extraout_ECX[0x9ba5] = 0;
                        ExceptionList = local_10;
                        return 1;
                      }
                      (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                    }
                    else if (param_1 == 0x332) {
                      extraout_ECX[0x9ba5] = 0;
                      iVar14 = FUN_0040c0f0();
                      if (iVar14 == 0) {
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
                        *(int *)(extraout_ECX[0x23] + 0x1e8) = extraout_ECX[0x9bb6];
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
                        ExceptionList = local_10;
                        return 1;
                      }
                      (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                    }
                    else if (param_1 == 0x35e) {
                      iVar14 = FUN_0040c0f0();
                      if (iVar14 == 0) {
                        if (extraout_ECX[0x9ba5] == 0) {
                          ExceptionList = local_10;
                          return 1;
                        }
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
                        *(uint *)(extraout_ECX[0x23] + 0x1e8) =
                             (uint)*(ushort *)(extraout_ECX[0x9ba5] + 0x680);
                        (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
                        extraout_ECX[0x9ba5] = 0;
                        ExceptionList = local_10;
                        return 1;
                      }
                      (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                    }
                    else if (param_1 == 0x284) {
                      extraout_ECX[0x9ba5] = 0;
                      (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                    }
                    else {
                      if (param_1 == 0x29c) {
                        FUN_0044ae38();
                        ExceptionList = local_10;
                        return 1;
                      }
                      if (param_1 == 0x26c) {
                        local_1608 = extraout_ECX[0x9ba5];
                        if (extraout_ECX[0x9f1d] != 0) {
                          (**(code **)(*(int *)extraout_ECX[0xa1ed] + 0x60))();
                          (**(code **)(*(int *)extraout_ECX[0xa1ee] + 0x60))();
                          (**(code **)(*(int *)extraout_ECX[0xa1ef] + 0x60))();
                          (**(code **)(*(int *)extraout_ECX[0xa1f0] + 0x60))();
                          (**(code **)(*(int *)extraout_ECX[0xa1f1] + 0x60))();
                          puStackY_34 = (undefined1 *)0x46ce84;
                          (**(code **)(*(int *)extraout_ECX[0xa1f2] + 0x60))();
                          puStackY_34 = (undefined1 *)0x1;
                          piStackY_38 = (int *)0x46cea3;
                          (**(code **)(*(int *)extraout_ECX[0xa1f3] + 0x60))();
                          piStackY_38 = (int *)0x1;
                          puStackY_3c = (undefined1 *)0x46cec2;
                          (**(code **)(*(int *)extraout_ECX[0xa1f4] + 0x60))();
                          puStackY_3c = (undefined1 *)0x1;
                          pcStackY_40 = "j";
                          (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                          pcStackY_40 = (char *)0x0;
                          puStackY_44 = (undefined1 *)0x46cf00;
                          (**(code **)(*(int *)extraout_ECX[0xa1f9] + 0x60))();
                          puStackY_44 = (undefined1 *)0x0;
                          puStackY_48 = (undefined1 *)0x46cf1f;
                          (**(code **)(*(int *)extraout_ECX[0xa1fa] + 0x60))();
                          puStackY_48 = (undefined1 *)0x0;
                          piStackY_4c = (int *)0x46cf3e;
                          (**(code **)(*(int *)extraout_ECX[0xa1fb] + 0x60))();
                          piStackY_4c = (int *)0x0;
                          pcStackY_50 = (char *)0x46cf5d;
                          (**(code **)(*(int *)extraout_ECX[0xa1fc] + 0x60))();
                        }
                      }
                      else if (((param_1 == 0x27f) || (param_1 == 0x26d)) ||
                              ((param_1 == 0x26e || (param_1 == 0x26f)))) {
                        local_160c = extraout_ECX[0x9ba5];
                        if (local_160c != 0) {
                          _memset(local_1620,0,0x14);
                          local_161c = 0x39f;
                          local_161a = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1614 = *(undefined4 *)(local_160c + 0x20);
                          if (param_1 == 0x27f) {
                            local_1610 = 0;
                          }
                          else if (param_1 == 0x26d) {
                            local_1610 = 1;
                          }
                          else if (param_1 == 0x26e) {
                            local_1610 = 2;
                          }
                          else if (param_1 == 0x26f) {
                            local_1610 = 3;
                          }
                          FUN_0055f2dd();
                          extraout_ECX[0x9ba5] = 0;
                          if (extraout_ECX[0x9f1d] != 0) {
                            (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                          }
                        }
                      }
                      else if (param_1 == 0x1209) {
                        if (param_2 == 0) {
                          uVar9 = FUN_004640e5();
                          ExceptionList = local_10;
                          return uVar9;
                        }
                        if (DAT_013b722c == 1) {
                          (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
                        }
                      }
                      else {
                        if (param_1 == 0x431) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if ((short)DAT_013b71e8[0x273] < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          _memset(local_1634,0,0x14);
                          local_162e = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1630 = 0x277;
                          local_1628 = 0;
                          local_1626 = 0;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x434) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if ((short)DAT_013b71e8[0x273] < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          _memset(local_1648,0,0x14);
                          local_1642 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1644 = 0x277;
                          local_163c = 0;
                          local_163a = 1;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x437) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if ((short)DAT_013b71e8[0x273] < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          _memset(local_165c,0,0x14);
                          local_1656 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1658 = 0x277;
                          local_1650 = 0;
                          local_164e = 2;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x440) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if ((short)DAT_013b71e8[0x273] < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          _memset(local_1670,0,0x14);
                          local_166a = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_166c = 0x277;
                          local_1664 = 0;
                          local_1662 = 3;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x473) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if (*(short *)((int)DAT_013b71e8 + 0x9ce) < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          local_1688 = (((short)DAT_013b71e8[0x1cb] + 1) * 3) / 2;
                          iVar6 = 0x46d3ca;
                          iVar14 = FUN_004ff388();
                          if (iVar14 == 2) {
                            local_1688 = 200;
                          }
                          if (local_1688 <= (int)(uint)*(byte *)(iVar6 + 0x58)) {
                            FUN_00403df2();
                            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                            ExceptionList = local_10;
                            return 1;
                          }
                          _memset(local_1684,0,0x14);
                          local_167e = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1680 = 0x277;
                          local_1678 = 1;
                          local_1676 = 0;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x476) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if (*(short *)((int)DAT_013b71e8 + 0x9ce) < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          local_16a0 = (((short)DAT_013b71e8[0x1cb] + 1) * 3) / 2;
                          iVar14 = FUN_004ff388();
                          if (iVar14 == 2) {
                            local_16a0 = 200;
                          }
                          iVar6 = 0x1f;
                          iVar14 = FUN_00434315();
                          if (iVar14 == 1) {
                            local_16a0 = 0xff;
                          }
                          else if (200 < local_16a0) {
                            local_16a0 = 200;
                          }
                          if (local_16a0 <= (int)(uint)*(byte *)(iVar6 + 0x59)) {
                            FUN_00403df2();
                            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                            ExceptionList = local_10;
                            return 1;
                          }
                          _memset(local_169c,0,0x14);
                          local_1696 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1698 = 0x277;
                          local_1690 = 1;
                          local_168e = 1;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x479) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if (*(short *)((int)DAT_013b71e8 + 0x9ce) < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          local_16b8 = (((short)DAT_013b71e8[0x1cb] + 1) * 3) / 2;
                          iVar14 = FUN_004ff388();
                          if (iVar14 == 2) {
                            local_16b8 = 200;
                          }
                          iVar6 = 0x27;
                          iVar14 = FUN_00434315();
                          if (iVar14 == 1) {
                            local_16b8 = 0xff;
                          }
                          else if (200 < local_16b8) {
                            local_16b8 = 200;
                          }
                          if (local_16b8 <= (int)(uint)*(byte *)(iVar6 + 0x5a)) {
                            FUN_00403df2();
                            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                            ExceptionList = local_10;
                            return 1;
                          }
                          _memset(local_16b4,0,0x14);
                          local_16ae = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_16b0 = 0x277;
                          local_16a8 = 1;
                          local_16a6 = 2;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x482) {
                          if (*(char *)(extraout_ECX[0x13] + 0x245) == '\x01') {
                            ExceptionList = local_10;
                            return 1;
                          }
                          if (*(short *)((int)DAT_013b71e8 + 0x9ce) < 1) {
                            ExceptionList = local_10;
                            return 0;
                          }
                          local_16d0 = (((short)DAT_013b71e8[0x1cb] + 1) * 3) / 2;
                          iVar14 = FUN_004ff388();
                          if (iVar14 == 2) {
                            local_16d0 = 200;
                          }
                          iVar6 = 0x2f;
                          iVar14 = FUN_00434315();
                          if (iVar14 == 1) {
                            local_16d0 = 0xff;
                          }
                          else if (200 < local_16d0) {
                            local_16d0 = 200;
                          }
                          if (local_16d0 <= (int)(uint)*(byte *)(iVar6 + 0x5b)) {
                            FUN_00403df2();
                            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                            ExceptionList = local_10;
                            return 1;
                          }
                          _memset(local_16cc,0,0x14);
                          local_16c6 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_16c8 = 0x277;
                          local_16c0 = 1;
                          local_16be = 3;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x747) {
                          local_16d8 = extraout_ECX[0xa1cb];
                          local_16d4 = (int *)FUN_00409046();
                          if (((local_16d4 == (int *)0x0) || (local_16d4[0x397] != 1)) ||
                             (*(int *)(local_16d8 + 0x1ec) != 0)) {
                            if (((local_16d4 == (int *)0x0) || (*(int *)(DAT_005d0504 + 0x438) != 1)
                                ) || ((local_16d4[0xf] != 1 || (*(int *)(local_16d8 + 0x1ec) != 0)))
                               ) {
                              if ((local_16d4 != (int *)0x0) && (*(int *)(local_16d8 + 0x1ec) == 1))
                              {
                                local_1784 = *(char *)((int)DAT_013b71e8 +
                                                      *(char *)((int)DAT_013b71e8 + 0xef5) + 0xef6);
                                local_178c = *(int *)(&DAT_0092ff04 + local_1784 * 0x60);
                                if ((8 < extraout_ECX[0x9b9e]) && (1 < local_178c)) {
                                  local_178c = local_178c + -1;
                                }
                                if (uVar4 < (uint)(*(int *)(&DAT_0092eaf8 + local_1784 * 4) +
                                                  local_178c * 1000)) {
                                  ExceptionList = local_10;
                                  return 1;
                                }
                                local_1788 = (uint)*(short *)(extraout_ECX[0x13] + 0x444);
                                local_1780 = (int)local_1784 +
                                             (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18;
                                if (local_1784 < '`') {
                                  local_1788 = (uint)*(byte *)((int)DAT_013b71e8 +
                                                              ((int)(local_1780 +
                                                                    (local_1780 >> 0x1f & 7U)) >> 3)
                                                              + 0x745);
                                }
                                iVar6 = FUN_00555790();
                                if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar6) {
                                  local_1790 = extraout_ECX[0x9eba];
                                  pvVar7 = operator_new(0xe50);
                                  local_8 = 8;
                                  if (pvVar7 == (void *)0x0) {
                                    local_1b14 = 0;
                                  }
                                  else {
                                    puStackY_34 = (undefined1 *)0x0;
                                    piStackY_38 = (int *)0x0;
                                    puStackY_3c = (undefined1 *)0xffffaaaa;
                                    pcStackY_40 = &DAT_00a35078;
                                    puStackY_44 = (undefined1 *)0x46dca5;
                                    local_1b14 = FUN_00407203();
                                  }
                                  local_8 = 0xffffffff;
                                  local_1794 = local_1b14;
                                  FUN_00408d33();
                                  if ((DAT_005ccf98 != 0) && (iVar14 = FUN_00429a6d(), iVar14 != 0))
                                  {
                                    FUN_00429a6d();
                                    FUN_0042ad2b();
                                  }
                                  ExceptionList = local_10;
                                  return 1;
                                }
                                if (((extraout_ECX[0x9d18] + 1000U < uVar4) && (local_1784 == '*'))
                                   && (local_16d4[0x394] == *(int *)(extraout_ECX[0x13] + 0x20))) {
                                  FUN_00403df2();
                                  (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                                  ExceptionList = local_10;
                                  return 1;
                                }
                                if ((extraout_ECX[0x9d18] + 1000U < uVar4) &&
                                   (((((local_1784 == '\x19' || (local_1784 == '\x1b')) ||
                                      ((local_1784 == '*' ||
                                       ((local_1784 == '+' || (local_1784 == ',')))))) ||
                                     (local_1784 == '-')) || (local_1784 == '\x03')))) {
                                  if (local_1784 != '*') {
                                    local_186c = (**(code **)(*DAT_013b71e8 + 0x34))();
                                    if (local_186c == 0) {
                                      ExceptionList = local_10;
                                      return 1;
                                    }
                                    if (*(char *)(local_186c + 0x248) == '\0') {
                                      ExceptionList = local_10;
                                      return 1;
                                    }
                                    local_1874 = __ftol();
                                    local_1880 = __ftol();
                                    if (extraout_ECX[0xa1e5] != 0) {
                                      local_1874 = extraout_ECX[0xa1e5];
                                      local_1880 = extraout_ECX[0xa1e6];
                                    }
                                    local_1860 = __ftol();
                                    local_1868 = __ftol();
                                    local_187c = FUN_005563a0();
                                    local_1878 = *(int *)(&DAT_0092ff08 + local_1784 * 0x60) +
                                                 iVar14;
                                    local_1858 = local_1860;
                                    local_185c = local_1868;
                                    puStackY_34 = (undefined1 *)0x46df90;
                                    FUN_00556416();
                                    if (((local_1878 < local_187c) || (local_1858 != local_1860)) ||
                                       (local_185c != local_1868)) {
                                      ExceptionList = local_10;
                                      return 1;
                                    }
                                    _local_1864 = CONCAT31(uStackY_1863,
                                                           (&DAT_00934178)
                                                           [((int)(local_1874 +
                                                                  (local_1874 >> 0x1f & 3U)) >> 2) +
                                                            ((int)(local_1880 +
                                                                  (local_1880 >> 0x1f & 3U)) >> 2) *
                                                            0x400]);
                                    _local_1870 = CONCAT31(uStackY_186f,
                                                           (&DAT_00934178)
                                                           [((int)(local_1860 +
                                                                  (local_1860 >> 0x1f & 3U)) >> 2) +
                                                            ((int)(local_1868 +
                                                                  (local_1868 >> 0x1f & 3U)) >> 2) *
                                                            0x400]);
                                    if ((((&DAT_00934178)
                                          [((int)(local_1874 + (local_1874 >> 0x1f & 3U)) >> 2) +
                                           ((int)(local_1880 + (local_1880 >> 0x1f & 3U)) >> 2) *
                                           0x400] & 0x40) == 0) &&
                                       (((&DAT_00934178)
                                         [((int)(local_1860 + (local_1860 >> 0x1f & 3U)) >> 2) +
                                          ((int)(local_1868 + (local_1868 >> 0x1f & 3U)) >> 2) *
                                          0x400] & 0x40) != 0)) {
                                      ExceptionList = local_10;
                                      return 1;
                                    }
                                  }
                                  _memset(local_1854,0,0x60);
                                  local_1850 = 0x36c;
                                  local_184e = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                                  local_1848 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                                  local_1844 = __ftol();
                                  local_1842 = __ftol();
                                  local_183a = 0xffff;
                                  local_183c = (short)local_1784;
                                  local_1837 = 0;
                                  local_1838 = 0xff;
                                  local_1828 = (undefined2)local_16d4[0x394];
                                  local_1826 = 0xffff;
                                  local_1840 = __ftol();
                                  local_183e = __ftol();
                                  if (extraout_ECX[0xa1e5] != 0) {
                                    local_1844 = (undefined2)extraout_ECX[0xa1e5];
                                    local_1842 = (undefined2)extraout_ECX[0xa1e6];
                                    local_1840 = local_1844;
                                    local_183e = local_1842;
                                  }
                                  FUN_0055f2dd();
                                  FUN_0058f220();
                                  local_17ee = (undefined2)extraout_ECX[8];
                                  local_17d6 = 1;
                                  if (iVar14 != 0) {
                                    local_17d5 = local_17d5 | 8;
                                  }
                                  (**(code **)(*extraout_ECX + 4))();
                                  extraout_ECX[0x9d18] = uVar4;
                                  *(uint *)(&DAT_0092eaf8 + local_1784 * 4) = uVar4;
                                }
                              }
                            }
                            else if (*(ushort *)(local_16d4 + 0x394) < 1000) {
                              (**(code **)(*local_16d4 + 0x88))();
                              FUN_0058f078();
                              (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
                              *(uint *)(extraout_ECX[0x23] + 0x1e8) =
                                   (uint)*(ushort *)(local_16d4 + 0x394);
                              (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
                            }
                          }
                          else {
                            _memset(local_16fc,0,0x20);
                            local_16f6 = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                            local_16f8 = 0x3ab;
                            local_16f0 = (undefined2)local_16d4[0x394];
                            (**(code **)(*local_16d4 + 0x88))();
                            FUN_0058f078();
                            local_16dc = (**(code **)(*DAT_013b71e8 + 0x34))();
                            if (local_16dc != 0) {
                              *(undefined1 *)(local_16dc + 0x248) = 1;
                            }
                            FUN_0055f2dd();
                          }
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x748) {
                          _memset(local_1890,0,0x10);
                          local_188c = 0x37e;
                          local_188a = *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                          local_1884 = 0;
                          FUN_0055f2dd();
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x749) {
                          iVar14 = FUN_0040c0f0();
                          if (iVar14 == 1) {
                            FUN_0044da6f();
                          }
                          ExceptionList = local_10;
                          return 1;
                        }
                        if (param_1 == 0x269) {
                          iVar14 = extraout_ECX[0x9d34];
                          uVar4 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
                          if (uVar4 < iVar14 + 2000U) {
                            FUN_00403df2();
                            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                            iVar14 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
                            extraout_ECX[0x9d34] = iVar14;
                            ExceptionList = local_10;
                            return 1;
                          }
                          local_1894 = (**(code **)(*(int *)extraout_ECX[10] + 0x48))();
                          *(uint *)(local_1894 + 0x1e8) = (uint)(*(int *)(local_1894 + 0x1e8) == 0);
                          *(undefined1 *)(DAT_013b71e8 + 0x33e) =
                               *(undefined1 *)(local_1894 + 0x1e8);
                          FUN_0058f220();
                          local_192c = 899;
                          FUN_0055f2dd();
                          iVar14 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
                          extraout_ECX[0x9d34] = iVar14;
                        }
                        else {
                          if (param_1 == 0x29b) {
                            local_1934 = 1;
                            for (local_193c = 0; local_193c < 0xc; local_193c = local_193c + 1) {
                              if (0 < (short)extraout_ECX[local_193c * 2 + 0x9d44]) {
                                local_1934 = 0;
                              }
                            }
                            if (local_1934 == 1) {
                              FUN_00403df2();
                              (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
                              ExceptionList = local_10;
                              return 1;
                            }
                            local_1938 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
                            *(undefined2 *)(extraout_ECX + 0x9d3c) = 0x397;
                            *(undefined2 *)((int)extraout_ECX + 0x275ae) =
                                 *(undefined2 *)(extraout_ECX[0x13] + 0x20);
                            FUN_0055f2dd();
                            local_1940 = (int *)(**(code **)(*(int *)extraout_ECX[10] + 0x48))();
                            if (local_1938 != (int *)0x0) {
                              (**(code **)(*local_1938 + 0x60))();
                            }
                            if (local_1940 != (int *)0x0) {
                              (**(code **)(*local_1940 + 0x60))();
                            }
                            local_1944 = extraout_ECX[0x9ece];
                            if ((local_1944 != 0) && (iVar14 = FUN_0040c0f0(), iVar14 == 1)) {
                              local_1948 = (int *)extraout_ECX[0xa176];
                              (**(code **)(*local_1948 + 0x60))();
                              if (DAT_005b892c == 2) {
                                FUN_004012c6();
                              }
                              else {
                                uVar9 = __ftol();
                                *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar9;
                                FUN_004012c6();
                              }
                            }
                            ExceptionList = local_10;
                            return 1;
                          }
                          if ((param_1 == 0x335) || (param_1 == 0x336)) {
                            (**(code **)(*(int *)extraout_ECX[0x9f4e] + 0x60))();
                            (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
                            ExceptionList = local_10;
                            return 1;
                          }
                          if (param_1 == 0x339) {
                            if (extraout_ECX[0x9f50] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f50] + 0x60))();
                            }
                            if (extraout_ECX[0x9f52] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f52] + 0x60))();
                            }
                            if (extraout_ECX[0x9f51] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f51] + 0x60))();
                            }
                            FUN_004735ba();
                          }
                          else if (param_1 == 0x33b) {
                            if (extraout_ECX[0x9f50] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f50] + 0x60))();
                            }
                            if (extraout_ECX[0x9f52] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f52] + 0x60))();
                            }
                            if (extraout_ECX[0x9f51] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f51] + 0x60))();
                            }
                            if (extraout_ECX[0x9f58] != 0) {
                              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
                            }
                            FUN_00474802();
                          }
                          else if ((param_1 == 0x343) || (param_1 == 0x34b)) {
                            if (extraout_ECX[0x9f50] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f50] + 0x60))();
                            }
                            if (extraout_ECX[0x9f52] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f52] + 0x60))();
                            }
                            if (extraout_ECX[0x9f51] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f51] + 0x60))();
                            }
                            if (extraout_ECX[0x9f53] != 0) {
                              *(undefined4 *)(extraout_ECX[0x9f53] + 0x1fc) = 0xffffffff;
                            }
                            if (extraout_ECX[0x9f57] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f57] + 0x90))();
                            }
                            FUN_004729d4();
                          }
                          else if (param_1 == 0x340) {
                            if (extraout_ECX[0x9f50] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f50] + 0x60))();
                            }
                            if (extraout_ECX[0x9f52] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f52] + 0x60))();
                            }
                            if (extraout_ECX[0x9f51] != 0) {
                              (**(code **)(*(int *)extraout_ECX[0x9f51] + 0x60))();
                            }
                            FUN_0047461c();
                            if (extraout_ECX[0x9f56] != 0) {
                              (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
                            }
                          }
                          else if (param_1 == 0x345) {
                            if (param_2 == 0) {
                              FUN_00474896();
                            }
                            ExceptionList = local_10;
                            return 1;
                          }
                        }
                      }
                    }
                  }
                  else {
                    local_1604 = DAT_0067cf38;
                    if (DAT_0067cf38 == 0) {
                      ExceptionList = local_10;
                      return 0;
                    }
                    if (extraout_ECX[0x9ba5] != 0) {
                      if (*(short *)(extraout_ECX[0x9ba5] + 0x680) == 0) {
                        FUN_00403df2();
                        (**(code **)(**(int **)(local_1604 + 0x84) + 0x88))();
                        ExceptionList = local_10;
                        return 0;
                      }
                      if (*(short *)(extraout_ECX[0x9ba5] + 0x680) !=
                          *(short *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x680)) {
                        FUN_00403df2();
                        (**(code **)(**(int **)(local_1604 + 0x84) + 0x88))();
                        ExceptionList = local_10;
                        return 0;
                      }
                      if ((0x20d < *(short *)(extraout_ECX[0x9ba5] + 0x682)) &&
                         (*(short *)(extraout_ECX[0x9ba5] + 0x682) < 0x211)) {
                        FUN_00403df2();
                        (**(code **)(**(int **)(local_1604 + 0x84) + 0x88))();
                        ExceptionList = local_10;
                        return 0;
                      }
                      if (DAT_013b71e8[0x1c1] < 50000000) {
                        FUN_00403df2();
                        (**(code **)(**(int **)(local_1604 + 0x84) + 0x88))();
                        ExceptionList = local_10;
                        return 0;
                      }
                    }
                    FUN_004657ab();
                    (**(code **)(*(int *)extraout_ECX[0x9f1d] + 0x60))();
                  }
                }
              }
            }
          }
        }
      }
    }
    else {
      for (iVar14 = 0; iVar14 < 6; iVar14 = iVar14 + 1) {
        pcVar5 = (char *)(**(code **)(*(int *)extraout_ECX[iVar14 + 0x9e2e] + 0x88))();
        if (pcVar5 != (char *)0x0) {
          iVar14 = 0x46652e;
          iVar6 = FUN_0059115a();
          if (param_1 - 0x80a == iVar6) {
            FUN_00403df2();
            (**(code **)(*(int *)extraout_ECX[0x21] + 0x88))();
            ExceptionList = local_10;
            return 1;
          }
        }
        if ((pcVar5 != (char *)0x0) && (*pcVar5 == '\0')) {
          FUN_0058f078();
          (**(code **)(*(int *)extraout_ECX[(int)&stack0x00009d86] + 0x80))();
          break;
        }
      }
    }
  }
LAB_0046ea98:
  if (param_1 == 0x347) {
    (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
    uVar9 = 1;
  }
  else if (param_1 == 0x341) {
    FUN_00474802();
    uVar9 = 1;
  }
  else if (param_1 == 0x349) {
    FUN_00474ce2();
    uVar9 = 1;
  }
  else if (param_1 == 0x34a) {
    FUN_00474ce2();
    uVar9 = 1;
  }
  else if (param_1 == 0x342) {
    (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
    (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
    uVar9 = 1;
  }
  else if (param_1 == 0x33f) {
    FUN_00474896();
    uVar9 = 1;
  }
  else if ((param_1 < 0x352) || (0x35b < param_1)) {
    if (param_1 == 0x350) {
      local_1950 = *(undefined4 *)(extraout_ECX[0x9f5a] + 0xa8);
      FUN_004729d4();
      uVar9 = 1;
    }
    else if (param_1 == 0x351) {
      local_1954 = *(undefined4 *)(extraout_ECX[0x9f5a] + 0xb0);
      FUN_004729d4();
      uVar9 = 1;
    }
    else if (param_1 == 0x35c) {
      local_1958 = *(undefined4 *)(extraout_ECX[0x9f5a] + 0xb4);
      FUN_004729d4();
      uVar9 = 1;
    }
    else if (param_1 == 0x35d) {
      local_195c = *(undefined4 *)(extraout_ECX[0x9f5a] + 0xac);
      FUN_004729d4();
      uVar9 = 1;
    }
    else if (param_1 == 0x149) {
      if (extraout_ECX[0x9f44] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f44] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f44] + 0x3c) = 1;
      }
      if (extraout_ECX[0x9f48] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f48] + 0x60))();
      }
      if (extraout_ECX[0x9f45] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f45] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f45] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f49] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f49] + 0x60))();
      }
      if (extraout_ECX[0x9f46] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f46] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f46] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f4a] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4a] + 0x60))();
      }
      if (extraout_ECX[0x9f47] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f47] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f47] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f4b] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4b] + 0x60))();
      }
      (**(code **)(*(int *)extraout_ECX[0x9f4c] + 0x60))();
      uVar9 = 1;
    }
    else if (param_1 == 0x14a) {
      if (extraout_ECX[0x9f44] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f44] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f44] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f48] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f48] + 0x60))();
      }
      if (extraout_ECX[0x9f45] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f45] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f45] + 0x3c) = 1;
      }
      if (extraout_ECX[0x9f49] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f49] + 0x60))();
      }
      if (extraout_ECX[0x9f46] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f46] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f46] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f4a] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4a] + 0x60))();
      }
      if (extraout_ECX[0x9f47] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f47] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f47] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f4b] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4b] + 0x60))();
      }
      (**(code **)(*(int *)extraout_ECX[0x9f4c] + 0x60))();
      uVar9 = 1;
    }
    else if (param_1 == 0x14b) {
      if (extraout_ECX[0x9f44] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f44] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f44] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f48] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f48] + 0x60))();
      }
      if (extraout_ECX[0x9f45] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f45] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f45] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f49] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f49] + 0x60))();
      }
      if (extraout_ECX[0x9f46] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f46] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f46] + 0x3c) = 1;
      }
      if (extraout_ECX[0x9f4a] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4a] + 0x60))();
      }
      if (extraout_ECX[0x9f47] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f47] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f47] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f4b] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4b] + 0x60))();
      }
      (**(code **)(*(int *)extraout_ECX[0x9f4c] + 0x60))();
      uVar9 = 1;
    }
    else if (param_1 == 0x14d) {
      if (extraout_ECX[0x9f44] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f44] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f44] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f48] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f48] + 0x60))();
      }
      if (extraout_ECX[0x9f45] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f45] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f45] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f49] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f49] + 0x60))();
      }
      if (extraout_ECX[0x9f46] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f46] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f46] + 0x3c) = 0;
      }
      if (extraout_ECX[0x9f4a] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4a] + 0x60))();
      }
      if (extraout_ECX[0x9f47] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f47] + 0x60))();
        *(undefined4 *)(extraout_ECX[0x9f47] + 0x3c) = 1;
      }
      if (extraout_ECX[0x9f4b] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f4b] + 0x60))();
      }
      (**(code **)(*(int *)extraout_ECX[0x9f4c] + 0x60))();
      uVar9 = 1;
    }
    else if (param_1 == 0x14c) {
      if (extraout_ECX[0x9f3e] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f3e] + 0x60))();
        (**(code **)(*(int *)extraout_ECX[0x9f4c] + 0x60))();
        local_1960 = DAT_013b71e8 + 0x1bb;
        puStackY_34 = (undefined1 *)0x46f571;
        FUN_0049e77b();
        puStackY_34 = (undefined1 *)0x46f5af;
        FUN_0049e77b();
        puStackY_34 = (undefined1 *)0x46f5ed;
        FUN_0049e77b();
        puStackY_34 = (undefined1 *)0x46f62b;
        FUN_0049e77b();
        if (*(char *)((int)extraout_ECX + (short)local_1960[0x10] + 0x276c0) == 'a') {
          (**(code **)(*extraout_ECX + 0x58))();
        }
        else if (*(char *)((int)extraout_ECX + (short)local_1960[0x10] + 0x276c0) == 'b') {
          (**(code **)(*extraout_ECX + 0x58))();
        }
        else if (*(char *)((int)extraout_ECX + (short)local_1960[0x10] + 0x276c0) == 'c') {
          (**(code **)(*extraout_ECX + 0x58))();
        }
        else if (*(char *)((int)extraout_ECX + (short)local_1960[0x10] + 0x276c0) == 'e') {
          (**(code **)(*extraout_ECX + 0x58))();
        }
      }
      uVar9 = 1;
    }
    else if (param_1 == 0x363) {
      if (DAT_005b892c == 2) {
        if (extraout_ECX[0x9f34] != 0) {
          (**(code **)(*(int *)extraout_ECX[0x9f34] + 0x60))();
        }
      }
      else if (extraout_ECX[0x9f2f] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f2f] + 0x60))();
      }
      if (extraout_ECX[0x9f30] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f30] + 0x60))();
      }
      if (extraout_ECX[0x9f31] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f31] + 0x60))();
      }
      if (extraout_ECX[0x9f32] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f32] + 0x60))();
      }
      uVar9 = 1;
    }
    else if (param_1 == 0x365) {
      if (DAT_005b892c == 2) {
        if (extraout_ECX[0x9f34] != 0) {
          (**(code **)(*(int *)extraout_ECX[0x9f34] + 0x60))();
        }
      }
      else if (extraout_ECX[0x9f2f] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f2f] + 0x60))();
      }
      if (extraout_ECX[0x9f30] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f30] + 0x60))();
      }
      if (extraout_ECX[0x9f31] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f31] + 0x60))();
      }
      if (extraout_ECX[0x9f32] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f32] + 0x60))();
      }
      uVar9 = 1;
    }
    else if (param_1 == 0x367) {
      if (DAT_005b892c == 2) {
        if (extraout_ECX[0x9f34] != 0) {
          (**(code **)(*(int *)extraout_ECX[0x9f34] + 0x60))();
        }
      }
      else if (extraout_ECX[0x9f2f] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f2f] + 0x60))();
      }
      if (extraout_ECX[0x9f30] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f30] + 0x60))();
      }
      if (extraout_ECX[0x9f31] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f31] + 0x60))();
      }
      if (extraout_ECX[0x9f32] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f32] + 0x60))();
      }
      uVar9 = 1;
    }
    else if (param_1 == 0x369) {
      if (DAT_005b892c == 2) {
        if (extraout_ECX[0x9f34] != 0) {
          (**(code **)(*(int *)extraout_ECX[0x9f34] + 0x60))();
        }
      }
      else if (extraout_ECX[0x9f2f] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f2f] + 0x60))();
      }
      if (extraout_ECX[0x9f30] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f30] + 0x60))();
      }
      if (extraout_ECX[0x9f31] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f31] + 0x60))();
      }
      if (extraout_ECX[0x9f32] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f32] + 0x60))();
      }
      (**(code **)(*(int *)extraout_ECX[0x9f3c] + 0x60))();
      uVar9 = 1;
    }
    else if (param_1 == 0x36f) {
      (**(code **)(*(int *)extraout_ECX[0x9f2a] + 0x60))();
      uVar9 = 1;
    }
    else if (param_1 == 0x36b) {
      if (extraout_ECX[0x9f2a] != 0) {
        (**(code **)(*(int *)extraout_ECX[0x9f2a] + 0x60))();
        (**(code **)(*(int *)extraout_ECX[0x9f29] + 0x8c))();
        (**(code **)(*extraout_ECX + 0x58))();
      }
      uVar9 = 1;
    }
    else {
      if (param_1 == 0x36e) {
        FUN_0058f078();
        (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
        (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
        (**(code **)(*(int *)extraout_ECX[0x9f3d] + 0x60))();
      }
      else if (param_1 == 0x362) {
        if (extraout_ECX[0x9f2a] != 0) {
          (**(code **)(*(int *)extraout_ECX[0x9f2a] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0x9f29] + 0x8c))();
        }
      }
      else if (param_1 == 0x143) {
        if (extraout_ECX[0x9f3e] != 0) {
          (**(code **)(*(int *)extraout_ECX[0x9f3e] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0x9f25] + 0x8c))();
        }
      }
      else {
        if (param_1 == 0x55f) {
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x17df) {
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1802) {
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1922) {
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1952) {
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x1972) {
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0x23] + 0x60))();
          ExceptionList = local_10;
          return 1;
        }
        if (param_1 == 0x24b) {
          (**(code **)(*(int *)extraout_ECX[0xa1c6] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0xa1c7] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0xa1c8] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0xa1c9] + 0x8c))();
          if ((DAT_005ccf98 != 0) && (iVar14 = FUN_00429a6d(), iVar14 != 0)) {
            puStackY_34 = (undefined1 *)0x35;
            piStackY_38 = (int *)0x470049;
            FUN_00429a6d();
            puStackY_34 = (undefined1 *)0x470050;
            FUN_0042ad2b();
          }
          (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
          if (DAT_005b892c == 2) {
            puStackY_34 = &DAT_005b8184;
            piStackY_38 = (int *)0x470099;
            (**(code **)(*(int *)extraout_ECX[0xa202] + 0x80))();
          }
          puStackY_34 = (undefined1 *)0x31;
          piStackY_38 = (int *)0x4700a8;
          FUN_004528c5();
          if (*(int *)(DAT_013b71e0 + 0x10c) == 0) {
            puStackY_34 = (undefined1 *)0x30;
            piStackY_38 = (int *)0x4700cb;
            (**(code **)(*extraout_ECX + 0x14))();
          }
        }
        else if (param_1 == 0x24c) {
          (**(code **)(*(int *)extraout_ECX[0xa1c6] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0xa1c7] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0xa1c8] + 0x8c))();
          (**(code **)(*(int *)extraout_ECX[0xa1c9] + 0x8c))();
          if ((DAT_005ccf98 != 0) && (iVar14 = FUN_00429a6d(), iVar14 != 0)) {
            puStackY_34 = (undefined1 *)0x35;
            piStackY_38 = (int *)0x47018a;
            FUN_00429a6d();
            puStackY_34 = (undefined1 *)0x470191;
            FUN_0042ad2b();
          }
          (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
          if (DAT_005b892c == 2) {
            puStackY_34 = &DAT_005b8188;
            piStackY_38 = (int *)0x4701da;
            (**(code **)(*(int *)extraout_ECX[0xa202] + 0x80))();
          }
          puStackY_34 = (undefined1 *)0x31;
          piStackY_38 = (int *)0x4701e9;
          FUN_004528c5();
          if (*(int *)(DAT_013b71e0 + 0x10c) == 0) {
            puStackY_34 = (undefined1 *)0x30;
            piStackY_38 = (int *)0x47020c;
            (**(code **)(*extraout_ECX + 0x14))();
          }
        }
        else if (param_1 == 0x13a) {
          (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
          FUN_004524ab();
        }
        else if (param_1 == 0x13c) {
          (**(code **)(*(int *)extraout_ECX[10] + 0x40))();
          FUN_00452737();
        }
        else if (param_1 == 0x13b) {
          local_1a68 = FUN_0040c0f0();
          (**(code **)(*(int *)extraout_ECX[0x9f3e] + 0x60))();
          (**(code **)(*(int *)extraout_ECX[0x9f25] + 0x8c))();
          local_1a64 = DAT_013b71e8 + 0x1bb;
          if (local_1a68 == 0) {
            puStackY_34 = (undefined1 *)0x470355;
            FUN_0049e77b();
            puStackY_34 = (undefined1 *)0x470393;
            FUN_0049e77b();
            puStackY_34 = (undefined1 *)0x4703d1;
            FUN_0049e77b();
            puStackY_34 = (undefined1 *)0x47040f;
            FUN_0049e77b();
          }
          if ((DAT_005ccf98 != 0) && (iVar14 = FUN_00429a6d(), iVar14 != 0)) {
            FUN_00429a6d();
            FUN_0042ad2b();
          }
        }
        else if (param_1 == 0x141) {
          FUN_0049e50f();
        }
        else if (param_1 == 0x145) {
          FUN_0049e50f();
        }
        else if (param_1 == 0x147) {
          FUN_0049e50f();
        }
        else if (param_1 == 0x14f) {
          FUN_0049e50f();
        }
        else if ((param_1 == 0x1640) && (DAT_005b892c == 2)) {
          if (extraout_ECX[0x9bba] == 3) {
            extraout_ECX[0x9bba] = 3;
          }
          else {
            extraout_ECX[0x9bba] = 2;
          }
          FUN_00452271();
        }
        else if ((param_1 == 0x1641) && (DAT_005b892c == 2)) {
          *(uint *)(extraout_ECX[0x9ebd] + 0x1e8) =
               (uint)(*(int *)(extraout_ECX[0x9ebd] + 0x1e8) == 0);
          *(undefined4 *)(extraout_ECX[0x9ec1] + 0x1e8) =
               *(undefined4 *)(extraout_ECX[0x9ebd] + 0x1e8);
        }
        else if ((param_1 == 0x1642) && (DAT_005b892c == 2)) {
          *(uint *)(extraout_ECX[0x9ebe] + 0x1e8) =
               (uint)(*(int *)(extraout_ECX[0x9ebe] + 0x1e8) == 0);
          *(undefined4 *)(extraout_ECX[0x9ec2] + 0x1e8) =
               *(undefined4 *)(extraout_ECX[0x9ebe] + 0x1e8);
        }
        else if ((param_1 == 0x1643) && (DAT_005b892c == 2)) {
          *(uint *)(extraout_ECX[0x9ebf] + 0x1e8) =
               (uint)(*(int *)(extraout_ECX[0x9ebf] + 0x1e8) == 0);
          *(undefined4 *)(extraout_ECX[0x9ec3] + 0x1e8) =
               *(undefined4 *)(extraout_ECX[0x9ebf] + 0x1e8);
        }
        else if ((param_1 == 0x1644) && (DAT_005b892c == 2)) {
          *(uint *)(extraout_ECX[0x9ec0] + 0x1e8) =
               (uint)(*(int *)(extraout_ECX[0x9ec0] + 0x1e8) == 0);
          *(undefined4 *)(extraout_ECX[0x9ec4] + 0x1e8) =
               *(undefined4 *)(extraout_ECX[0x9ec0] + 0x1e8);
        }
        else if ((param_1 == 0x1652) && (DAT_005b892c == 2)) {
          if (_DAT_005bde1c < _DAT_005a3660) {
            FUN_0044ca65();
          }
        }
        else if ((param_1 == 0x1653) && (DAT_005b892c == 2)) {
          if (_DAT_005a3660 < _DAT_005bde1c) {
            *(undefined4 *)(extraout_ECX[0x9f66] + 0x28) = 0;
            FUN_0044ca65();
          }
        }
        else if ((param_1 == 0x165a) && (DAT_005b892c == 2)) {
          (**(code **)(*(int *)extraout_ECX[0x9e8e] + 0x60))();
          *(uint *)(extraout_ECX[0x9e90] + 0x1e8) =
               (uint)(*(int *)(extraout_ECX[0x9e90] + 0x1e8) == 0);
        }
        else if ((param_1 == 0x1664) && (DAT_005b892c == 2)) {
          (**(code **)(*(int *)extraout_ECX[0x9e91] + 0x60))();
          *(uint *)(extraout_ECX[0x9e94] + 0x1e8) =
               (uint)(*(int *)(extraout_ECX[0x9e94] + 0x1e8) == 0);
        }
        else if ((param_1 == 0x1667) && (DAT_005b892c == 2)) {
          if ((extraout_ECX[0x9e96] != 0) && (extraout_ECX[0x9e98] != 0)) {
            (**(code **)(*(int *)extraout_ECX[0x9e96] + 0x60))();
            *(uint *)(extraout_ECX[0x9e98] + 0x1e8) =
                 (uint)(*(int *)(extraout_ECX[0x9e98] + 0x1e8) == 0);
          }
        }
        else if ((param_1 == 0x166e) && (DAT_005b892c == 2)) {
          if ((extraout_ECX[0x9f63] != 0) && (extraout_ECX[0x9f64] != 0)) {
            (**(code **)(*(int *)extraout_ECX[0x9f63] + 0x60))();
            *(uint *)(extraout_ECX[0x9f64] + 0x1e8) =
                 (uint)(*(int *)(extraout_ECX[0x9f64] + 0x1e8) == 0);
          }
        }
        else if ((param_1 == 0x17b4) && (DAT_005b892c == 2)) {
          if (0 < extraout_ECX[0x9f33]) {
            piVar10 = (int *)extraout_ECX[extraout_ECX[0x9f33] + 0x9f35];
            extraout_ECX[0x9f33] = extraout_ECX[0x9f33] + -1;
            (**(code **)(*piVar10 + 0x60))();
            (**(code **)(*(int *)extraout_ECX[extraout_ECX[0x9f33] + 0x9f35] + 0x60))();
          }
        }
        else if ((param_1 == 0x17b5) && (DAT_005b892c == 2)) {
          if (extraout_ECX[0x9f33] < 2) {
            piVar10 = (int *)extraout_ECX[extraout_ECX[0x9f33] + 0x9f35];
            extraout_ECX[0x9f33] = extraout_ECX[0x9f33] + 1;
            (**(code **)(*piVar10 + 0x60))();
            (**(code **)(*(int *)extraout_ECX[extraout_ECX[0x9f33] + 0x9f35] + 0x60))();
          }
        }
        else {
          if ((0x380 < param_1) && (param_1 < 0x385)) {
            (**(code **)(*(int *)extraout_ECX[0xa184] + 0x60))();
            _memset(local_1a78,0,0x10);
            local_1a72 = (undefined2)DAT_013b71e8[0x317];
            local_1a74 = 0x2c7;
            local_1a6c = param_1 - 0x381;
            FUN_0055f2dd();
            ExceptionList = local_10;
            return 1;
          }
          if ((0x2201 < param_1) && (param_1 < 0x2267)) {
            FUN_00470d43();
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x226a) {
            FUN_00470dcb();
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x2267) {
            if (extraout_ECX[0xa269] != 0) {
              (**(code **)(*(int *)extraout_ECX[0xa269] + 0x60))();
              *(undefined2 *)(extraout_ECX + 0x9c4c) = 0xffff;
              *(undefined2 *)((int)extraout_ECX + 0x27132) = 0xffff;
            }
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x2269) {
            if (extraout_ECX[0xa269] != 0) {
              (**(code **)(*(int *)extraout_ECX[0xa269] + 0x60))();
              FUN_00470e36();
            }
            ExceptionList = local_10;
            return 1;
          }
          if ((0x226a < param_1) && (param_1 < 0x2270)) {
            FUN_00471267();
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x2304) {
            FUN_00472651();
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x2306) {
            FUN_00472966();
            ExceptionList = local_10;
            return 1;
          }
          if (param_1 == 0x2312) {
            FUN_004727cc();
            ExceptionList = local_10;
            return 1;
          }
        }
      }
      uVar9 = 0;
    }
  }
  else {
    local_194c = (param_1 - 0x352) + *(int *)(extraout_ECX[0x9f5a] + 0xa0);
    FUN_004729d4();
    uVar9 = 1;
  }
  ExceptionList = local_10;
  return uVar9;
}

