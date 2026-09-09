// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b222b | Name: FUN_004b222b


undefined4 __fastcall FUN_004b222b(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 0;
  _memset((void *)(param_1 + 0xf48),0,0x1a000);
  uVar1 = FUN_0058f716(s_UI_RC_bin_005b9a70,&DAT_005b9a6c,param_1,uVar1,0);
  FUN_00590d68(param_1 + 0xf48,0x1a000,1,uVar1);
  FUN_0058f62c(uVar1);
  return 0;
}

