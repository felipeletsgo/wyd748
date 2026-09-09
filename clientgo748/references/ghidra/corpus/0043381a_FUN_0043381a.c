// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0043381a | Name: FUN_0043381a


void __fastcall FUN_0043381a(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0058f716(s_UI_TimeTable_bin_005b4ca8,&DAT_005b4ca4);
  _memset((void *)(param_1 + 0x2aa58),0,800);
  FUN_00590d68(param_1 + 0x27858,0x3200,1,uVar1);
  FUN_0058f62c(uVar1);
  return;
}

