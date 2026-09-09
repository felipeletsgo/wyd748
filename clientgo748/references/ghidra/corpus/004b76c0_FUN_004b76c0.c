// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b76c0 | Name: FUN_004b76c0


int __thiscall FUN_004b76c0(int param_1,int param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0659;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004b62a0(param_2);
  local_8 = 0;
  FUN_004b62a0(param_2 + 0x10);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_2 + 0x20);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_2 + 0x24);
  ExceptionList = local_10;
  return param_1;
}

