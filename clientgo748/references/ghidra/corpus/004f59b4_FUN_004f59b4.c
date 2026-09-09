// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f59b4 | Name: FUN_004f59b4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004f59b4(int *param_1)

{
  int iVar1;
  undefined4 auStack_bac [686];
  undefined4 uStack_f4;
  undefined4 uStack_f0;
  undefined4 uStack_ec;
  undefined4 uStack_e8;
  undefined4 uStack_e4;
  undefined4 uStack_e0;
  int iStack_dc;
  float fStack_d8;
  int iStack_d4;
  int iStack_d0;
  undefined4 uStack_cc;
  undefined4 uStack_c8;
  int iStack_c4;
  float fStack_c0;
  int iStack_bc;
  int iStack_b8;
  float fStack_b4;
  int iStack_b0;
  int *piStack_ac;
  float fStack_a8;
  int iStack_a4;
  undefined1 *puStack_a0;
  float fStack_9c;
  int iStack_98;
  float fStack_94;
  int iStack_90;
  int iStack_8c;
  float fStack_88;
  int iStack_84;
  int *piStack_80;
  float fStack_7c;
  int iStack_78;
  int *piStack_74;
  int *local_70;
  int local_6c;
  int local_68;
  int local_64;
  undefined4 local_60;
  float local_5c;
  float local_58;
  float local_54;
  uint local_50;
  int local_4c;
  undefined1 local_48 [16];
  uint local_38;
  int local_30;
  int *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int *local_1c;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_1[9] == -1) {
    return 1;
  }
  if ((short)param_1[0x58] == 0) {
    return 1;
  }
  local_14 = *(int *)(DAT_013b71e8 + 0x1b084);
  local_10 = *(undefined4 *)(local_14 + 0x48);
  local_c = *(undefined4 *)(local_14 + 0x4c);
  local_8 = *(undefined4 *)(local_14 + 0x50);
  piStack_74 = (int *)0x4f5a18;
  local_70 = param_1;
  iVar1 = (**(code **)(*param_1 + 0x4c))();
  if (iVar1 != 1) {
    return 1;
  }
  local_18 = (int)*(short *)(&DAT_00d44990 + (short)local_70[0x58] * 0x8c);
  if ((0x2e0 < local_18) && (local_18 < 0x2e4)) {
    local_28 = 0xffff4400;
    local_24 = 0xff3366ff;
    local_20 = 0xff00ff00;
    piStack_74 = (int *)0x0;
    iStack_78 = 0x89;
    fStack_7c = 7.287471e-39;
    (**(code **)(*DAT_005ccf9c + 0x30))();
    fStack_7c = 2.8026e-45;
    piStack_80 = (int *)0x14;
    iStack_84 = 0x4f5a95;
    (**(code **)(*DAT_005ccf9c + 0x30))();
    iStack_84 = 5;
    fStack_88 = 2.66247e-44;
    iStack_8c = 0x4f5aa9;
    (**(code **)(*DAT_005ccf9c + 0x30))();
    iStack_8c = 2;
    iStack_90 = 4;
    fStack_94 = 0.0;
    iStack_98 = 0x4f5abf;
    (**(code **)(*DAT_005ccf9c + 0x34))();
    iStack_98 = 1;
    fStack_9c = 3.78351e-44;
    puStack_a0 = (undefined1 *)0x4f5ad3;
    (**(code **)(*DAT_005ccf9c + 0x30))();
    puStack_a0 = (undefined1 *)0x2bf20;
    iStack_a4 = 1;
    fStack_a8 = (float)local_18;
    piStack_ac = (int *)0x4f5ae9;
    local_2c = (int *)FUN_004b14fd();
    if (local_2c != (int *)0x0) {
      puStack_a0 = local_48;
      iStack_a4 = local_2c[0xc6];
      fStack_a8 = 7.287674e-39;
      (**(code **)(*(int *)local_2c[0xc6] + 0x34))();
      fStack_a8 = 0.0;
      piStack_ac = &local_30;
      iStack_b0 = 0;
      fStack_b4 = 0.0;
      iStack_b8 = local_2c[0xc6];
      iStack_bc = 0x4f5b34;
      (**(code **)(*(int *)local_2c[0xc6] + 0x2c))();
      local_50 = local_38 / 0x18;
      for (local_4c = 0; local_4c < (int)(local_38 / 0x18); local_4c = local_4c + 1) {
        *(undefined4 *)(local_30 + 0xc + local_4c * 0x18) = auStack_bac[local_18];
      }
      iStack_bc = local_2c[0xc6];
      fStack_c0 = 7.287848e-39;
      (**(code **)(*(int *)local_2c[0xc6] + 0x30))();
      local_2c[0xa6] = 0xcc;
      fStack_c0 = 0.0;
      iStack_c4 = 0;
      uStack_c8 = 0;
      uStack_cc = 0;
      iStack_d0 = local_70[0xd];
      iStack_d4 = local_70[0xb];
      fStack_d8 = (float)local_70[0xc] + _DAT_005a3534;
      iStack_dc = local_70[10];
      uStack_e0 = 0x4f5bd3;
      (**(code **)(*local_2c + 8))();
    }
    puStack_a0 = (undefined1 *)0x1;
    iStack_a4 = 0x89;
    fStack_a8 = 7.287977e-39;
    (**(code **)(*DAT_005ccf9c + 0x30))();
    return 1;
  }
  if ((0 < (short)local_70[0x6f]) && (DAT_005ccf9c[0xa8e4] == 0)) {
    local_70[0x5a] = (int)((float)local_70[0x5a] / _DAT_005a365c);
    local_70[0x5b] = (int)((float)local_70[0x5b] / _DAT_005a46e8);
    local_70[0x5c] = (int)((float)local_70[0x5c] / _DAT_005a4524);
    local_70[0x62] = local_70[0x5a];
    local_70[99] = local_70[0x5b];
    local_70[100] = local_70[0x5c];
    local_70[0x65] = local_70[0x5d];
  }
  piStack_74 = local_70 + 0x5a;
  iStack_78 = DAT_005ccf9c[0xa8f9];
  fStack_7c = 7.288247e-39;
  (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))();
  fStack_7c = 2.52234e-40;
  piStack_80 = (int *)0x0;
  iStack_84 = local_70[9];
  fStack_88 = 7.288282e-39;
  local_1c = (int *)FUN_004b14fd();
  if (local_1c == (int *)0x0) {
    return 1;
  }
  local_58 = 0.0;
  local_54 = 1.5707964;
  if ((local_70[0x6e] == 4) || (local_70[0x6e] == 5)) {
    local_54 = 0.0;
  }
  fStack_7c = 5.32493e-44;
  piStack_80 = local_70 + 0x58;
  iStack_84 = 0x4f5d0e;
  iVar1 = FUN_0054cd07();
  if (iVar1 != 2) {
    fStack_7c = 5.32493e-44;
    piStack_80 = local_70 + 0x58;
    iStack_84 = 0x4f5d27;
    iVar1 = FUN_0054cd07();
    if (iVar1 != 1) goto LAB_004f5d36;
  }
  local_54 = 0.0;
LAB_004f5d36:
  if (*(short *)(&DAT_00d44990 + (short)local_70[0x58] * 0x8c) == 0x2d) {
    local_54 = 0.0;
  }
  else if (((short)local_70[0x58] == 0x1a3) || ((short)local_70[0x58] == 0x1a4)) {
    local_54 = 0.0;
  }
  else if ((short)local_70[0x58] == 0x2eb) {
    local_54 = 0.0;
  }
  else if (((short)local_70[0x58] < 0x6a5) || (0x6b0 < (short)local_70[0x58])) {
    if ((short)local_70[0x58] == 0x6bf) {
      local_54 = 0.0;
    }
    else if (((short)local_70[0x58] < 0x6c5) || (0x6c8 < (short)local_70[0x58])) {
      if (((short)local_70[0x58] == 0x6d8) ||
         ((0x6df < (short)local_70[0x58] && ((short)local_70[0x58] < 0x6e4)))) {
        local_54 = 0.0;
        local_58 = 0.1;
      }
      else if (((short)local_70[0x58] < 0x6d0) || (0x6d7 < (short)local_70[0x58])) {
        if (((short)local_70[0x58] < 0x8fc) || (0x919 < (short)local_70[0x58])) {
          if (((short)local_70[0x58] < 0x91a) || (0x937 < (short)local_70[0x58])) {
            if (((short)local_70[0x58] < 0x938) || (0x955 < (short)local_70[0x58])) {
              if (((short)local_70[0x58] < 0x956) || (0x991 < (short)local_70[0x58])) {
                if (((short)local_70[0x58] < 0xb90) || (2999 < (short)local_70[0x58])) {
                  if (((short)local_70[0x58] < 0x13f6) || (0x140d < (short)local_70[0x58])) {
                    if (((short)local_70[0x58] == 0xd73) ||
                       (((short)local_70[0x58] == 0xd74 || ((short)local_70[0x58] == 0x140f)))) {
                      local_54 = 0.0;
                      local_58 = -0.13;
                    }
                    else if (((short)local_70[0x58] < 0xc7d) || (0xc7f < (short)local_70[0x58])) {
                      if (((short)local_70[0x58] < 0xdac) || (0xdae < (short)local_70[0x58])) {
                        if (((short)local_70[0x58] < 0xc44) || (0xdde < (short)local_70[0x58])) {
                          if (((short)local_70[0x58] < 0xc49) || (0xc4d < (short)local_70[0x58])) {
                            if (((short)local_70[0x58] < 0xf8c) || (3999 < (short)local_70[0x58])) {
                              if ((short)local_70[0x58] == 0xfad) {
                                local_54 = -1.5707964;
                              }
                              else if (((short)local_70[0x58] < 0x100a) ||
                                      (0x100d < (short)local_70[0x58])) {
                                if ((short)local_70[0x58] == 0x14da) {
                                  local_54 = 0.0;
                                  local_58 = -0.15;
                                }
                                else if ((short)local_70[0x58] == 0x140e) {
                                  local_54 = 0.0;
                                  local_58 = -0.1;
                                }
                                else if (((short)local_70[0x58] == 0xd77) ||
                                        ((short)local_70[0x58] == 0xd78)) {
                                  local_58 = -0.1;
                                }
                                else if ((short)local_70[0x58] == 0x1411) {
                                  local_54 = -1.5707964;
                                }
                              }
                              else {
                                local_54 = 0.0;
                              }
                            }
                            else {
                              local_54 = 0.0;
                            }
                          }
                          else {
                            local_54 = 0.0;
                          }
                        }
                        else {
                          local_54 = 0.0;
                        }
                      }
                      else {
                        local_54 = 0.0;
                      }
                    }
                    else {
                      local_54 = -1.5707964;
                    }
                  }
                  else {
                    local_54 = -1.5707964;
                  }
                }
                else {
                  local_54 = 0.0;
                }
              }
              else {
                local_54 = 0.0;
              }
            }
            else {
              local_54 = 0.0;
            }
          }
          else {
            local_54 = 0.0;
          }
        }
        else {
          local_54 = 0.0;
          local_58 = 0.3;
        }
      }
      else {
        local_54 = 0.0;
        local_58 = 0.3;
      }
    }
    else {
      local_54 = 0.0;
    }
  }
  else {
    local_54 = -1.5707964;
  }
  if ((local_70[9] == 0x195) || (local_70[9] == 0x647)) {
    local_54 = 0.0;
  }
  local_5c = (float)(int)*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c);
  if (local_18 == 0x1b) {
    if ((*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) < 6) ||
       (9 < *(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c))) {
      if ((*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) < 0xb) ||
         (0x11 < *(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c))) {
        if (*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) == 0x13) {
          local_5c = (float)(*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) + -3);
        }
        else if ((*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) == 0x15) ||
                (*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) == 0x16)) {
          local_5c = (float)(*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) + -4);
        }
      }
      else {
        local_5c = (float)(*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) + -2);
      }
    }
    else {
      local_5c = (float)(*(short *)(&DAT_00d44992 + (short)local_70[0x58] * 0x8c) + -1);
    }
  }
  if (((short)local_70[0x6f] < 1) &&
     ((*(short *)((int)local_70 + 0x1be) < 4 || (8 < *(short *)((int)local_70 + 0x1be))))) {
    if ((short)local_70[0x58] == 0xa9) {
      fStack_7c = -NAN;
      piStack_80 = (int *)0x0;
      iStack_84 = 0;
      fStack_88 = local_54;
      iStack_8c = local_70[0xd];
      iStack_90 = local_70[0xb];
      fStack_94 = local_58 + (float)local_70[0xc];
      iStack_98 = local_70[10];
      fStack_9c = 7.293963e-39;
      (**(code **)(*local_1c + 8))();
    }
    else {
      fStack_7c = local_5c;
      piStack_80 = (int *)0x0;
      iStack_84 = 0;
      fStack_88 = local_54;
      iStack_8c = local_70[0xd];
      iStack_90 = local_70[0xb];
      fStack_94 = local_58 + (float)local_70[0xc];
      iStack_98 = local_70[10];
      fStack_9c = 7.294046e-39;
      (**(code **)(*local_1c + 8))();
    }
  }
  else {
    if (*(short *)((int)local_70 + 0x1be) == 4) {
      local_60 = 0xb3;
      fStack_7c = 5.04467e-40;
      piStack_80 = (int *)((short)local_70[0x6f] + 0xb3);
      iStack_84 = 0x4f63c7;
      fStack_7c = (float)FUN_004b925c();
      piStack_80 = (int *)0x1;
      iStack_84 = 0x4f63db;
      (**(code **)(*DAT_005ccf9c + 0x38))();
      iStack_84 = 1;
      fStack_88 = 1.54143e-44;
      iStack_8c = 1;
      iStack_90 = 0x4f63f2;
      (**(code **)(*DAT_005ccf9c + 0x34))();
      if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
         (DAT_005ccf9c[0xa8e6] == 0)) {
        iStack_90 = 5;
        fStack_94 = 1.4013e-45;
        iStack_98 = 0;
        fStack_9c = 7.291015e-39;
        (**(code **)(*DAT_005ccf9c + 0x34))();
        fStack_9c = 5.60519e-45;
        puStack_a0 = (undefined1 *)0x1;
        iStack_a4 = 1;
        fStack_a8 = 7.291046e-39;
        (**(code **)(*DAT_005ccf9c + 0x34))();
      }
      else {
        iStack_90 = 4;
        fStack_94 = 1.4013e-45;
        iStack_98 = 0;
        fStack_9c = 7.29095e-39;
        (**(code **)(*DAT_005ccf9c + 0x34))();
        fStack_9c = 5.60519e-45;
        puStack_a0 = (undefined1 *)0x1;
        iStack_a4 = 1;
        fStack_a8 = 7.290981e-39;
        (**(code **)(*DAT_005ccf9c + 0x34))();
      }
      fStack_a8 = local_5c;
      piStack_ac = (int *)0x1;
      iStack_b0 = 0;
      fStack_b4 = local_54;
      iStack_b8 = local_70[0xd];
      iStack_bc = local_70[0xb];
      fStack_c0 = local_58 + (float)local_70[0xc];
      iStack_c4 = local_70[10];
      uStack_c8 = 0x4f64b1;
      (**(code **)(*local_1c + 8))();
    }
    else if ((*(short *)((int)local_70 + 0x1be) < 5) || (8 < *(short *)((int)local_70 + 0x1be))) {
      if ((*(short *)((int)local_70 + 0x1be) < 9) || (0xc < *(short *)((int)local_70 + 0x1be))) {
        if (((*(short *)((int)local_70 + 0x1be) < 0x74) ||
            (0x7d < *(short *)((int)local_70 + 0x1be))) || ((short)local_70[0x6f] < 1)) {
          fStack_7c = 5.04467e-40;
          piStack_80 = (int *)((short)local_70[0x6f] + 0xe9);
          iStack_84 = 0x4f6acd;
          fStack_7c = (float)FUN_004b925c();
          piStack_80 = (int *)0x1;
          iStack_84 = 0x4f6ae1;
          (**(code **)(*DAT_005ccf9c + 0x38))();
          iStack_84 = 1;
          fStack_88 = 1.54143e-44;
          iStack_8c = 1;
          iStack_90 = 0x4f6af8;
          (**(code **)(*DAT_005ccf9c + 0x34))();
          if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
             (DAT_005ccf9c[0xa8e6] == 0)) {
            if ((short)local_70[0x6f] < 7) {
              iStack_90 = 4;
              fStack_94 = 1.4013e-45;
              iStack_98 = 0;
              fStack_9c = 7.293623e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
              fStack_9c = 1.54143e-44;
              puStack_a0 = (undefined1 *)0x1;
              iStack_a4 = 1;
              fStack_a8 = 7.293653e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
            }
            else {
              iStack_90 = 5;
              fStack_94 = 1.4013e-45;
              iStack_98 = 0;
              fStack_9c = 7.293557e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
              fStack_9c = 1.54143e-44;
              puStack_a0 = (undefined1 *)0x1;
              iStack_a4 = 1;
              fStack_a8 = 7.293589e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
            }
          }
          else {
            iStack_90 = 4;
            fStack_94 = 1.4013e-45;
            iStack_98 = 0;
            fStack_9c = 7.29347e-39;
            (**(code **)(*DAT_005ccf9c + 0x34))();
            fStack_9c = 9.80909e-45;
            puStack_a0 = (undefined1 *)0x1;
            iStack_a4 = 1;
            fStack_a8 = 7.2935e-39;
            (**(code **)(*DAT_005ccf9c + 0x34))();
          }
          fStack_a8 = local_5c;
          piStack_ac = (int *)0x1;
          iStack_b0 = 0;
          fStack_b4 = local_54;
          iStack_b8 = local_70[0xd];
          iStack_bc = local_70[0xb];
          fStack_c0 = local_58 + (float)local_70[0xc];
          iStack_c4 = local_70[10];
          uStack_c8 = 0x4f6bf6;
          (**(code **)(*local_1c + 8))();
        }
        else {
          local_6c = 0x113;
          if (*(short *)((int)local_70 + 0x1be) == 0x75) {
            local_6c = 0x120;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x76) {
            local_6c = 0x12d;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x77) {
            local_6c = 0x13a;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x78) {
            local_6c = 0x147;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x79) {
            local_6c = 0x154;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x7a) {
            local_6c = 0x161;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x7b) {
            local_6c = 0x16e;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x7c) {
            local_6c = 0x1a9;
          }
          if (*(short *)((int)local_70 + 0x1be) == 0x7d) {
            local_6c = 0x188;
          }
          if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
             ((DAT_005ccf9c[0xa8e6] == 0 && (DAT_005ccf9c[0xa972] == 0)))) {
            fStack_7c = 5.04467e-40;
            piStack_80 = (int *)(local_6c + -1 + (int)(short)local_70[0x6f]);
            iStack_84 = 0x4f69c9;
            fStack_7c = (float)FUN_004b925c();
            piStack_80 = (int *)0x1;
            iStack_84 = 0x4f69dd;
            (**(code **)(*DAT_005ccf9c + 0x38))();
            iStack_84 = 1;
            fStack_88 = 4.2039e-45;
            iStack_8c = 0;
            iStack_90 = 0x4f69f3;
            (**(code **)(*DAT_005ccf9c + 0x34))();
            iStack_90 = 0x18;
            fStack_94 = 1.4013e-45;
            iStack_98 = 0;
            fStack_9c = 7.293042e-39;
            (**(code **)(*DAT_005ccf9c + 0x34))();
            if (*(short *)((int)local_70 + 0x1be) == 0x78) {
              fStack_9c = 5.60519e-45;
              puStack_a0 = (undefined1 *)0x1;
              iStack_a4 = 1;
              fStack_a8 = 7.293096e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
            }
            else if (*(short *)((int)local_70 + 0x1be) == 0x77) {
              fStack_9c = 1.12104e-44;
              puStack_a0 = (undefined1 *)0x1;
              iStack_a4 = 1;
              fStack_a8 = 7.29315e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
            }
            else {
              fStack_9c = 8.40779e-45;
              puStack_a0 = (undefined1 *)0x1;
              iStack_a4 = 1;
              fStack_a8 = 7.293184e-39;
              (**(code **)(*DAT_005ccf9c + 0x34))();
            }
          }
          else {
            fStack_7c = 5.04467e-40;
            piStack_80 = (int *)(local_6c + -1 + (int)(short)local_70[0x6f]);
            iStack_84 = 0x4f6962;
            fStack_7c = (float)FUN_004b925c();
            piStack_80 = (int *)0x1;
            iStack_84 = 0x4f6976;
            (**(code **)(*DAT_005ccf9c + 0x38))();
            iStack_84 = 4;
            fStack_88 = 1.4013e-45;
            iStack_8c = 0;
            iStack_90 = 0x4f698c;
            (**(code **)(*DAT_005ccf9c + 0x34))();
            iStack_90 = 7;
            fStack_94 = 1.4013e-45;
            iStack_98 = 1;
            fStack_9c = 7.292898e-39;
            (**(code **)(*DAT_005ccf9c + 0x34))();
          }
          fStack_9c = local_5c;
          puStack_a0 = (undefined1 *)0x1;
          iStack_a4 = 0;
          fStack_a8 = local_54;
          piStack_ac = (int *)local_70[0xd];
          iStack_b0 = local_70[0xb];
          fStack_b4 = local_58 + (float)local_70[0xc];
          iStack_b8 = local_70[10];
          iStack_bc = 0x4f6aa7;
          (**(code **)(*local_1c + 8))();
        }
      }
      else {
        local_68 = 0x99;
        if (*(short *)((int)local_70 + 0x1be) == 10) {
          local_68 = 0xa6;
        }
        if (*(short *)((int)local_70 + 0x1be) == 0xb) {
          local_68 = 0xf6;
        }
        if (*(short *)((int)local_70 + 0x1be) == 0xc) {
          local_68 = 0x104;
        }
        if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
           (DAT_005ccf9c[0xa8e6] == 0)) {
          fStack_7c = 5.04467e-40;
          piStack_80 = (int *)(local_68 + (short)local_70[0x6f]);
          iStack_84 = 0x4f676d;
          fStack_7c = (float)FUN_004b925c();
          piStack_80 = (int *)0x1;
          iStack_84 = 0x4f6780;
          (**(code **)(*DAT_005ccf9c + 0x38))();
          iStack_84 = 5;
          fStack_88 = 1.4013e-45;
          iStack_8c = 0;
          iStack_90 = 0x4f6796;
          (**(code **)(*DAT_005ccf9c + 0x34))();
          iStack_90 = 0xb;
          fStack_94 = 1.4013e-45;
          iStack_98 = 1;
          fStack_9c = 7.292195e-39;
          (**(code **)(*DAT_005ccf9c + 0x34))();
        }
        else {
          fStack_7c = 5.04467e-40;
          piStack_80 = (int *)(local_68 + (short)local_70[0x6f]);
          iStack_84 = 0x4f670b;
          fStack_7c = (float)FUN_004b925c();
          piStack_80 = (int *)0x1;
          iStack_84 = 0x4f671f;
          (**(code **)(*DAT_005ccf9c + 0x38))();
          iStack_84 = 4;
          fStack_88 = 1.4013e-45;
          iStack_8c = 0;
          iStack_90 = 0x4f6735;
          (**(code **)(*DAT_005ccf9c + 0x34))();
          iStack_90 = 7;
          fStack_94 = 1.4013e-45;
          iStack_98 = 1;
          fStack_9c = 7.292059e-39;
          (**(code **)(*DAT_005ccf9c + 0x34))();
        }
        fStack_9c = local_5c;
        puStack_a0 = (undefined1 *)0x1;
        iStack_a4 = 0;
        fStack_a8 = local_54;
        piStack_ac = (int *)local_70[0xd];
        iStack_b0 = local_70[0xb];
        fStack_b4 = local_58 + (float)local_70[0xc];
        iStack_b8 = local_70[10];
        iStack_bc = 0x4f67e5;
        (**(code **)(*local_1c + 8))();
      }
    }
    else {
      local_64 = 0x99;
      if (*(short *)((int)local_70 + 0x1be) == 6) {
        local_64 = 0xa6;
      }
      if (*(short *)((int)local_70 + 0x1be) == 7) {
        local_64 = 0xf6;
      }
      if (*(short *)((int)local_70 + 0x1be) == 8) {
        local_64 = 0x104;
      }
      if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
         (DAT_005ccf9c[0xa8e6] == 0)) {
        fStack_7c = 5.04467e-40;
        piStack_80 = (int *)(local_64 + (short)local_70[0x6f]);
        iStack_84 = 0x4f65d3;
        fStack_7c = (float)FUN_004b925c();
        piStack_80 = (int *)0x1;
        iStack_84 = 0x4f65e6;
        (**(code **)(*DAT_005ccf9c + 0x38))();
        iStack_84 = 4;
        fStack_88 = 1.4013e-45;
        iStack_8c = 0;
        iStack_90 = 0x4f65fc;
        (**(code **)(*DAT_005ccf9c + 0x34))();
        iStack_90 = 0xb;
        fStack_94 = 1.4013e-45;
        iStack_98 = 1;
        fStack_9c = 7.29162e-39;
        (**(code **)(*DAT_005ccf9c + 0x34))();
      }
      else {
        fStack_7c = 5.04467e-40;
        piStack_80 = (int *)(local_64 + (short)local_70[0x6f]);
        iStack_84 = 0x4f6571;
        fStack_7c = (float)FUN_004b925c();
        piStack_80 = (int *)0x1;
        iStack_84 = 0x4f6585;
        (**(code **)(*DAT_005ccf9c + 0x38))();
        iStack_84 = 4;
        fStack_88 = 1.4013e-45;
        iStack_8c = 0;
        iStack_90 = 0x4f659b;
        (**(code **)(*DAT_005ccf9c + 0x34))();
        iStack_90 = 7;
        fStack_94 = 1.4013e-45;
        iStack_98 = 1;
        fStack_9c = 7.291484e-39;
        (**(code **)(*DAT_005ccf9c + 0x34))();
      }
      fStack_9c = local_5c;
      puStack_a0 = (undefined1 *)0x1;
      iStack_a4 = 0;
      fStack_a8 = local_54;
      piStack_ac = (int *)local_70[0xd];
      iStack_b0 = local_70[0xb];
      fStack_b4 = local_58 + (float)local_70[0xc];
      iStack_b8 = local_70[10];
      iStack_bc = 0x4f664b;
      (**(code **)(*local_1c + 8))();
    }
    uStack_c8 = 0;
    uStack_cc = 1;
    iStack_d0 = 0x4f6c0a;
    (**(code **)(*DAT_005ccf9c + 0x38))();
    iStack_d0 = 1;
    iStack_d4 = 0xb;
    fStack_d8 = 1.4013e-45;
    iStack_dc = 0x4f6c20;
    (**(code **)(*DAT_005ccf9c + 0x34))();
    iStack_dc = 4;
    uStack_e0 = 1;
    uStack_e4 = 0;
    uStack_e8 = 0x4f6c36;
    (**(code **)(*DAT_005ccf9c + 0x34))();
    uStack_e8 = 1;
    uStack_ec = 1;
    uStack_f0 = 1;
    uStack_f4 = 0x4f6c4c;
    (**(code **)(*DAT_005ccf9c + 0x34))();
  }
  return 1;
}

