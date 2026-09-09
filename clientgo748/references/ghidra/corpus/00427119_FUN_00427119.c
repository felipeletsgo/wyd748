// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00427119 | Name: FUN_00427119


int __fastcall FUN_00427119(int *param_1)

{
  undefined4 uVar1;
  char *pcVar2;
  HWND hWnd;
  int iVar3;
  int *lpString;
  uint local_a0;
  int *local_9c;
  int *local_98;
  uint *local_94;
  char local_90 [128];
  HDC local_10;
  int local_c;
  int *local_8;
  
  local_8 = param_1 + param_1[0xa8e0] * 0x10e3 + 1;
  local_98 = local_8 + local_8[0x10e2] * 0x32a + 0x110;
  local_94 = (uint *)(local_98 + local_98[0x327] * 5 + 0x39);
  uVar1 = FUN_0059e5fe(local_8 + 0x80);
  FUN_0058f078(local_90,&DAT_005b27f4,uVar1);
  pcVar2 = _strstr(local_90,s_VOODOO_005b27f8);
  if ((pcVar2 != (char *)0x0) && (local_98[0x17] == 0x100)) {
    param_1[0xa8e4] = 1;
    param_1[0xa944] = 0x10;
    DAT_005ba694 = 0x100;
    DAT_005ba698 = 0x100;
    lpString = local_8 + 0x80;
    hWnd = (HWND)FUN_0055b288();
    SetWindowTextA(hWnd,(LPCSTR)lpString);
    if (800 < *local_94) {
      MessageBoxA((HWND)0x0,s_Set_Resolution_800x600_or_less__005b280c,s_Init_Error_005b2800,0x10);
      return -0x7fffbffb;
    }
  }
  if (0x100 < (uint)local_98[0x17]) {
    DAT_005ba694 = 0x200;
    DAT_005ba698 = 0x200;
  }
  pcVar2 = _strstr(local_90,s_NVIDIA_005b282c);
  if (pcVar2 != (char *)0x0) {
    param_1[0xa8e5] = 1;
    pcVar2 = _strstr(local_90,&DAT_005b2834);
    if (((pcVar2 != (char *)0x0) ||
        (pcVar2 = _strstr(local_90,&DAT_005b283c), pcVar2 != (char *)0x0)) ||
       (pcVar2 = _strstr(local_90,&DAT_005b2840), pcVar2 != (char *)0x0)) {
      param_1[0xa972] = 1;
    }
  }
  pcVar2 = _strstr(local_90,s_SAVAGE_005b2848);
  if (pcVar2 != (char *)0x0) {
    param_1[0xa8e7] = 1;
  }
  pcVar2 = _strstr(local_90,s_MATROX_005b2850);
  if ((pcVar2 != (char *)0x0) && (pcVar2 = _strstr(local_90,&DAT_005b2858), pcVar2 != (char *)0x0))
  {
    param_1[0xa8e6] = 1;
  }
  pcVar2 = _strstr(local_90,s_INTEL_005b285c);
  if ((pcVar2 != (char *)0x0) || (pcVar2 = _strstr(local_90,&DAT_005b2864), pcVar2 != (char *)0x0))
  {
    param_1[0xa8e8] = 1;
  }
  _memset(param_1 + 0xa8e9,0,0x34);
  param_1[0xa8f0] = 1;
  param_1[0xa8ec] = 1;
  param_1[0xa8ed] = local_98[0x329];
  param_1[0xa8ee] = 4;
  param_1[0xa8f1] = param_1[0xa969];
  param_1[0xa8f2] = local_94[4];
  param_1[0xa8ef] = param_1[0xa8f6];
  if (param_1[0xa8e1] != 0) {
    param_1[0xa8e9] = param_1[0xa93f] - param_1[0xa93d];
    param_1[0xa8ea] = param_1[0xa940] - param_1[0xa93e];
    param_1[0xa8eb] = local_8[0x10e];
    param_1[0xa8f3] = param_1[0xa8f3] | 1;
    if (((param_1[0xa8eb] == 0x17) || (param_1[0xa8eb] == 0x18)) || (param_1[0xa8eb] == 0x19)) {
      param_1[0xa944] = 0x10;
    }
    else {
      param_1[0xa944] = 0x20;
    }
  }
  local_10 = CreateDCA(s_DISPLAY_005b2868,(LPCSTR)0x0,(LPCSTR)0x0,(DEVMODEA *)0x0);
  iVar3 = GetDeviceCaps(local_10,8);
  param_1[0xa96c] = iVar3;
  iVar3 = GetDeviceCaps(local_10,10);
  param_1[0xa96d] = iVar3;
  iVar3 = GetDeviceCaps(local_10,0xc);
  param_1[0xa96e] = iVar3;
  iVar3 = GetDeviceCaps(local_10,0x74);
  param_1[0xa96f] = iVar3;
  DeleteDC(local_10);
  if (param_1[0xa943] != 0) {
    FUN_00426fdc(param_1[0xa941],param_1[0xa942],param_1[0xa96e],0);
  }
  local_c = (**(code **)(*(int *)param_1[0xa8f8] + 0x3c))
                      (param_1[0xa8f8],param_1[0xa8e0],*local_98,param_1[0xa8f7],local_94[3] | 4,
                       param_1 + 0xa8e9,param_1 + 0xa8f9);
  if (local_c < 0) {
    FUN_0043113f(0x10000003,local_c);
  }
  else {
    (**(code **)(*(int *)param_1[0xa8f9] + 0x1c))(param_1[0xa8f9],param_1 + 0xa8fa);
    param_1[0xa937] = local_94[3];
    if (param_1[0xa910] == 0x100) {
      DAT_005b892c = 1;
    }
    if (*local_98 == 2) {
      lstrcpyA((LPSTR)(param_1 + 0xa948),&DAT_005b2870);
    }
    else if (*local_98 == 1) {
      lstrcpyA((LPSTR)(param_1 + 0xa948),&DAT_005b2874);
    }
    else if (*local_98 == 3) {
      lstrcpyA((LPSTR)(param_1 + 0xa948),&DAT_005b2878);
    }
    if (((local_94[3] & 0x40) == 0) || ((local_94[3] & 0x10) == 0)) {
      if ((local_94[3] & 0x40) == 0) {
        if ((local_94[3] & 0x80) == 0) {
          if ((local_94[3] & 0x20) != 0) {
            lstrcatA((LPSTR)(param_1 + 0xa948),s__sw_vp__005b28e8);
          }
        }
        else if (*local_98 == 1) {
          lstrcatA((LPSTR)(param_1 + 0xa948),s__mixed_vp__005b28c4);
        }
        else {
          lstrcatA((LPSTR)(param_1 + 0xa948),s__simulated_mixed_vp__005b28d0);
        }
      }
      else if (*local_98 == 1) {
        lstrcatA((LPSTR)(param_1 + 0xa948),s__hw_vp__005b28a4);
      }
      else {
        lstrcatA((LPSTR)(param_1 + 0xa948),s__simulated_hw_vp__005b28b0);
      }
    }
    else if (*local_98 == 1) {
      lstrcatA((LPSTR)(param_1 + 0xa948),s__pure_hw_vp__005b287c);
    }
    else {
      lstrcatA((LPSTR)(param_1 + 0xa948),s__simulated_pure_hw_vp__005b288c);
    }
    if (*local_98 == 1) {
      lstrcatA((LPSTR)(param_1 + 0xa948),&DAT_005b28f4);
      lstrcatA((LPSTR)(param_1 + 0xa948),(LPCSTR)(local_8 + 0x80));
    }
    (**(code **)(*(int *)param_1[0xa8f9] + 0x40))(param_1[0xa8f9],0,0,&local_9c);
    (**(code **)(*local_9c + 0x20))(local_9c,param_1 + 0xa92f);
    (**(code **)(*local_9c + 8))(local_9c);
    local_c = (**(code **)(*param_1 + 0xc))();
    if ((-1 < local_c) && (local_c = (**(code **)(*param_1 + 0x10))(), -1 < local_c)) {
      param_1[0xa8e2] = 1;
      return 0;
    }
    (**(code **)(*param_1 + 0x14))();
    (**(code **)(*param_1 + 0x18))();
    if (param_1[0xa8f9] != 0) {
      (**(code **)(*(int *)param_1[0xa8f9] + 8))(param_1[0xa8f9]);
      param_1[0xa8f9] = 0;
    }
  }
  if (*local_98 == 1) {
    FUN_00425d36(local_c,2);
    param_1[0xa8e0] = 0;
    local_8 = param_1 + param_1[0xa8e0] * 0x10e3 + 1;
    for (local_a0 = 0; local_a0 < (uint)local_8[0x10f]; local_a0 = local_a0 + 1) {
      if (local_8[local_a0 * 0x32a + 0x110] == 2) {
        local_8[0x10e2] = local_a0;
        local_98 = local_8 + local_a0 * 0x32a + 0x110;
        param_1[0xa8e1] = local_98[0x328];
        break;
      }
    }
    if (local_8[local_8[0x10e2] * 0x32a + 0x110] == 2) {
      local_c = FUN_00427119();
    }
  }
  return local_c;
}

