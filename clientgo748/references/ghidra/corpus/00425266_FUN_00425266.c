// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00425266 | Name: FUN_00425266


undefined4 __fastcall FUN_00425266(SOCKET *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int local_108;
  undefined1 local_104 [256];
  
  if (*param_1 == 0) {
    param_1[3] = 0;
    param_1[6] = 0;
    uVar1 = 0;
  }
  else {
    if (0 < (int)param_1[6]) {
      FUN_00425200();
    }
    if (((int)param_1[3] < 0x20001) && (-1 < (int)param_1[3])) {
      if ((((int)param_1[3] < (int)param_1[6]) || (0x1ffff < (int)param_1[6])) ||
         ((int)param_1[6] < 0)) {
        FUN_0058f078(local_104,s_err_send2__d__d__d_005b20e8,param_1[3],param_1[6],*param_1);
        FUN_00423b20(local_104,s__system_005b20fc,0);
        param_1[3] = 0;
        param_1[6] = 0;
      }
      for (local_108 = 0; local_108 < 1; local_108 = local_108 + 1) {
        iVar2 = send(*param_1,(char *)(param_1[1] + param_1[6]),param_1[3] - param_1[6],0);
        if (iVar2 == -1) {
          WSAGetLastError();
        }
        else {
          param_1[6] = param_1[6] + iVar2;
        }
        if (((int)param_1[3] <= (int)param_1[6]) && (iVar2 != -1)) {
          param_1[3] = 0;
          param_1[6] = 0;
          return 1;
        }
      }
      if ((int)param_1[3] < 0x20000) {
        uVar1 = 1;
      }
      else {
        uVar1 = 0;
      }
    }
    else {
      FUN_0058f078(local_104,s_err_send1__d__d__d_005b20cc,param_1[3],param_1[6],*param_1);
      FUN_00423b20(local_104,s__system_005b20e0,0);
      param_1[3] = 0;
      param_1[6] = 0;
      uVar1 = 0;
    }
  }
  return uVar1;
}

