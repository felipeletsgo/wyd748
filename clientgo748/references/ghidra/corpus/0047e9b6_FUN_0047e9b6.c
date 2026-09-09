// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e9b6 | Name: FUN_0047e9b6


void __fastcall FUN_0047e9b6(int param_1)

{
  int iVar1;
  size_t sVar2;
  HWND hWnd;
  char *pcVar3;
  UINT uCmd;
  undefined1 local_4a0 [4];
  undefined2 local_49c;
  undefined2 local_49a;
  undefined4 local_494;
  undefined4 local_490;
  CHAR local_48c [256];
  CHAR local_38c [256];
  HWND local_28c;
  int local_288;
  undefined1 local_284 [20];
  int local_270;
  CHAR local_260 [256];
  undefined1 local_160 [36];
  CHAR local_13c [3];
  undefined1 local_139;
  undefined1 local_138;
  undefined1 local_137;
  undefined1 local_136;
  undefined1 local_135;
  undefined1 local_134;
  undefined1 local_133;
  undefined1 local_132;
  undefined1 local_131;
  undefined1 local_130;
  undefined1 local_12f;
  undefined1 local_12e;
  undefined1 local_12d;
  undefined1 local_12c;
  undefined1 local_12b [239];
  uint local_3c;
  uint local_38;
  uint local_34;
  uint local_30;
  size_t local_2c;
  int local_28;
  int local_24;
  int *local_20;
  int local_1c;
  undefined1 local_18 [4];
  undefined2 local_14;
  undefined2 local_12;
  uint local_c;
  int local_8;
  
  local_8 = (**(code **)(*DAT_0092e654 + 8))();
  if (250000 < (uint)(local_8 - DAT_0092e65c)) {
    _memset(local_18,0,0xc);
    local_12 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_14 = 0x3a0;
    FUN_0055f2dd(local_18,0xc);
  }
  local_c = *(uint *)(*(int *)(param_1 + 0x4c) + 0x20) % 100;
  if (3600000 < (local_8 - *(int *)(param_1 + 0x274d8)) + local_c * 5000) {
    local_1c = 0;
    local_20 = (int *)0x0;
    local_2c = 0;
    local_24 = GetTcpTable(0,&local_2c,1);
    if (local_20 == (int *)0x0) {
      local_20 = _malloc(local_2c);
      if (local_20 == (int *)0x0) {
        return;
      }
      local_24 = GetTcpTable(local_20,&local_2c,1);
    }
    if (local_24 == 0) {
      for (local_28 = 0; local_28 < *local_20; local_28 = local_28 + 1) {
        local_30 = local_20[local_28 * 5 + 4] & 0xff;
        local_34 = (local_20[local_28 * 5 + 4] & 0xff00U) >> 8;
        local_38 = (local_20[local_28 * 5 + 4] & 0xff0000U) >> 0x10;
        local_3c = (uint)local_20[local_28 * 5 + 4] >> 0x18;
        if (((local_30 == 0xd3) && (local_34 == 0xea)) && (local_38 == 0x77)) {
          local_1c = 1;
          break;
        }
      }
      if (local_20 != (int *)0x0) {
        FUN_0058ffd8(local_20);
      }
      if ((local_1c == 0) && (*(char *)(param_1 + 0x28ce8) != '\0')) {
        GetSystemDirectoryA(local_13c,0x100);
        local_139 = 0x50;
        local_138 = 0x72;
        local_137 = 0x6f;
        local_136 = 0x67;
        local_135 = 0x72;
        local_134 = 0x61;
        local_133 = 0x6d;
        local_132 = 0x20;
        local_131 = 0x46;
        local_130 = 0x69;
        local_12f = 0x6c;
        local_12e = 0x65;
        local_12d = 0x73;
        local_12c = 0x5c;
        FUN_0058f078(local_12b,param_1 + 0x28ce8);
        iVar1 = FUN_005906a7(local_13c,local_160);
        if (iVar1 == 0) {
          local_1c = 1;
        }
      }
      if ((local_1c == 0) && (*(char *)(param_1 + 0x28da8) != '\0')) {
        GetSystemDirectoryA(local_260,0x100);
        sVar2 = _strlen(local_260);
        local_260[sVar2] = '\\';
        local_288 = sVar2 + 1;
        FUN_0058f078(local_260 + sVar2 + 1,param_1 + 0x28da8);
        iVar1 = FUN_005906a7(local_260,local_284);
        if ((iVar1 == 0) && (60000 < local_270)) {
          local_1c = 1;
        }
      }
      if (local_1c == 0) {
        local_28c = (HWND)0x0;
        uCmd = 5;
        hWnd = GetDesktopWindow();
        for (local_28c = GetWindow(hWnd,uCmd); local_28c != (HWND)0x0;
            local_28c = GetWindow(local_28c,2)) {
          iVar1 = GetWindowTextA(local_28c,local_38c,0x100);
          if (iVar1 != 0) {
            local_490 = FUN_0059e5fe(local_38c);
            FUN_0058f078(local_38c,local_490);
            pcVar3 = _strstr(local_38c,(char *)(param_1 + 0x28c28));
            if ((pcVar3 != (char *)0x0) && (*(char *)(param_1 + 0x28c28) != '\0')) {
              local_1c = 1;
              break;
            }
          }
          iVar1 = GetClassNameA(local_28c,local_48c,0x100);
          if (iVar1 != 0) {
            local_494 = FUN_0059e5fe(local_48c);
            FUN_0058f078(local_48c,local_494);
            pcVar3 = _strstr(local_48c,(char *)(param_1 + 0x28c88));
            if ((pcVar3 != (char *)0x0) && (*(char *)(param_1 + 0x28c88) != '\0')) {
              local_1c = 1;
              break;
            }
          }
        }
      }
      if (local_1c == 1) {
        _memset(local_4a0,0,0xc);
        local_49a = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
        local_49c = 0x215;
        FUN_0055f2dd(local_4a0,0xc);
        FUN_00403df2(&DAT_00a35778,4000);
        (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
        FUN_00431159(s__CharacterLogout____Auto_005b872c);
      }
      *(int *)(param_1 + 0x274d8) = local_8;
    }
  }
  return;
}

