// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055f7f9 | Name: FUN_0055f7f9


undefined4 FUN_0055f7f9(undefined4 param_1,undefined4 param_2,char *param_3)

{
  char cVar1;
  void *pvVar2;
  int iVar3;
  HWND pHVar4;
  undefined4 uVar5;
  HRESULT HVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int *local_228;
  uint local_210;
  undefined1 local_208 [256];
  CHAR local_108 [256];
  LPSTR local_8;
  
  local_8 = GetCommandLineA();
  cVar1 = FUN_00423b25();
  if (cVar1 == '\0') {
    FUN_0058f078(&DAT_005c5034,&DAT_005c503c);
  }
  else {
    FUN_0058f078(&DAT_005c5034,&DAT_005c5038);
  }
  FUN_004b411c(0,1,0);
  pvVar2 = operator_new(0x114);
  if (pvVar2 == (void *)0x0) {
    local_228 = (int *)0x0;
  }
  else {
    local_228 = (int *)FUN_0055b18f();
  }
  _memset(&DAT_013b7354,0,0x10);
  _memset(&DAT_013b733c,0,0x18);
  if ((param_3 != (char *)0x0) && (*param_3 != '\0')) {
    iVar3 = FUN_0055ad65(param_3,local_208);
    if (iVar3 == 0) {
      MessageBoxA((HWND)0x0,s_Login_Failed__005c5058,&DAT_005c5054,0);
      return 0;
    }
    iVar3 = FUN_0055adf6(local_208,&DAT_013b723c);
    if (iVar3 == 0) {
      MessageBoxA((HWND)0x0,s_Login_Failed__005c5044,&DAT_005c5040,0);
      return 0;
    }
  }
  pHVar4 = FindWindowA((LPCSTR)0x0,s_Wxth_Your_Destiny_005c5068);
  if (pHVar4 == (HWND)0x0) {
    _memset(local_108,0,0x100);
    GetKeyboardLayoutNameA(local_108);
    HVar6 = CoInitialize((LPVOID)0x0);
    if (HVar6 < 0) {
      FUN_005926b9(1);
    }
    HVar6 = OleInitialize((LPVOID)0x0);
    if (HVar6 < 0) {
      FUN_005926b9(1);
    }
    iVar3 = _strcmp(param_3,&DAT_005c507c);
    local_210 = (uint)(iVar3 != 0);
    iVar3 = _strcmp(param_3,&DAT_005c5080);
    if (iVar3 == 0) {
      local_210 = 1;
    }
    uVar8 = param_1;
    iVar3 = (**(code **)(*local_228 + 4))(param_1,local_210);
    if (iVar3 == 0) {
      uVar5 = 0;
    }
    else {
      FUN_00423c1f(param_1);
      uVar5 = (**(code **)(*local_228 + 0xc))();
      OleUninitialize();
      CoUninitialize();
      if (local_228 == (int *)0x0) {
        uVar7 = 0;
      }
      else {
        uVar7 = (**(code **)*local_228)(1);
      }
      FUN_004b428e(uVar8,local_210,uVar7);
    }
  }
  else {
    uVar5 = 0;
  }
  return uVar5;
}

