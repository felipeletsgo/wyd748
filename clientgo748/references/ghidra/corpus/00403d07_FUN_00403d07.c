// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403d07 | Name: FUN_00403d07


void __fastcall FUN_00403d07(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059e9fe;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3708;
  local_8 = 0;
  if (param_1[0x7a] != 0) {
    if ((undefined4 *)param_1[0x7a] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x7a])(1);
    }
    param_1[0x7a] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0040151a();
  ExceptionList = local_10;
  return;
}

