// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004075e6 | Name: FUN_004075e6


void __fastcall FUN_004075e6(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ebfb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3aa8;
  local_8 = 0;
  if (param_1[0x396] != 0) {
    if ((undefined4 *)param_1[0x396] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x396])(1);
    }
    param_1[0x396] = 0;
  }
  if (param_1[0x395] != 0) {
    if ((undefined4 *)param_1[0x395] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x395])(1);
    }
    param_1[0x395] = 0;
  }
  if (param_1[0x394] != 0) {
    if ((undefined4 *)param_1[0x394] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x394])(1);
    }
    param_1[0x394] = 0;
  }
  local_8 = 0xffffffff;
  FUN_00407265();
  ExceptionList = local_10;
  return;
}

