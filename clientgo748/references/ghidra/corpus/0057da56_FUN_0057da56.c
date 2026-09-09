// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057da56 | Name: FUN_0057da56


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057da56(undefined8 *param_1,undefined8 *param_2,undefined4 *param_3)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  
  uVar3 = _DAT_005a8690;
  uVar4 = *param_1;
  uVar1 = param_1[1];
  uVar5 = PackedFloatingMUL(uVar4,uVar4);
  uVar2 = PackedFloatingMUL(uVar1,uVar1);
  uVar5 = PackedFloatingADD(uVar5,uVar2);
  uVar5 = PackedFloatingAccumulate(uVar5,uVar5);
  uVar6 = PackedFloatingCompareGE(_PTR_DAT_005a8710,uVar5);
  uVar2 = PackedFloatingReciprocalSQRAprox(uVar2,uVar5);
  if ((int)uVar6 == 0) {
    uVar3 = PackedFloatingMUL(uVar2,uVar2);
    if (param_2 != (undefined8 *)0x0) {
      uVar3 = PackedFloatingReciprocalSQRIter1(uVar3,uVar5);
      uVar3 = PackedFloatingReciprocalIter2(uVar3,uVar2);
      uVar2 = PackedFloatingMUL(uVar4,uVar3);
      uVar4 = PackedFloatingMUL(uVar3,uVar1);
      *param_2 = uVar2;
      *(int *)(param_2 + 1) = (int)uVar4;
    }
    if (param_3 != (undefined4 *)0x0) {
      uVar4 = FUN_0058a980();
      uVar4 = PackedFloatingMUL(uVar4,_DAT_005a86a8);
      *param_3 = (int)uVar4;
    }
  }
  else {
    if (param_2 != (undefined8 *)0x0) {
      *(undefined8 *)((int)param_2 + 4) = 0;
      *(int *)param_2 = (int)uVar3;
    }
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = 0;
    }
  }
  FastExitMediaState();
  return;
}

