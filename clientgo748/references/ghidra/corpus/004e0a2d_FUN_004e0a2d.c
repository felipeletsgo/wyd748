// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0a2d | Name: FUN_004e0a2d


void __fastcall FUN_004e0a2d(undefined4 *param_1)

{
  void *pvVar1;
  int local_24;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_005a0da7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4d44;
  local_8 = 0;
  pvVar1 = operator_new(0x9c);
  local_8._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    local_24 = 0;
  }
  else {
    local_24 = FUN_004e2360(param_1[0x1d],param_1[0x1e],param_1[0x1f],0,0x3f000000,0xfa,0xffffffff);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_24 != 0) {
    FUN_0054ac09(local_24);
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

