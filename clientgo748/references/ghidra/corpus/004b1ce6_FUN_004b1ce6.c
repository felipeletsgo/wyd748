// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1ce6 | Name: FUN_004b1ce6


undefined4 * FUN_004b1ce6(undefined4 *param_1)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined1 local_34 [12];
  undefined1 local_28 [12];
  undefined1 local_1c [12];
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  CSimpleArray<>();
  FUN_00493db0(local_1c,&stack0x00000008);
  FUN_00493db0(local_28,&stack0x00000008);
  FUN_004b1c72(&local_10,local_1c,local_28);
  fVar2 = (float10)FUN_004b3aa0();
  uVar1 = FUN_00493e10(local_34,(float)fVar2);
  FUN_004310a0(uVar1);
  *param_1 = local_10;
  param_1[1] = local_c;
  param_1[2] = local_8;
  return param_1;
}

