// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b4364 | Name: FUN_004b4364


void FUN_004b4364(int param_1)

{
  DWORD DVar1;
  CHAR local_108 [260];
  
  if (DAT_0092e628 == 0) {
    DVar1 = GetModuleFileNameA((HMODULE)0x0,local_108,0x104);
    if (DVar1 == 0) {
      if (param_1 == 2) {
        FUN_0058ee20(local_108,s__mem_leaks_xml_leaks_005b9ebc);
      }
      else {
        FUN_0058ee20(local_108,s__mem_leaks_log_005b9ed4);
      }
    }
    else {
      FUN_0058ee20(s__exceptions_log_005b9b64,local_108);
      if (param_1 == 2) {
        FUN_0058ee30(s__exceptions_log_005b9b64,s__exp_xml_005b9e88);
        FUN_0058ee30(local_108,s__mem_xml_leaks_005b9ea0);
      }
      else {
        FUN_0058ee30(s__exceptions_log_005b9b64,s__exp_log_005b9e94);
        FUN_0058ee30(local_108,s__mem_log_005b9eb0);
      }
    }
    DAT_0092e588 = param_1;
    SetUnhandledExceptionFilter(FUN_004b421e);
    DAT_0092e628 = 1;
  }
  return;
}

