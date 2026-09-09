// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423f37 | Name: FUN_00423f37


SOCKET __thiscall
FUN_00423f37(SOCKET *param_1,char *param_2,u_short param_3,undefined4 param_4,u_int param_5)

{
  short sVar1;
  int iVar2;
  sockaddr local_2c;
  char local_1c [4];
  sockaddr local_18;
  SOCKET local_8;
  
  _memset(&local_2c,0,0x10);
  _memset(&local_18,0,0x10);
  param_1[3] = 0;
  param_1[6] = 0;
  param_1[4] = 0;
  param_1[5] = 0;
  if (*param_1 != 0) {
    FUN_00423d7a();
  }
  local_2c.sa_data._2_4_ = inet_addr(param_2);
  local_2c.sa_family = 2;
  local_2c.sa_data._0_2_ = htons(param_3);
  local_8 = socket(2,1,0);
  if (local_8 == 0xffffffff) {
    MessageBoxA((HWND)0x0,s_Initialize_socket_fai_005b1f80,s_ERROR_005b1f78,0);
    local_8 = 0;
  }
  else {
    local_18.sa_family = 2;
    local_18.sa_data._2_4_ = param_4;
    local_18.sa_data[0] = '\0';
    local_18.sa_data[1] = '\0';
    iVar2 = bind(local_8,&local_18,0x10);
    if (iVar2 == -1) {
      sVar1 = (short)DAT_005ccf90;
      DAT_005ccf90 = DAT_005ccf90 + 10;
      local_18.sa_data._0_2_ = htons(sVar1 + 0x1392);
      iVar2 = bind(local_8,&local_18,0x10);
      if (iVar2 == -1) {
        sVar1 = (short)DAT_005ccf90;
        DAT_005ccf90 = DAT_005ccf90 + 10;
        local_18.sa_data._0_2_ = htons(sVar1 + 0x1392);
        iVar2 = bind(local_8,&local_18,0x10);
        if (iVar2 == -1) {
          MessageBoxA((HWND)0x0,s_Binding_fail_005b1fa0,s_ERROR_005b1f98,0);
          closesocket(local_8);
          return 0;
        }
      }
    }
    if (local_8 == 0xffffffff) {
      local_8 = 0;
    }
    else {
      iVar2 = connect(local_8,&local_2c,0x10);
      if (iVar2 < 0) {
        WSAGetLastError();
        closesocket(local_8);
        *param_1 = 0;
        local_8 = 0;
      }
      else {
        iVar2 = WSAAsyncSelect(local_8,DAT_013b7210,param_5,0x21);
        if (iVar2 < 1) {
          *param_1 = local_8;
          local_1c[0] = '\x11';
          local_1c[1] = -0xd;
          local_1c[2] = '\x11';
          local_1c[3] = '\x1f';
          send(local_8,local_1c,4,0);
          param_1[7] = 1;
        }
        else {
          closesocket(local_8);
          *param_1 = 0;
          local_8 = 0;
        }
      }
    }
  }
  return local_8;
}

