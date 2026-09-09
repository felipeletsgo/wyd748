// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591dbd | Name: FUN_00591dbd


undefined4 FUN_00591dbd(undefined1 *param_1,int param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined1 *local_24;
  int local_20;
  undefined1 *local_1c;
  undefined4 local_18;
  
  local_1c = param_1;
  local_24 = param_1;
  local_18 = 0x42;
  local_20 = param_2;
  uVar1 = FUN_00594029(&local_24,param_3,&stack0x00000010);
  local_20 = local_20 + -1;
  if (local_20 < 0) {
    FUN_00593f11(0,&local_24);
  }
  else {
    *local_24 = 0;
  }
  return uVar1;
}

