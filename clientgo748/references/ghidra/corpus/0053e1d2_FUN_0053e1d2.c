// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053e1d2 | Name: FUN_0053e1d2


undefined4 * __fastcall FUN_0053e1d2(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a26b9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00430f10();
  CSimpleArray<>();
  FUN_00421910(param_1 + 0x19,0xc,10,CSimpleArray<>);
  FUN_00421910(param_1 + 0x37,0xc,10,CSimpleArray<>);
  *param_1 = &PTR_FUN_005a5a00;
  FUN_0053e324();
  ExceptionList = local_10;
  return param_1;
}

