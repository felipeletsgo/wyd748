// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590a95 | Name: FUN_00590a95


undefined4 FUN_00590a95(undefined1 *param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  undefined1 *local_24;
  int local_20;
  undefined1 *local_1c;
  undefined4 local_18;
  
  local_1c = param_1;
  local_24 = param_1;
  local_20 = param_2;
  local_18 = 0x42;
  uVar1 = FUN_00594029(&local_24,param_3,param_4);
  local_20 = local_20 + -1;
  if (local_20 < 0) {
    FUN_00593f11(0,&local_24);
  }
  else {
    *local_24 = 0;
  }
  return uVar1;
}

