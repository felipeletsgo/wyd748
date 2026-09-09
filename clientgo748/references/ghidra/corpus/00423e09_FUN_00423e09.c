// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423e09 | Name: FUN_00423e09


SOCKET __thiscall
FUN_00423e09(SOCKET *param_1,HWND param_2,undefined4 param_3,u_short param_4,u_int param_5)

{
  int iVar1;
  sockaddr local_118;
  char local_108 [256];
  SOCKET local_8;
  
  local_8 = socket(2,1,0);
  if (local_8 == 0xffffffff) {
    MessageBoxA(param_2,s_Initialize_socket_fai_005b1f18,s_ERROR_005b1f10,0);
    local_8 = 0;
  }
  else {
    gethostname(local_108,0x100);
    local_118.sa_family = 2;
    local_118.sa_data._2_4_ = param_3;
    local_118.sa_data._0_2_ = htons(param_4);
    iVar1 = bind(local_8,&local_118,0x10);
    if (iVar1 == -1) {
      MessageBoxA(param_2,s_Binding_fail_005b1f38,s_ERROR_005b1f30,0);
      closesocket(local_8);
      local_8 = 0;
    }
    else {
      iVar1 = listen(local_8,8);
      if (iVar1 < 0) {
        MessageBoxA(param_2,s_Listen_fail_005b1f50,s_ERROR_005b1f48,0);
        closesocket(local_8);
        local_8 = 0;
      }
      else {
        iVar1 = WSAAsyncSelect(local_8,param_2,param_5,8);
        if (iVar1 < 1) {
          *param_1 = local_8;
        }
        else {
          MessageBoxA(param_2,s_WSAAsyncSelect_fail_005b1f64,s_ERROR_005b1f5c,0);
          closesocket(local_8);
          local_8 = 0;
        }
      }
    }
  }
  return local_8;
}

