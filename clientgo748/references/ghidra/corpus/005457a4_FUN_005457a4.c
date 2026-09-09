// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005457a4 | Name: FUN_005457a4


void __fastcall FUN_005457a4(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a29c9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5afc;
  local_8 = 0;
  if (param_1[0x5f] != 0) {
    if ((undefined4 *)param_1[0x5f] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x5f])(1);
    }
    param_1[0x5f] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

