// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b411c | Name: FUN_004b411c


void FUN_004b411c(int param_1,int param_2,undefined4 param_3)

{
  DWORD DVar1;
  CHAR local_108 [260];
  
  DVar1 = GetModuleFileNameA((HMODULE)0x0,local_108,0x104);
  if (DVar1 == 0) {
    if (param_1 == 2) {
      FUN_0058ee20(local_108,s__mem_leaks_xml_leaks_005b9df8);
    }
    else {
      FUN_0058ee20(local_108,s__mem_leaks_log_005b9e10);
    }
  }
  else {
    FUN_0058ee20(s__exceptions_log_005b9b64,local_108);
    if (param_1 == 2) {
      FUN_0058ee30(s__exceptions_log_005b9b64,s__exp_xml_005b9dc4);
      FUN_0058ee30(local_108,s__mem_xml_leaks_005b9ddc);
    }
    else {
      FUN_0058ee30(s__exceptions_log_005b9b64,s__exp_log_005b9dd0);
      FUN_0058ee30(local_108,s__mem_log_005b9dec);
    }
  }
  if ((param_2 != 0) && (DAT_0092e628 == 0)) {
    SetUnhandledExceptionFilter(FUN_004b421e);
    DAT_0092e628 = 1;
  }
  FUN_004b3eb2(param_1,local_108,param_3);
  return;
}

