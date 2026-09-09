// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057dfff | Name: FUN_0057dfff


void FUN_0057dfff(undefined8 *param_1,undefined4 param_2,uint param_3)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined4 uVar4;
  undefined8 local_18;
  uint local_10;
  
  FastExitMediaState();
  FUN_00582d32(&local_18,param_2);
  PackedFloatingMUL((ulonglong)param_3,DAT_005a8730);
  uVar1 = FUN_0058af60();
  uVar4 = (undefined4)((ulonglong)uVar1 >> 0x20);
  uVar3 = CONCAT44(uVar4,uVar4);
  uVar2 = PackedFloatingMUL(local_18,uVar3);
  uVar3 = PackedFloatingMUL((ulonglong)local_10,uVar3);
  *param_1 = uVar2;
  param_1[1] = CONCAT44((int)uVar1,(int)uVar3);
  FastExitMediaState();
  return;
}

