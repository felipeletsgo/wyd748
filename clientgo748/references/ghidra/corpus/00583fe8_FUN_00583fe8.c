// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00583fe8 | Name: FUN_00583fe8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00583fe8(undefined8 *param_1,ulonglong *param_2)

{
  ulonglong uVar1;
  undefined8 uVar2;
  ulonglong uVar3;
  undefined8 uVar4;
  undefined8 in_MM4;
  undefined8 uVar5;
  
  uVar1 = *param_2;
  uVar3 = param_2[1] & _DAT_005a86f0;
  uVar2 = PackedFloatingMUL(uVar1,uVar1);
  uVar4 = PackedFloatingMUL(uVar3,uVar3);
  uVar2 = PackedFloatingADD(uVar2,uVar4);
  uVar2 = PackedFloatingAccumulate(uVar2,uVar2);
  uVar5 = PackedFloatingReciprocalSQRAprox(in_MM4,uVar2);
  uVar3 = PackedFloatingCompareGT(uVar2,_PTR_DAT_005a8710);
  uVar4 = PackedFloatingMUL(uVar5,uVar5);
  uVar2 = PackedFloatingReciprocalSQRIter1(uVar2,uVar4);
  uVar2 = PackedFloatingReciprocalIter2(uVar2,uVar5);
  uVar4 = PackedFloatingMUL(uVar1 & uVar3,uVar2);
  uVar2 = PackedFloatingMUL(param_2[1] & uVar3,uVar2);
  *param_1 = uVar4;
  param_1[1] = uVar2;
  FastExitMediaState();
  return;
}

