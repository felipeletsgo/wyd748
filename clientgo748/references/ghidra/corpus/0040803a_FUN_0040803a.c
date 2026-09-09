// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040803a | Name: FUN_0040803a


void __fastcall FUN_0040803a(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059eca8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3bc8;
  local_8 = 0;
  if (param_1[0x394] != 0) {
    if ((undefined4 *)param_1[0x394] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x394])(1);
    }
    param_1[0x394] = 0;
  }
  if (param_1[0x397] != 0) {
    if ((undefined4 *)param_1[0x397] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x397])(1);
    }
    param_1[0x397] = 0;
  }
  if (param_1[0x395] != 0) {
    if ((undefined4 *)param_1[0x395] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x395])(1);
    }
    param_1[0x395] = 0;
  }
  if (param_1[0x396] != 0) {
    if ((undefined4 *)param_1[0x396] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x396])(1);
    }
    param_1[0x396] = 0;
  }
  local_8 = 0xffffffff;
  FUN_00407265();
  ExceptionList = local_10;
  return;
}

