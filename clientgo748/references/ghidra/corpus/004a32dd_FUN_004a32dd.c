// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a32dd | Name: FUN_004a32dd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004a32dd(int *param_1,int param_2,int param_3)

{
  char cVar1;
  int iVar2;
  char *pcVar3;
  size_t sVar4;
  undefined4 uVar5;
  char *_Str1;
  undefined4 uVar6;
  undefined1 local_134 [4];
  undefined2 local_130;
  undefined2 local_12e;
  undefined4 local_128;
  int local_124;
  undefined1 local_120 [16];
  undefined1 local_110 [16];
  int local_100;
  undefined1 local_fc [4];
  undefined2 local_f8;
  undefined2 local_f6;
  int local_f0;
  undefined1 local_ec [16];
  undefined1 local_dc [12];
  int local_d0;
  size_t local_cc;
  int local_c8;
  int local_c4;
  undefined1 local_c0 [4];
  undefined2 local_bc;
  undefined2 local_ba;
  undefined4 local_b4;
  int local_b0;
  undefined1 local_ac [16];
  undefined1 local_9c [16];
  char *local_8c;
  int *local_88;
  int local_84;
  int local_80;
  int local_7c;
  int *local_78;
  int local_74;
  undefined1 local_70 [4];
  undefined2 local_6c;
  undefined2 local_6a;
  int local_64;
  undefined1 local_60 [16];
  int local_50;
  size_t local_4c;
  int local_48;
  int local_44;
  int local_40;
  int *local_3c;
  char *local_38;
  undefined1 local_34 [4];
  undefined2 local_30;
  undefined2 local_2e;
  int local_28;
  int local_10;
  int *local_c;
  int local_8;
  
  local_c = DAT_013b71e8 + 1;
  local_8 = (**(code **)(*DAT_0092e654 + 8))();
  if (param_2 == 0x1214) {
    if (((*(int *)(param_1[10] + 300) != 0) && (*(int *)(*(int *)(param_1[10] + 300) + 0x40) == 0xd)
        ) && (*(int *)(*(int *)(param_1[10] + 300) + 0x28) != 0)) {
      iVar2 = FUN_004aea2c();
      if (iVar2 == 0) {
        FUN_004aea8a();
      }
      else {
        FUN_004aeaeb();
      }
    }
  }
  else if ((param_2 == 0x1204) && (2000 < (uint)(local_8 - param_1[0x9ba1]))) {
    local_10 = (int)(char)DAT_013b71e8[0x3bd];
    if ((local_10 < 0) || (3 < local_10)) {
      FUN_00403df2(&DAT_00a34878,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    else if ((param_1[local_10 + 0x9b8c] != 0) && ((char)local_c[local_10 * 4 + 4] != '\0')) {
      _memset(local_34,0,0x24);
      local_2e = 0;
      local_30 = 0x213;
      local_28 = local_10;
      FUN_0042550e(local_34,0x24);
      param_1[0x9ba1] = local_8;
      (**(code **)(*(int *)param_1[0x9b94] + 100))(0);
      (**(code **)(*(int *)param_1[0x9b95] + 100))(0);
      (**(code **)(*(int *)param_1[0x9b96] + 100))(0);
    }
  }
  else if (param_2 == 0x609) {
    local_3c = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x1212);
    pcVar3 = (char *)(**(code **)(*local_3c + 0x88))();
    sVar4 = _strlen(pcVar3);
    if (sVar4 < 4) {
      FUN_00403df2(&DAT_00a348f8,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
    }
    else {
      pcVar3 = (char *)(**(code **)(*local_3c + 0x88))();
      sVar4 = _strlen(pcVar3);
      if (sVar4 < 0xd) {
        uVar5 = (**(code **)(*local_3c + 0x88))();
        cVar1 = FUN_0054eee7(uVar5);
        if (cVar1 == '\0') {
          FUN_00403df2(&DAT_00a349f8,2000);
          (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
        }
        else {
          uVar5 = (**(code **)(*local_3c + 0x88))();
          local_38 = (char *)FUN_004b17e2(uVar5);
          local_4c = _strlen(local_38);
          for (local_48 = 0; local_48 < (int)(local_4c - 1); local_48 = local_48 + 1) {
            if ((local_38[local_48] == -0x5f) && (local_38[local_48 + 1] == -0x5f)) {
              FUN_00403df2(&DAT_00a349f8,2000);
              (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
              return 1;
            }
          }
          local_40 = 0;
          local_48 = 0;
          while ((local_48 < 4 &&
                 ((local_c == (int *)0x0 || ((char)local_c[local_48 * 4 + 4] != '\0'))))) {
            local_40 = local_40 + 1;
            local_48 = local_48 + 1;
          }
          local_44 = -1;
          for (local_48 = 0; local_48 < 4; local_48 = local_48 + 1) {
            if (DAT_013b71e8[0x6bd2] == param_1[local_48 + 0x9b90]) {
              local_44 = local_48;
            }
          }
          if ((DAT_013b71e8[0x6bd2] == 0) || (local_44 == -1)) {
            FUN_00403df2(&DAT_00a34878,2000);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
          }
          else if ((-1 < local_40) && (local_40 < 4)) {
            _memset(local_70,0,0x24);
            local_6a = 0;
            local_6c = 0x20f;
            local_50 = local_44;
            local_64 = local_40;
            uVar5 = (**(code **)(*local_3c + 0x88))();
            FUN_0058f078(local_60,&DAT_005b9404,uVar5);
            FUN_0042550e(local_70,0x24);
            param_1[0x9ba2] = local_8;
            (**(code **)(*(int *)param_1[0x9b97] + 100))(0);
          }
        }
      }
      else {
        FUN_00403df2(&DAT_00a34978,2000);
        (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      }
    }
  }
  else if (param_2 == 0x1207) {
    (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a34a78,0x1207,0);
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  else if (param_2 == 0x1205) {
    (**(code **)(*param_1 + 0x68))(0);
  }
  else if (param_2 == 0x1208) {
    if (DAT_005b892c == 2) {
      local_74 = 0;
      while ((local_74 < 4 &&
             ((param_1[local_74 + 0x9b8c] == 0 ||
              (*(int *)(param_1[local_74 + 0x9b8c] + 0x234) == 0))))) {
        local_74 = local_74 + 1;
      }
      if (local_74 == 4) {
        (**(code **)(*param_1 + 0x68))(1);
      }
      else {
        FUN_004a218c(local_74,0,1);
        *(undefined4 *)(param_1[local_74 + 0x9b8c] + 0x234) = 0;
        local_78 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x502);
        (**(code **)(*local_78 + 0x60))(0);
      }
    }
    else {
      iVar2 = FUN_0040c0f0();
      if (iVar2 == 1) {
        (**(code **)(*(int *)param_1[0x24] + 0x60))(0);
      }
      else {
        local_7c = (**(code **)(*(int *)param_1[10] + 0x48))(0x606);
        iVar2 = FUN_0040c0f0();
        if (iVar2 == 1) {
          if (_DAT_005a43cc <= *(float *)(local_7c + 0x50) / DAT_005b4910) {
            if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x33), iVar2 != 0)) {
              uVar6 = 0;
              uVar5 = 0;
              FUN_00429a6d(0x33);
              FUN_0042ad2b(uVar5,uVar6);
            }
            (**(code **)(*param_1 + 0x68))(1);
          }
          else {
            if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x39), iVar2 != 0)) {
              uVar6 = 0;
              uVar5 = 0;
              FUN_00429a6d(0x39);
              FUN_0042ad2b(uVar5,uVar6);
            }
            iVar2 = (**(code **)(*DAT_0092e654 + 8))();
            param_1[0x9ba0] = iVar2;
            FUN_004a24b4(s_UI_CreCamAction3_005b9408);
          }
        }
        else {
          local_80 = (**(code **)(*(int *)param_1[10] + 0x48))(0x502);
          if (_DAT_005a43cc <= *(float *)(local_80 + 0x50) / DAT_005b4910) {
            (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a346f8,0x130,0);
            (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
          }
          else {
            if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x39), iVar2 != 0)) {
              uVar6 = 0;
              uVar5 = 0;
              FUN_00429a6d(0x39);
              FUN_0042ad2b(uVar5,uVar6);
            }
            iVar2 = (**(code **)(*DAT_0092e654 + 8))();
            param_1[0x9b9e] = iVar2;
            FUN_004a24b4(s_UI_SelCamAction3_005b941c);
          }
        }
      }
    }
  }
  else if (param_2 == 0x610) {
    if (DAT_005b892c == 2) {
      local_84 = 0;
      while ((local_84 < 4 &&
             ((param_1[local_84 + 0x9b90] == 0 ||
              (*(int *)(param_1[local_84 + 0x9b90] + 0x234) == 0))))) {
        local_84 = local_84 + 1;
      }
      if (local_84 == 4) {
        (**(code **)(*param_1 + 0x68))(1);
      }
      else {
        FUN_004a218c(local_84,0,0);
        *(undefined4 *)(param_1[local_84 + 0x9b90] + 0x234) = 0;
        local_88 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x606);
        (**(code **)(*local_88 + 0x60))(0);
      }
    }
    else {
      (**(code **)(*param_1 + 0x68))(1);
    }
  }
  else if (param_2 == 0x274) {
    if (param_1[0x9b9a] != 0) {
      (**(code **)(*(int *)param_1[0x9b9a] + 0x60))(0);
    }
    if (param_1[0x9b9b] != 0) {
      pcVar3 = (char *)(**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
      sVar4 = _strlen(pcVar3);
      if (sVar4 < 4) {
        FUN_00403df2(&DAT_00a348f8,2000);
        (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      }
      else {
        pcVar3 = (char *)(**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
        sVar4 = _strlen(pcVar3);
        if (sVar4 < 0xd) {
          uVar5 = (**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
          cVar1 = FUN_0054eee7(uVar5);
          if (cVar1 == '\0') {
            FUN_00403df2(&DAT_00a349f8,2000);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
          }
          else {
            uVar5 = (**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
            pcVar3 = (char *)FUN_004b17e2(uVar5);
            _Str1 = (char *)(**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
            iVar2 = _strcmp(_Str1,pcVar3);
            if (iVar2 == 0) {
              local_8c = (char *)(**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
              local_cc = _strlen(local_8c);
              for (local_c8 = 0; local_c8 < (int)(local_cc - 1); local_c8 = local_c8 + 1) {
                if ((local_8c[local_c8] == -0x5f) && (local_8c[local_c8 + 1] == -0x5f)) {
                  FUN_00403df2(&DAT_00a349f8,2000);
                  (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
                  return 1;
                }
              }
              local_c4 = (int)(char)DAT_013b71e8[0x3bd];
              local_ba = 0;
              local_bc = 0xfaa;
              local_b4 = 0;
              local_b0 = local_c4;
              FUN_0058f078(local_ac,&DAT_005b9430,local_c + local_c4 * 4 + 4);
              uVar5 = (**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
              FUN_0058f078(local_9c,&DAT_005b9434,uVar5);
              FUN_0042550e(local_c0,0x34);
              param_1[0x9b87] = 1;
              param_1[0x9b9c] = local_8;
            }
            else {
              FUN_00403df2(&DAT_00a349f8,2000);
              (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
            }
          }
        }
        else {
          FUN_00403df2(&DAT_00a34978,2000);
          (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
        }
      }
    }
  }
  else if (param_2 == 0x275) {
    if (param_1[0x9b9a] != 0) {
      (**(code **)(*(int *)param_1[0x9b9a] + 0x60))(0);
    }
    param_1[0x9b87] = 0;
  }
  else if (param_2 == 0x400) {
    local_d0 = (int)(char)DAT_013b71e8[0x3bd];
    _memset(local_fc,0,0x2c);
    local_f6 = 0;
    local_f8 = 0x211;
    local_f0 = local_d0;
    FUN_0058f078(local_ec,&DAT_005b9438,local_c + local_d0 * 4 + 4);
    uVar5 = (**(code **)(*(int *)param_1[0x9ba4] + 0x88))();
    FUN_0058f078(local_dc,&DAT_005b943c,uVar5);
    FUN_0042550e(local_fc,0x2c);
    _memset((void *)(param_1[0x9ba4] + 0xe4c),0,4);
    (**(code **)(*(int *)param_1[0x9ba3] + 0x60))(0);
    (**(code **)(*(int *)param_1[10] + 0x40))(0);
  }
  else if (param_2 == 0x399) {
    (**(code **)(*(int *)param_1[0x9ba3] + 0x60))(0);
    (**(code **)(*(int *)param_1[10] + 0x40))(0);
  }
  else if (param_2 == 0x1209) {
    if (param_3 == 0) {
      local_124 = (int)(char)DAT_013b71e8[0x3bd];
      local_100 = local_124;
      if (*(int *)(param_1[0x23] + 0x1ec) == 0x130) {
        (**(code **)(*DAT_013b71e8 + 0x54))(7);
      }
      else if (((*(int *)(param_1[0x23] + 0x1ec) == 0x1207) && (-1 < local_124)) && (local_124 < 4))
      {
        (**(code **)(*(int *)param_1[0x9ba3] + 0x60))(1);
        (**(code **)(*(int *)param_1[10] + 0x40))(param_1[0x9ba4]);
      }
      else if (*(int *)(param_1[0x23] + 0x1ec) == 0x1202) {
        ShellExecuteA((HWND)0x0,(LPCSTR)0x0,&DAT_00a3c4f8,(LPCSTR)0x0,(LPCSTR)0x0,3);
      }
      else if (*(int *)(param_1[0x23] + 0x1ec) == 1) {
        if (-1 < local_124) {
          local_12e = 0;
          local_130 = 0xfaa;
          local_128 = 0;
          FUN_0058f078(local_120,&DAT_005b9440,local_c + local_124 * 4 + 4);
          FUN_0058f078(local_110,&DAT_005b9444,local_c + local_100 * 4 + 4);
          FUN_0042550e(local_134,0x34);
          param_1[0x9b87] = 1;
          param_1[0x9b9c] = local_8;
          FUN_00403df2(&DAT_00a3a878,0);
          (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
        }
      }
      else if (*(int *)(param_1[0x23] + 0x1ec) == 0) {
        (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
      }
    }
    else if (param_3 == 1) {
      (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
    }
  }
  else if ((DAT_005b892c == 2) && (param_2 == 0x1629)) {
    if (param_1[0x9bbd] == 0) {
      (**(code **)(*param_1 + 0x68))(1);
    }
    else {
      (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a346f8,0x130,0);
      (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
    }
  }
  return 1;
}

