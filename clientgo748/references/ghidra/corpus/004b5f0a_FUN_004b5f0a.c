// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b5f0a | Name: FUN_004b5f0a


bool FUN_004b5f0a(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FARPROC pFVar1;
  FARPROC pFVar2;
  void *pvVar3;
  char *pcVar4;
  int iVar5;
  DWORD DVar6;
  bool bVar7;
  uint local_60;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_54 [16];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_44 [16];
  undefined4 local_34;
  undefined4 local_30;
  uint local_2c;
  HMODULE local_28;
  FARPROC local_24;
  FARPROC local_20;
  undefined4 local_1c;
  undefined4 local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a05a0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004b61c0();
  local_8 = 0;
  local_28 = LoadLibraryA(s_psapi_dll_005ba5dc);
  if (local_28 == (HMODULE)0x0) {
    local_8 = 0xffffffff;
    FUN_004b6220();
    bVar7 = false;
  }
  else {
    FUN_004b65d0();
    pFVar1 = GetProcAddress(local_28,s_EnumProcessModules_005ba5e8);
    local_24 = GetProcAddress(local_28,s_GetModuleFileNameExA_005ba5fc);
    local_20 = GetProcAddress(local_28,s_GetModuleBaseNameA_005ba614);
    pFVar2 = GetProcAddress(local_28,s_GetModuleInformation_005ba628);
    if ((((pFVar1 == (FARPROC)0x0) || (local_24 == (FARPROC)0x0)) || (local_20 == (FARPROC)0x0)) ||
       (pFVar2 == (FARPROC)0x0)) {
      FreeLibrary(local_28);
      local_8 = 0xffffffff;
      FUN_004b6220();
      bVar7 = false;
    }
    else {
      pvVar3 = _malloc(0x1fa0);
      pcVar4 = _malloc(0x1fa0);
      iVar5 = (*pFVar1)(param_3,pvVar3,0x1fa0,&local_2c);
      if (iVar5 == 0) {
        DVar6 = GetLastError();
        FUN_0058f6a9(param_4,s__lu__EPM_failed__GetLastError_____005ba640,DAT_0092e5d4,DVar6);
      }
      else if (local_2c < 0x1fa1) {
        for (local_60 = 0; local_60 < local_2c >> 2; local_60 = local_60 + 1) {
          (*pFVar2)(param_3,*(undefined4 *)((int)pvVar3 + local_60 * 4),&local_1c,0xc);
          local_34 = local_1c;
          local_30 = local_18;
          *pcVar4 = '\0';
          (*local_24)(param_3,*(undefined4 *)((int)pvVar3 + local_60 * 4),pcVar4,0x1fa0);
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
                    (local_54,pcVar4);
          *pcVar4 = '\0';
          (*local_20)(param_3,*(undefined4 *)((int)pvVar3 + local_60 * 4),pcVar4,0x1fa0);
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
                    (local_44,pcVar4);
          FUN_004b65a0(local_54);
        }
      }
      else {
        FUN_0058f6a9(param_4,s__lu__More_than__lu_module_handle_005ba668,DAT_0092e5d4,1);
      }
      if (local_28 != (HMODULE)0x0) {
        FreeLibrary(local_28);
      }
      FUN_0058ffd8(pcVar4);
      FUN_0058ffd8(pvVar3);
      iVar5 = FUN_004b6560();
      bVar7 = iVar5 != 0;
      local_8 = 0xffffffff;
      FUN_004b6220();
    }
  }
  ExceptionList = local_10;
  return bVar7;
}

