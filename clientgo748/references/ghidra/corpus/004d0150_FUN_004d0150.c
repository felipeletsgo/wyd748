// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d0150 | Name: FUN_004d0150


void FUN_004d0150(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 local_1c [8];
  float local_14;
  float local_10;
  double local_c;
  
  FUN_00430f10();
  uVar1 = FUN_00493c60(local_1c,param_1);
  FUN_0040bd30(uVar1);
  local_c = (double)(local_10 * local_10 + local_14 * local_14);
  FUN_00591614(local_c);
  return;
}

