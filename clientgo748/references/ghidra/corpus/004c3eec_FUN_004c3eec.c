// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c3eec | Name: FUN_004c3eec


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004c3eec(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  float10 fVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined1 *puVar8;
  undefined1 *puVar9;
  int *piVar10;
  undefined1 auStack_2a8 [64];
  undefined1 auStack_268 [64];
  undefined1 auStack_228 [64];
  undefined1 auStack_1e8 [64];
  undefined1 auStack_1a8 [64];
  float fStack_168;
  float fStack_164;
  float fStack_160;
  undefined4 uStack_15c;
  undefined4 uStack_158;
  undefined4 uStack_154;
  uint uStack_150;
  undefined1 auStack_14c [64];
  undefined4 uStack_10c;
  undefined4 uStack_108;
  undefined4 uStack_104;
  float fStack_100;
  undefined4 auStack_fc [17];
  undefined1 local_b8 [64];
  undefined1 local_78 [64];
  undefined1 local_38 [12];
  uint local_2c;
  int *local_28;
  uint local_24;
  uint local_20;
  int local_1c;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  char local_8;
  
  if (*(int *)(param_1 + 0xc0) == 0) {
    uVar1 = 0;
  }
  else {
    local_c = 0;
    if (((((**(int **)(param_1 + 0xc0) == 0x19) || (**(int **)(param_1 + 0xc0) == 0x1c)) ||
         (**(int **)(param_1 + 0xc0) == 0x1d)) ||
        ((**(int **)(param_1 + 0xc0) == 7 || (**(int **)(param_1 + 0xc0) == 0x18)))) ||
       (**(int **)(param_1 + 0xc0) == 0x14)) {
      local_c = FUN_004b9a4b(*(undefined4 *)(param_1 + 200),3600000);
    }
    else {
      local_c = FUN_004b9a4b(*(undefined4 *)(param_1 + 200),360000);
    }
    local_8 = *(char *)(DAT_0092e640 + 0x43427 + *(int *)(param_1 + 200) * 0x108);
    if (local_8 == 'C') {
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    }
    (**(code **)(*DAT_005ccf9c + 0x38))(0,local_c);
    if (param_2 == 0) {
      FUN_004c3a1b(local_8);
    }
    if (((*(short *)(param_1 + 0xc4) < 1) || (*(char *)(*(int *)(param_1 + 0xc0) + 0x306) != '\x01')
        ) && ((*(short *)(param_1 + 0xc6) < 4 || (8 < *(short *)(param_1 + 0xc6))))) {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
      if (*(short *)(param_1 + 0xc6) == 4) {
        local_10 = 0xb3;
        uVar1 = FUN_004b925c(*(short *)(param_1 + 0xc4) + 0xb3,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
        if (((local_8 != 'C') || (DAT_005ccf9c[0xa8e4] != 0)) ||
           ((DAT_005ccf9c[0xa8e8] != 0 || (DAT_005ccf9c[0xa8e6] != 0)))) {
          (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
        }
        if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
           (DAT_005ccf9c[0xa8e6] == 0)) {
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,5);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
        }
      }
      else if ((*(short *)(param_1 + 0xc6) < 5) || (8 < *(short *)(param_1 + 0xc6))) {
        if ((*(short *)(param_1 + 0xc6) < 9) || (0xc < *(short *)(param_1 + 0xc6))) {
          if (((*(short *)(param_1 + 0xc6) < 0x74) || (0x7d < *(short *)(param_1 + 0xc6))) ||
             (*(short *)(param_1 + 0xc4) < 1)) {
            if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
               (DAT_005ccf9c[0xa8e6] == 0)) {
              if (*(short *)(param_1 + 0xc4) < 0xd) {
                uVar1 = FUN_004b925c(*(int *)(s_Wrong_Match_Time_Clock___l____l__005ba9e4 +
                                             *(short *)(param_1 + 0xc4) * 4 + 0x20) + 0xe9,360000);
                (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
              }
              else {
                uVar1 = FUN_004b925c(0x79,360000);
                (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
              }
              if (local_8 == 'C') {
                (**(code **)(*DAT_005ccf9c + 0x34))(1,2,1);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,3,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0x12);
              }
              else {
                (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
              }
              if (*(short *)(param_1 + 0xc4) < 7) {
                (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
                if (local_8 != 'C') {
                  (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
                }
              }
              else {
                (**(code **)(*DAT_005ccf9c + 0x34))(0,1,5);
                if (local_8 != 'C') {
                  (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
                }
              }
            }
            else {
              uVar1 = FUN_004b925c(*(short *)(param_1 + 0xc4) + 0xe9,360000);
              (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
              (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
            }
          }
          else {
            local_1c = 0x99;
            if (*(short *)(param_1 + 0xc6) == 0x74) {
              local_1c = 0x113;
            }
            if (*(short *)(param_1 + 0xc6) == 0x75) {
              local_1c = 0x120;
            }
            if (*(short *)(param_1 + 0xc6) == 0x76) {
              local_1c = 0x12d;
            }
            if (*(short *)(param_1 + 0xc6) == 0x77) {
              local_1c = 0x13a;
            }
            if (*(short *)(param_1 + 0xc6) == 0x78) {
              local_1c = 0x147;
            }
            if (*(short *)(param_1 + 0xc6) == 0x79) {
              local_1c = 0x154;
            }
            if (*(short *)(param_1 + 0xc6) == 0x7a) {
              local_1c = 0x161;
            }
            if (*(short *)(param_1 + 0xc6) == 0x7b) {
              local_1c = 0x16e;
            }
            if (*(short *)(param_1 + 0xc6) == 0x7c) {
              local_1c = 0x1a9;
            }
            if (*(short *)(param_1 + 0xc6) == 0x7d) {
              local_1c = 0x188;
            }
            uVar1 = FUN_004b925c(local_1c + -1 + (int)*(short *)(param_1 + 0xc4),360000);
            (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
            (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
            if (((local_8 != 'C') || (DAT_005ccf9c[0xa8e4] != 0)) ||
               ((DAT_005ccf9c[0xa8e8] != 0 || (DAT_005ccf9c[0xa8e6] != 0)))) {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
              (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
              (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
            }
            if ((((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
                (DAT_005ccf9c[0xa8e6] == 0)) && (DAT_005ccf9c[0xa972] == 0)) {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,3,1);
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,0x18);
              if (*(short *)(param_1 + 0xc6) == 0x78) {
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
              }
              else {
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,6);
              }
            }
            else {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
              (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
            }
          }
        }
        else {
          local_18 = 0x99;
          if (*(short *)(param_1 + 0xc6) == 10) {
            local_18 = 0xa6;
          }
          if (*(short *)(param_1 + 0xc6) == 0xb) {
            local_18 = 0xf6;
          }
          if (*(short *)(param_1 + 0xc6) == 0xc) {
            local_18 = 0x104;
          }
          uVar1 = FUN_004b925c(local_18 + *(short *)(param_1 + 0xc4),360000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          if (((local_8 == 'C') && (DAT_005ccf9c[0xa8e4] == 0)) &&
             ((DAT_005ccf9c[0xa8e8] == 0 && (DAT_005ccf9c[0xa8e6] == 0)))) {
            (**(code **)(*DAT_005ccf9c + 0x34))(1,2,1);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,3,2);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0x12);
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
          }
          if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
             (DAT_005ccf9c[0xa8e6] == 0)) {
            if (*(short *)(param_1 + 0xc4) < 7) {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
              if (local_8 != 'C') {
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
              }
            }
            else {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,5);
              if (local_8 != 'C') {
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
              }
            }
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
          }
        }
      }
      else {
        local_14 = 0x99;
        if (*(short *)(param_1 + 0xc6) == 6) {
          local_14 = 0xa6;
        }
        if (*(short *)(param_1 + 0xc6) == 7) {
          local_14 = 0xf6;
        }
        if (*(short *)(param_1 + 0xc6) == 8) {
          local_14 = 0x104;
        }
        uVar1 = FUN_004b925c(local_14 + *(short *)(param_1 + 0xc4),360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
        if ((((local_8 == 'C') && (DAT_005ccf9c[0xa8e4] == 0)) && (DAT_005ccf9c[0xa8e8] == 0)) &&
           (DAT_005ccf9c[0xa8e6] == 0)) {
          (**(code **)(*DAT_005ccf9c + 0x34))(1,2,1);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,3,2);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0x12);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
        }
        if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
           (DAT_005ccf9c[0xa8e6] == 0)) {
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          if (local_8 != 'C') {
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
          }
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
        }
      }
    }
    if ((*(int *)(*(int *)(param_1 + 0xc0) + 0x2f4) == 0) ||
       (((((((**(int **)(param_1 + 0xc0) != 0 && (**(int **)(param_1 + 0xc0) != 1)) &&
            ((**(int **)(param_1 + 0xc0) != 2 &&
             ((**(int **)(param_1 + 0xc0) != 4 && (**(int **)(param_1 + 0xc0) != 6)))))) &&
           (**(int **)(param_1 + 0xc0) != 0xb)) &&
          (((**(int **)(param_1 + 0xc0) != 9 && (**(int **)(param_1 + 0xc0) != 0xc)) &&
           (**(int **)(param_1 + 0xc0) != 10)))) && (**(int **)(param_1 + 0xc0) != 5)) ||
        ((*(int *)(param_1 + 8) != *(int *)(&DAT_005baa88 + **(int **)(param_1 + 0xc0) * 8) &&
         (*(int *)(param_1 + 8) != *(int *)(&DAT_005baa8c + **(int **)(param_1 + 0xc0) * 8))))))) {
      if (*(int *)(param_1 + 4) == 0) {
        if (0 < *(short *)(param_1 + 0xc4)) {
          if (*(char *)(*(int *)(param_1 + 0xc0) + 0x307) == '\x01') {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
          }
          (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        }
        uVar1 = 1;
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x8c,0);
        if (DAT_005ccf9c[0xaa24] != 0) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x99,1);
        }
        iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x14c))
                          (DAT_005ccf9c[0xa8f9],0,*(undefined4 *)(*(int *)(param_1 + 4) + 0x318),
                           *(undefined4 *)(*(int *)(param_1 + 4) + 0xc));
        if (iVar2 < 0) {
          uVar1 = 0;
        }
        else {
          iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x154))
                            (DAT_005ccf9c[0xa8f9],*(undefined4 *)(*(int *)(param_1 + 4) + 0x31c),0);
          if (iVar2 < 0) {
            uVar1 = 0;
          }
          else if ((*(uint *)(param_1 + 0xb8) < 5) && (*(int *)(param_1 + 0xb8) != 0)) {
            if (**(int **)(param_1 + 0xc0) == 0x3d) {
              iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))
                                (DAT_005ccf9c[0xa8f9],
                                 *(undefined4 *)(&DAT_005b48f8 + *(int *)(param_1 + 0xb8) * 4));
              if (iVar2 < 0) {
                return 0;
              }
            }
            else {
              iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))
                                (DAT_005ccf9c[0xa8f9],(&DAT_005b48e8)[*(int *)(param_1 + 0xb8)]);
              if (iVar2 < 0) {
                return 0;
              }
            }
            for (local_2c = 0; local_2c < *(uint *)(param_1 + 0xc); local_2c = local_2c + 1) {
              FUN_004c5b40();
              thunk_FUN_00561a5b(local_b8,*(int *)(param_1 + 0x10) + local_2c * 0x40,
                                 *(undefined4 *)(param_1 + 0x14 + local_2c * 4));
              thunk_FUN_00561d24(local_b8,local_b8,DAT_005ccf9c + 0xa98b);
              (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x13c))
                        (DAT_005ccf9c[0xa8f9],local_2c * 3 + 9,local_b8,3);
            }
            FUN_00430f50(0xbf800000,0x3f800000,0x3f800000);
            thunk_FUN_005611dd(local_38,local_38);
            (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x13c))(DAT_005ccf9c[0xa8f9],1,local_38,1);
            FUN_004c5b40();
            thunk_FUN_00561f6d(local_78,0,DAT_005ccf9c + 0xa98b);
            thunk_FUN_00561ca4(local_78,local_78);
            uVar1 = 0x5c;
            (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x13c))
                      (DAT_005ccf9c[0xa8f9],0x5c,local_78,4);
            if ((*(int *)(param_1 + 0xc0) != 0) &&
               (*(short *)(*(int *)(param_1 + 0xc0) + 0x354) == 1)) {
              uVar1 = FUN_013c6100(uVar1);
              return uVar1;
            }
            if ((*(int *)(param_1 + 0xc0) != 0) && (**(int **)(param_1 + 0xc0) == 0x55)) {
              (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
            }
            iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                              (DAT_005ccf9c[0xa8f9],4,0,
                               *(undefined4 *)(*(int *)(param_1 + 4) + 0x28),0,
                               *(undefined4 *)(param_1 + 0xcc));
            if (iVar2 < 0) {
              uVar1 = 0;
            }
            else {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,1);
              (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
              (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
              if ((((*(int *)(DAT_0067cf38 + 0x7c) == *(int *)(*(int *)(param_1 + 0xc0) + 0x2f4)) &&
                   (*(int *)(DAT_0067cf38 + 0x7c) != 0)) && (*(int *)(DAT_013b71e8 + 0x1e428) == 0))
                 && ((**(int **)(param_1 + 0xc0) != 0xb || (*(int *)(param_1 + 8) != 2)))) {
                FUN_00430f10();
                FUN_00430f10();
                FUN_00430f10();
                iVar2 = *(int *)(DAT_013b71e8 + 0x1b084);
                uStack_15c = *(undefined4 *)(iVar2 + 0x48);
                uStack_158 = *(undefined4 *)(iVar2 + 0x4c);
                uStack_154 = *(undefined4 *)(iVar2 + 0x50);
                CSimpleArray<>();
                uStack_10c = *(undefined4 *)(*(int *)(param_1 + 0x14) + 0x30);
                uStack_108 = *(undefined4 *)(*(int *)(param_1 + 0x14) + 0x34);
                uStack_104 = *(undefined4 *)(*(int *)(param_1 + 0x14) + 0x38);
                FUN_00493db0(&fStack_168,&uStack_15c);
                fVar5 = (float10)FUN_004b3aa0();
                fStack_100 = (float)(fVar5 * (float10)_DAT_005a46f8);
                thunk_FUN_005611dd(&fStack_168,&fStack_168);
                thunk_FUN_0056239d(auStack_14c,fStack_168 * _DAT_005a4214,fStack_164 * _DAT_005a4214
                                   ,fStack_160 * _DAT_005a4214);
                if ((**(int **)(param_1 + 0xc0) == 0x29) &&
                   (*(short *)(*(int *)(param_1 + 0xc0) + 0x248) == 0)) {
                  thunk_FUN_0056230d(auStack_1a8,_DAT_005a46f4 + fStack_100,
                                     _DAT_005a46f4 + fStack_100,_DAT_005a46f4 + fStack_100);
                }
                else {
                  thunk_FUN_0056230d(auStack_1a8,_DAT_005a3660 + fStack_100,
                                     _DAT_005a3660 + fStack_100,_DAT_005a3660 + fStack_100);
                }
                for (uStack_150 = 0; uStack_150 < *(uint *)(param_1 + 0xc);
                    uStack_150 = uStack_150 + 1) {
                  piVar10 = DAT_005ccf9c + 0xa98b;
                  puVar9 = auStack_2a8;
                  puVar8 = auStack_14c;
                  puVar7 = auStack_268;
                  uVar1 = *(undefined4 *)(param_1 + 0x14 + uStack_150 * 4);
                  puVar6 = auStack_228;
                  FUN_004c5b90(auStack_1e8,auStack_1a8);
                  FUN_004c5b90(puVar6,uVar1);
                  FUN_004c5b90(puVar7,puVar8);
                  puVar3 = (undefined4 *)FUN_004c5b90(puVar9,piVar10);
                  puVar4 = auStack_fc;
                  for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
                    *puVar4 = *puVar3;
                    puVar3 = puVar3 + 1;
                    puVar4 = puVar4 + 1;
                  }
                  thunk_FUN_00561ca4(auStack_fc,auStack_fc);
                  (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x13c))
                            (DAT_005ccf9c[0xa8f9],uStack_150 * 3 + 9,auStack_fc,3);
                }
                if (*(int *)(DAT_0067cf38 + 0x7c) == *(int *)(*(int *)(param_1 + 0xc0) + 0x2f4)) {
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x288) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x28c) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x290) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2a8) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2ac) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2b0) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2b8) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 700) = 0x3f800000;
                  *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2c0) = 0x3f800000;
                }
                (**(code **)(*DAT_005ccf9c + 0x30))
                          (0x3c,*(undefined4 *)(*(int *)(DAT_0067cf38 + 0x7c) + 0x5e4));
                uVar1 = FUN_004b9a4b(*(undefined4 *)(param_1 + 200),180000);
                (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
                (**(code **)(*DAT_005ccf9c + 0x30))(9,1);
                (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
                (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
                (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
                (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,1,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,2,3);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
                iVar2 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                                  (DAT_005ccf9c[0xa8f9],4,0,
                                   *(undefined4 *)(*(int *)(param_1 + 4) + 0x28),0,
                                   *(undefined4 *)(param_1 + 0xcc));
                if (iVar2 < 0) {
                  return 0;
                }
                (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
                (**(code **)(*DAT_005ccf9c + 0x30))(9,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,2,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
                (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,1);
                (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
              }
              if (DAT_005ccf9c[0xaa24] != 0) {
                (**(code **)(*DAT_005ccf9c + 0x30))(0x99,0);
              }
              if (0 < *(short *)(param_1 + 0xc4)) {
                if (*(char *)(*(int *)(param_1 + 0xc0) + 0x307) == '\x01') {
                  (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
                }
                (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
                (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
                (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
                (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
              }
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
              (**(code **)(*DAT_005ccf9c + 0x34))(0,3,1);
              uVar1 = 1;
            }
          }
          else {
            uVar1 = 0;
          }
        }
      }
    }
    else {
      local_20 = 0;
      if (*(int *)(param_1 + 8) == *(int *)(&DAT_005baa88 + **(int **)(param_1 + 0xc0) * 8)) {
        if (*(short *)(*(int *)(param_1 + 0xc0) + 0x260) == 0) {
          local_20 = 0xffffffff;
        }
        else {
          local_20 = (uint)*(ushort *)(*(int *)(param_1 + 0xc0) + 0x260);
        }
      }
      else if (*(int *)(param_1 + 8) == *(int *)(&DAT_005baa8c + **(int **)(param_1 + 0xc0) * 8)) {
        if (*(short *)(*(int *)(param_1 + 0xc0) + 0x264) == 0) {
          local_20 = 0xffffffff;
        }
        else {
          local_20 = (uint)*(ushort *)(*(int *)(param_1 + 0xc0) + 0x264);
        }
      }
      if (((0xc < (int)local_20) && ((int)local_20 < 1000)) ||
         ((0xa8c < (int)local_20 && ((int)local_20 < 0xbb9)))) {
        local_24 = (uint)(0 < *(short *)(param_1 + 0xc4));
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,2);
        local_28 = (int *)FUN_004b14fd(local_20,1200000,180000);
        if (local_28 != (int *)0x0) {
          (**(code **)(*local_28 + 0xc))(local_24 & 0xff,0);
        }
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      }
      if (0 < *(short *)(param_1 + 0xc4)) {
        (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      }
      uVar1 = 1;
    }
  }
  return uVar1;
}

