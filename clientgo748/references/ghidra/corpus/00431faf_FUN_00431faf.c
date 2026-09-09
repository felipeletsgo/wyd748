// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431faf | Name: FUN_00431faf


void __fastcall FUN_00431faf(int param_1)

{
  size_t sVar1;
  undefined4 uVar2;
  void *pvVar3;
  undefined4 local_48;
  int local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f10c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  sVar1 = _strlen(&DAT_005b4b4c);
  _memset((void *)(param_1 + 0x2ad78),0,64000);
  uVar2 = FUN_0058f716(s_UI_Ending_bin_005b4bfc,&DAT_005b4bf8);
  FUN_00590d68(param_1 + 0x2ad78,64000,1,uVar2);
  FUN_0058f62c(uVar2);
  for (local_24 = 0; local_24 < 64000; local_24 = local_24 + 1) {
    *(char *)(param_1 + 0x2ad78 + local_24) =
         *(char *)(param_1 + 0x2ad78 + local_24) -
         ((char)local_24 + (&DAT_005b4b4c)[local_24 % (int)sVar1]);
  }
  for (local_24 = 0; local_24 < 500; local_24 = local_24 + 1) {
    sVar1 = _strlen((char *)(param_1 + 0x2ad78 + local_24 * 0x80));
    if (1 < (int)sVar1) {
      pvVar3 = operator_new(0xe4c);
      local_8 = 0;
      if (pvVar3 == (void *)0x0) {
        local_48 = 0;
      }
      else {
        local_48 = FUN_00401d03(0xfffffffe,param_1 + 0x2ad78 + local_24 * 0x80,0xffffffff,0,
                                (float)local_24 * 20.0,0x44480000,0x41a00000,1,0,1,1);
      }
      local_8 = 0xffffffff;
      FUN_0054ac09(local_48);
    }
  }
  ExceptionList = local_10;
  return;
}

