// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a2a3 | Name: FUN_0055a2a3


void FUN_0055a2a3(undefined4 param_1,int param_2)

{
  int local_8;
  
  _memset(&DAT_00e37e10,0,0x10);
  if (param_2 < 0x11) {
    local_8 = param_2;
  }
  else {
    local_8 = 0x10;
  }
  FUN_0058f220(&DAT_00e37e10,param_1,local_8);
  FUN_0055a483(&DAT_00e37f24,&DAT_00e37e10);
  if (8 < param_2) {
    FUN_0055a483(&DAT_00e38224,&DAT_00e37e18);
  }
  DAT_00e37f20 = 8 < param_2;
  return;
}

