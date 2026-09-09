// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00557943 | Name: FUN_00557943


undefined4 FUN_00557943(void)

{
  undefined4 uVar1;
  int iVar2;
  int local_110;
  int local_10c;
  undefined1 local_108 [256];
  undefined4 local_8;
  
  local_10c = 0;
  local_8 = 0;
  FUN_0058ee20(local_108,s___Env_AttributeMap_dat_005c47d0);
  local_110 = FUN_0058f716(local_108,&DAT_005c47e8);
  if (local_110 == 0) {
    local_110 = FUN_0058f716(s_______TMSRV_Run_AttributeMap_dat_005c47f0,&DAT_005c47ec);
  }
  if (local_110 == 0) {
    MessageBoxA((HWND)0x0,s_There_is_no_file_005c4828,s_Attributemap_dat_005c4814,0);
    uVar1 = 0;
  }
  else {
    FUN_00590d68(&DAT_00934178,0x400,0x400,local_110);
    FUN_00590d68(&local_10c,4,1,local_110);
    FUN_0058f62c(local_110);
    iVar2 = FUN_0054f2d9(&DAT_00934178,0x100000);
    if (iVar2 == local_10c) {
      uVar1 = 1;
    }
    else {
      uVar1 = 0;
    }
  }
  return uVar1;
}

