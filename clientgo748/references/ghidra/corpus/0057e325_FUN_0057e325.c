// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057e325 | Name: FUN_0057e325


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057e325(undefined8 *param_1,undefined8 *param_2)

{
  undefined4 uVar1;
  ulonglong uVar2;
  undefined8 uVar3;
  ulonglong uVar4;
  undefined8 uVar5;
  undefined8 in_MM4;
  undefined8 uVar6;
  
  FastExitMediaState();
  uVar3 = *param_2;
  uVar4 = param_2[1];
  uVar1 = (undefined4)(uVar4 >> 0x20);
  uVar5 = PackedFloatingCompareGE(CONCAT44(uVar1,uVar1),_DAT_005a8728);
  if ((int)uVar5 == 0) {
    uVar1 = FUN_0058a980();
    uVar2 = FUN_0058b060();
    uVar5 = PackedFloatingCompareGE(uVar2 & _DAT_005a86e0,DAT_005a86e8);
    uVar3 = *param_2;
    uVar4 = param_2[1];
    if ((int)uVar5 != 0) {
      uVar6 = FloatingReciprocalAprox(in_MM4,uVar2);
      uVar5 = PackedFloatingReciprocalIter1(uVar2,uVar6);
      uVar5 = PackedFloatingReciprocalIter2(uVar5,uVar6);
      uVar5 = PackedFloatingMUL(CONCAT44(uVar1,uVar1),uVar5);
      uVar3 = PackedFloatingMUL(uVar3,uVar5);
      uVar4 = PackedFloatingMUL(uVar4,uVar5);
    }
  }
  uVar4 = uVar4 & _DAT_005a86f0;
  *param_1 = uVar3;
  param_1[1] = uVar4;
  FastExitMediaState();
  return;
}

