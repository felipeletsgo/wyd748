// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042553b | Name: FUN_0042553b


SOCKET __thiscall
FUN_0042553b(SOCKET *param_1,char *param_2,undefined4 param_3,undefined4 param_4,u_int param_5)

{
  short sVar1;
  int iVar2;
  ulong uVar3;
  undefined1 local_128 [256];
  sockaddr local_28;
  sockaddr local_18;
  SOCKET local_8;
  
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[5] = 0;
  if (*param_1 != 0) {
    FUN_00423d7a();
  }
  local_8 = socket(2,1,0);
  if (local_8 == 0xffffffff) {
    local_8 = 0;
  }
  else {
    local_18.sa_family = 2;
    local_18.sa_data[2] = (char)param_4;
    local_18.sa_data[3] = (char)((uint)param_4 >> 8);
    local_18.sa_data[4] = (char)((uint)param_4 >> 0x10);
    local_18.sa_data[5] = (char)((uint)param_4 >> 0x18);
    local_18.sa_data[0] = '\0';
    local_18.sa_data[1] = '\0';
    iVar2 = bind(local_8,&local_18,0x10);
    if (iVar2 == -1) {
      local_18.sa_data._0_2_ = htons((short)DAT_005ccf90 + 6000);
      iVar2 = bind(local_8,&local_18,0x10);
      if (iVar2 == -1) {
        sVar1 = (short)DAT_005ccf90;
        DAT_005ccf90 = DAT_005ccf90 + 10;
        local_18.sa_data._0_2_ = htons(sVar1 + 0x177a);
        iVar2 = bind(local_8,&local_18,0x10);
        if (iVar2 == -1) {
          MessageBoxA((HWND)0x0,s_Binding_fail_005b210c,s_ERROR_005b2104,0);
          closesocket(local_8);
          return 0;
        }
      }
    }
    FUN_0058f078(local_128,s_sock__d_ip__d__d__d__d_port__d___005b211c,local_8,local_18.sa_data[2],
                 local_18.sa_data[3],local_18.sa_data[4],local_18.sa_data[5],local_18.sa_data._0_2_,
                 DAT_005ccf90);
    DAT_005ccf90 = DAT_005ccf90 + 1;
    uVar3 = inet_addr(param_2);
    local_28.sa_data[2] = (char)uVar3;
    local_28.sa_data[3] = (char)(uVar3 >> 8);
    local_28.sa_data[4] = (char)(uVar3 >> 0x10);
    local_28.sa_data[5] = (char)(uVar3 >> 0x18);
    local_28.sa_family = 2;
    local_28.sa_data._0_2_ = htons((u_short)param_3);
    iVar2 = connect(local_8,&local_28,0x10);
    if (iVar2 < 0) {
      FUN_0058f078(local_128,s_sock__d_ip__d__d__d__d_port__d___005b2140,local_8,local_28.sa_data[2]
                   ,local_28.sa_data[3],local_28.sa_data[4],local_28.sa_data[5],
                   local_28.sa_data._0_2_,param_3);
      closesocket(local_8);
      *param_1 = 0;
      local_8 = 0;
    }
    else {
      FUN_0058f078(local_128,s_sock__d_ip__d__d__d__d_port__d___005b2164,local_8,local_28.sa_data[2]
                   ,local_28.sa_data[3],local_28.sa_data[4],local_28.sa_data[5],
                   local_28.sa_data._0_2_,param_3);
      iVar2 = WSAAsyncSelect(local_8,DAT_013b7210,param_5,0x21);
      if (iVar2 < 1) {
        *param_1 = local_8;
      }
      else {
        closesocket(local_8);
        *param_1 = 0;
        local_8 = 0;
      }
    }
  }
  return local_8;
}

