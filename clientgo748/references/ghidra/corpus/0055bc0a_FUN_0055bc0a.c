// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055bc0a | Name: FUN_0055bc0a


undefined4 __thiscall FUN_0055bc0a(int param_1,HINSTANCE param_2,undefined4 param_3)

{
  int iVar1;
  HWND pHVar2;
  undefined4 uVar3;
  void *pvVar4;
  char *pcVar5;
  undefined4 local_2b8;
  int local_29c;
  WNDCLASSA local_298;
  undefined *local_270;
  tagRECT local_26c;
  int local_25c;
  int local_258;
  int local_254;
  int local_250;
  int local_24c;
  uint local_248;
  undefined1 local_244 [4];
  int local_240 [2];
  undefined2 local_238 [2];
  uint local_234;
  undefined4 local_230;
  char local_22c [8];
  char local_224;
  undefined1 local_223 [247];
  int local_12c;
  char local_128 [8];
  int local_120 [35];
  undefined1 *local_94;
  undefined1 local_90 [128];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2b5e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x18) = 0x10;
  *(undefined4 *)(param_1 + 0x1c) = 0;
  DAT_013b7224 = param_2;
  *(undefined4 *)(param_1 + 0x10) = 800;
  *(undefined4 *)(param_1 + 0x14) = 600;
  *(undefined4 *)(param_1 + 0x104) = 1;
  *(undefined4 *)(param_1 + 0x18) = 0x10;
  *(undefined4 *)(param_1 + 0x1c) = param_3;
  FUN_0055b299();
  local_120[1] = 0x280;
  local_120[2] = 0x1e0;
  local_120[3] = 0x10;
  local_120[4] = 800;
  local_120[5] = 600;
  local_120[6] = 0x10;
  local_120[7] = 0x400;
  local_120[8] = 0x300;
  local_120[9] = 0x10;
  local_120[10] = 0x500;
  local_120[0xb] = 0x400;
  local_120[0xc] = 0x10;
  local_120[0xd] = 0x640;
  local_120[0xe] = 0x4b0;
  local_120[0xf] = 0x10;
  local_120[0x10] = 0x280;
  local_120[0x11] = 0x1e0;
  local_120[0x12] = 0x20;
  local_120[0x13] = 800;
  local_120[0x14] = 600;
  local_120[0x15] = 0x20;
  local_120[0x16] = 0x400;
  local_120[0x17] = 0x300;
  local_120[0x18] = 0x20;
  local_120[0x19] = 0x500;
  local_120[0x1a] = 0x400;
  local_120[0x1b] = 0x20;
  local_120[0x1c] = 0x640;
  local_120[0x1d] = 0x4b0;
  local_120[0x1e] = 0x20;
  local_120[0x1f] = 0xc80;
  local_120[0x20] = 0x960;
  local_120[0x21] = 0x20;
  local_120[0] = 2;
  local_12c = 0x32;
  local_258 = 0;
  local_240[0] = 0;
  local_230 = 0x1e;
  local_234 = 0;
  local_248 = 0;
  local_120[0x22] = 1;
  local_254 = FUN_0058f716(s_config_txt_005c4c20,&DAT_005c4c1c);
  if (local_254 != 0) {
    local_25c = -1;
    FUN_00590a68(local_254,s__VERSION___d_005c4c2c,local_244);
    FUN_00590a68(local_254,s__RES___d_005c4c3c,local_120);
    FUN_00590a68(local_254,s__ANIMATION___d_005c4c48,&DAT_005bab20);
    if (2 < DAT_005bab20) {
      DAT_005bab20 = 2;
    }
    if (DAT_005bab20 >= 2) {
      DAT_0092e664 = 1;
    }
    DAT_0092e660 = (uint)(DAT_005bab20 < 2);
    FUN_00590a68(local_254,s__SOUND___d_005c4c5c,param_1 + 0x104);
    FUN_00590a68(local_254,s__MUSIC___d_005c4c6c,param_1 + 0x108);
    FUN_00590a68(local_254,s__SERVER___d_005c4c7c,local_244);
    iVar1 = FUN_00590a68(local_254,s__BRIGHT___d_005c4c8c,&local_12c);
    if (iVar1 == -1) {
      local_12c = 0x32;
    }
    iVar1 = FUN_00590a68(local_254,s__CURSOR___d_005c4c9c,&local_258);
    if (iVar1 == -1) {
      local_258 = 0;
    }
    iVar1 = FUN_00590a68(local_254,s__DEMO___d_005c4cac,&DAT_005c4958);
    if (iVar1 == -1) {
      DAT_005c4958 = 1;
    }
    FUN_00590a68(local_254,s__WINDOW___d_005c4cb8,&local_25c);
    if (-1 < local_25c) {
      *(uint *)(param_1 + 0x1c) = (uint)(local_25c == 0);
    }
    iVar1 = FUN_00590a68(local_254,s__CLASSIC___d_005c4cc8,local_240);
    if (iVar1 == -1) {
      DAT_005b892c = 2;
    }
    if (((local_240[0] == 1) || (local_120[0] == 1)) || (local_120[0] == 6)) {
      DAT_005b892c = 1;
    }
    FUN_00590a68(local_254,s__CAMERAROTATE___d_005c4cd8,&local_234);
    local_234 = (uint)(0 < (int)local_234);
    FUN_00590a68(local_254,s__DXT___d_005c4cec,&local_248);
    local_248 = (uint)(0 < (int)local_248);
    iVar1 = FUN_00590a68(local_254,s__KEYTYPE___d_005c4cf8,&DAT_013b722c);
    if (iVar1 == -1) {
      DAT_013b722c = 0;
    }
    *(undefined4 *)(param_1 + 0x10c) = 1;
    FUN_00590a68(local_254,s__CAMERAVIEW___d_005c4d08,local_120 + 0x22);
    *(int *)(param_1 + 0x10c) = local_120[0x22];
    FUN_0058f62c(local_254);
  }
  DAT_005b892c = 2;
  DAT_005b4924 = (uint)(local_248 == 0);
  DAT_005b4928 = local_234;
  DAT_005b4920 = local_230;
  if (local_12c < 0) {
    local_12c = 0;
  }
  if (100 < local_12c) {
    local_12c = 100;
  }
  DAT_005b491c = local_12c;
  DAT_005bab20 = 1;
  if (local_258 == 1) {
    local_258 = 0;
  }
  else {
    local_258 = 2;
  }
  DAT_005ccec8 = local_258;
  if (local_258 == 2) {
    DAT_005ccecc = LoadCursorA(param_2,(LPCSTR)0xa2);
    DAT_005cced0 = LoadCursorA(param_2,(LPCSTR)0xa4);
    if (DAT_005ccecc != (HCURSOR)0x0) {
      SetCursor(DAT_005ccecc);
    }
  }
  *(int *)(param_1 + 0x10) = local_120[(local_120[0] + -1) * 3 + 1];
  *(int *)(param_1 + 0x14) = local_120[(local_120[0] + -1) * 3 + 2];
  *(int *)(param_1 + 0x18) = local_120[(local_120[0] + -1) * 3 + 3];
  iVar1 = FUN_0055f24b(*(undefined4 *)(param_1 + 0x10),*(undefined4 *)(param_1 + 0x14),
                       *(undefined4 *)(param_1 + 0x18));
  if (iVar1 == 0) {
    *(int *)(param_1 + 0x10) = local_120[4];
    *(int *)(param_1 + 0x14) = local_120[5];
    *(int *)(param_1 + 0x18) = local_120[6];
  }
  if (DAT_005b892c == 2) {
    DAT_005b4914 = 0xc;
    DAT_005b4918 = 0x100;
  }
  else {
    if (*(int *)(param_1 + 0x10) == 0x280) {
      DAT_005b4914 = 10;
      DAT_005b4918 = 0x100;
    }
    if (*(int *)(param_1 + 0x10) == 800) {
      DAT_005b4914 = 0xc;
      DAT_005b4918 = 0x100;
    }
    if (*(int *)(param_1 + 0x10) == 0x400) {
      DAT_005b4914 = 0xe;
      DAT_005b4918 = 0x200;
    }
    if (*(int *)(param_1 + 0x10) == 0x500) {
      DAT_005b4914 = 0x14;
      DAT_005b4918 = 0x200;
    }
    if (*(int *)(param_1 + 0x10) == 0x640) {
      DAT_005b4914 = 0x18;
      DAT_005b4918 = 0x200;
    }
  }
  FUN_0055b176();
  FUN_0055cf8f();
  if (*(int *)(param_1 + 4) == 0) {
    local_298.style = 8;
    local_298.lpfnWndProc = FUN_0055fa89;
    local_298.cbClsExtra = 0;
    local_298.cbWndExtra = 0;
    local_298.hInstance = param_2;
    local_298.hIcon = LoadIconA(param_2,(LPCSTR)0x65);
    local_298.hCursor = LoadCursorA((HINSTANCE)0x0,(LPCSTR)0x7f00);
    local_298.hbrBackground = GetStockObject(4);
    local_298.lpszMenuName = (LPCSTR)0x0;
    local_298.lpszClassName = s_With_Your_Destiny_005c4d1c;
    RegisterClassA(&local_298);
    if (*(int *)(param_1 + 0x1c) == 0) {
      local_270 = &DAT_00ca0000;
    }
    else {
      local_270 = (undefined *)0x80000000;
    }
    *(uint *)(param_1 + 0xc) = (uint)local_270 | 0x10000000;
    SetRect(&local_26c,0,0,*(int *)(param_1 + 0x10),*(int *)(param_1 + 0x14));
    AdjustWindowRect(&local_26c,*(DWORD *)(param_1 + 0xc),0);
    pHVar2 = CreateWindowExA(0,s_With_Your_Destiny_005c4d30,(LPCSTR)(param_1 + 0x20),
                             *(DWORD *)(param_1 + 0xc),0,0,local_26c.right - local_26c.left,
                             local_26c.bottom - local_26c.top,(HWND)0x0,(HMENU)0x0,param_2,
                             (LPVOID)0x0);
    *(HWND *)(param_1 + 4) = pHVar2;
  }
  DAT_013b7364 = CreateWindowExA(0,s_static_005c4d44,&DAT_013b7234,0x50000000,
                                 *(int *)(param_1 + 0x10) - 800U >> 1,
                                 *(int *)(param_1 + 0x14) - 0x230U >> 1,800,0x230,
                                 *(HWND *)(param_1 + 4),(HMENU)0x0,param_2,(LPVOID)0x0);
  if (DAT_013b7364 != (HWND)0x0) {
    iVar1 = FUN_0059e150(DAT_013b7364);
    if (iVar1 != 0) {
      ExceptionList = local_10;
      return 0;
    }
    FUN_0059e010(DAT_013b7364,&DAT_00a3c7f8);
    FUN_0059e100(DAT_013b7364,800,0x230);
    ShowWindow(DAT_013b7364,0);
    SetFocus(*(HWND *)(param_1 + 4));
    FUN_0059dfe0(DAT_013b7364);
  }
  DAT_013b7368 = CreateWindowExA(0,s_Button_005c4d4c,&DAT_013b7238,0x5080000b,
                                 (*(int *)(param_1 + 0x10) - 800U >> 1) + 0x2ca,
                                 (*(int *)(param_1 + 0x14) - 0x230U >> 1) + 0x230,0x56,0x15,
                                 *(HWND *)(param_1 + 4),(HMENU)0x3e7,param_2,(LPVOID)0x0);
  ShowWindow(DAT_013b7368,0);
  DAT_013b7210 = *(undefined4 *)(param_1 + 4);
  iVar1 = FUN_0054f0f2();
  if (iVar1 == 0) {
    MessageBoxA(*(HWND *)(param_1 + 4),s_Initialize_Data_Failed_005c4d5c,s_Error_005c4d54,0);
    FUN_0043125a(s_Initialize_Data_Failed_005c4d74);
    uVar3 = 0;
  }
  else {
    FUN_004b1d64();
    FUN_0055b42f();
    pvVar4 = operator_new(0x124);
    local_8 = 0;
    if (pvVar4 == (void *)0x0) {
      local_2b8 = 0;
    }
    else {
      local_2b8 = FUN_00431320(*(undefined4 *)(param_1 + 0x1c));
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0xe8) = local_2b8;
    if (((*(int *)(param_1 + 0xe8) != 0) && (iVar1 = FUN_0043192f(s_wyd_avi_005c4d90), iVar1 == 0))
       && (*(int *)(param_1 + 0xe8) != 0)) {
      if (*(undefined4 **)(param_1 + 0xe8) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0xe8))(1);
      }
      *(undefined4 *)(param_1 + 0xe8) = 0;
    }
    local_94 = &LAB_00577a88;
    _memset(&DAT_00e3f340,0,0x577a88);
    FUN_0058f078(local_90,s_itemHelp_dat_005c4d98);
    local_254 = FUN_0058f716(local_90,&DAT_005c4da8);
    if (local_254 == 0) {
      uVar3 = 0;
    }
    else {
      local_24c = 0;
      local_240[1] = 0;
      local_238[0] = 0;
      for (local_250 = 0; local_250 < 65000; local_250 = local_250 + 1) {
        local_29c = 0;
        while ((local_29c < 10 && (iVar1 = FUN_00590a06(local_22c,0x100,local_254), iVar1 != 0))) {
          if (local_29c == 0) {
            FUN_005909d2(local_22c,&DAT_005c4dac,&local_24c);
          }
          else {
            _memset(local_128,0,7);
            _strncpy(local_128,local_22c,6);
            FUN_005909d2(local_128,&DAT_005c4db0,local_238);
            pcVar5 = _strstr(local_22c,&DAT_005c4db4);
            if (pcVar5 != (char *)0x0) {
              *pcVar5 = '\0';
            }
            if ((local_224 == '\t') || (local_224 == ' ')) {
              FUN_0058f078(&DAT_00e3f352 + (local_29c + -1) * 0x60 + local_24c * 0x372,&DAT_005c4db8
                           ,local_223);
            }
            *(undefined2 *)(&DAT_00e3f33e + local_29c * 2 + local_24c * 0x372) = local_238[0];
          }
          local_29c = local_29c + 1;
        }
      }
      FUN_0058f62c(local_254);
      if (*(int *)(param_1 + 0xe8) == 0) {
        uVar3 = FUN_0055ca18();
      }
      else {
        uVar3 = 1;
      }
    }
  }
  ExceptionList = local_10;
  return uVar3;
}

