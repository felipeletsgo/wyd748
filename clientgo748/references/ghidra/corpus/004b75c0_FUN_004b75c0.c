// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b75c0 | Name: FUN_004b75c0


void FUN_004b75c0(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0641;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_004b7630(0x28,param_1);
  local_8 = 0;
  if (iVar1 != 0) {
    FUN_004b76c0(param_2);
  }
  ExceptionList = local_10;
  return;
}

