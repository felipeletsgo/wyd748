// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b5d16 | Name: FUN_004b5d16


bool FUN_004b5d16(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  bool bVar2;
  undefined4 local_280 [5];
  undefined4 local_26c;
  undefined4 local_268;
  char local_260 [256];
  char local_160 [260];
  char *local_5c [2];
  HMODULE local_54;
  FARPROC local_50;
  HANDLE local_4c;
  uint local_48;
  FARPROC local_44;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_40 [16];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_30 [16];
  undefined4 local_20;
  undefined4 local_1c;
  uint local_18;
  FARPROC local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a058d;
  local_10 = ExceptionList;
  local_5c[0] = s_kernel32_dll_005ba580;
  local_5c[1] = s_tlhelp32_dll_005ba590;
  local_280[0] = 0x224;
  ExceptionList = &local_10;
  FUN_004b61c0();
  local_8 = 0;
  for (local_48 = 0; local_48 < 2; local_48 = local_48 + 1) {
    local_54 = LoadLibraryA(local_5c[local_48]);
    if (local_54 != (HMODULE)0x0) {
      local_44 = GetProcAddress(local_54,s_CreateToolhelp32Snapshot_005ba5a0);
      local_50 = GetProcAddress(local_54,s_Module32First_005ba5bc);
      local_14 = GetProcAddress(local_54,s_Module32Next_005ba5cc);
      if (((local_44 != (FARPROC)0x0) && (local_50 != (FARPROC)0x0)) && (local_14 != (FARPROC)0x0))
      break;
      FreeLibrary(local_54);
      local_54 = (HMODULE)0x0;
    }
  }
  if (local_54 == (HMODULE)0x0) {
    local_8 = 0xffffffff;
    FUN_004b6220();
    bVar2 = false;
  }
  else {
    local_4c = (HANDLE)(*local_44)(8,param_2);
    if (local_4c == (HANDLE)0xffffffff) {
      local_8 = 0xffffffff;
      FUN_004b6220();
      bVar2 = false;
    }
    else {
      iVar1 = (*local_50)(local_4c,local_280);
      local_18 = CONCAT31(local_18._1_3_,iVar1 != 0);
      while ((local_18 & 0xff) != 0) {
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
                  (local_40,local_160);
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
                  (local_30,local_260);
        local_20 = local_26c;
        local_1c = local_268;
        FUN_004b65a0(local_40);
        iVar1 = (*local_14)(local_4c,local_280);
        local_18 = CONCAT31(local_18._1_3_,iVar1 != 0);
      }
      CloseHandle(local_4c);
      FreeLibrary(local_54);
      iVar1 = FUN_004b6560();
      bVar2 = iVar1 != 0;
      local_8 = 0xffffffff;
      FUN_004b6220();
    }
  }
  ExceptionList = local_10;
  return bVar2;
}

