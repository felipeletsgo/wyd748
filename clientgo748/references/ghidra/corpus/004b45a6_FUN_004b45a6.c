// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b45a6 | Name: FUN_004b45a6


undefined4 FUN_004b45a6(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  HANDLE hTargetProcessHandle;
  HANDLE hSourceHandle;
  HANDLE hSourceProcessHandle;
  HANDLE *lpTargetHandle;
  DWORD dwDesiredAccess;
  BOOL bInheritHandle;
  DWORD dwOptions;
  undefined1 local_410 [8];
  undefined1 local_408 [1024];
  HANDLE local_8;
  
  uVar1 = FUN_0058f716(s_Error_log_005b9f10,&DAT_005b9f0c);
  FUN_0058f078(local_410,&DAT_005b9f1c,uVar1);
  uVar2 = FUN_004b4931(*param_1);
  uVar2 = FUN_004b4692(*(undefined4 *)*param_1,uVar2);
  FUN_0058f078(local_408,s__s_s_EC_0x_8_8X_EA_0x_8_8X_Ver___005b9f2c,&DAT_005b9f24,local_410,
               *(undefined4 *)*param_1,*(undefined4 *)(*param_1 + 0xc),0x2ec,uVar2);
  FUN_0058f6a9(uVar1,s_ERROR__s_005b9f58,local_408);
  dwOptions = 2;
  bInheritHandle = 0;
  dwDesiredAccess = 0;
  lpTargetHandle = &local_8;
  hTargetProcessHandle = GetCurrentProcess();
  hSourceHandle = GetCurrentThread();
  hSourceProcessHandle = GetCurrentProcess();
  DuplicateHandle(hSourceProcessHandle,hSourceHandle,hTargetProcessHandle,lpTargetHandle,
                  dwDesiredAccess,bInheritHandle,dwOptions);
  FUN_004b4ac8(local_8,param_1[1],uVar1);
  return param_2;
}

