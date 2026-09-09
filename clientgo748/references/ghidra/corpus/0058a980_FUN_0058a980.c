// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058a980 | Name: FUN_0058a980


ulonglong FUN_0058a980(void)

{
  uint uVar1;
  ulonglong in_MM0;
  ulonglong uVar2;
  undefined8 uVar3;
  uint uVar4;
  uint uVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  ulonglong uVar9;
  ulonglong uVar5;
  
  uVar2 = in_MM0 & DAT_005c871c;
  uVar6 = -(uint)(DAT_005c8728 < (int)uVar2);
  uVar3 = PackedFloatingMUL(uVar2,(ulonglong)DAT_005c872c);
  uVar3 = PackedFloatingSUBR(uVar3,(ulonglong)DAT_005c872c);
  uVar7 = PackedFloatingReciprocalSQRAprox((ulonglong)DAT_005c871c,uVar3);
  uVar8 = PackedFloatingMUL(uVar7,uVar7);
  uVar1 = (uint)((int)uVar2 < DAT_005c8760);
  uVar4 = -uVar1;
  uVar5 = (ulonglong)uVar4;
  uVar8 = PackedFloatingReciprocalSQRIter1(uVar8,uVar3);
  uVar7 = PackedFloatingReciprocalIter2(uVar8,uVar7);
  uVar9 = PackedFloatingMUL(uVar7,uVar3);
  uVar2 = ~uVar5 & uVar9 | uVar2 & uVar4;
  uVar3 = PackedFloatingMUL(uVar2,uVar2);
  uVar8 = PackedFloatingMUL((ulonglong)DAT_005c8768,uVar3);
  uVar7 = PackedFloatingADD((ulonglong)DAT_005c8770,uVar3);
  uVar8 = PackedFloatingADD(uVar8,(ulonglong)DAT_005c8764);
  uVar8 = PackedFloatingMUL(uVar8,uVar3);
  uVar3 = PackedFloatingMUL(uVar3,uVar7);
  uVar3 = PackedFloatingADD(uVar3,(ulonglong)DAT_005c876c);
  uVar8 = PackedFloatingMUL(uVar8,uVar2);
  uVar7 = FloatingReciprocalAprox(uVar7,uVar3);
  uVar3 = PackedFloatingReciprocalIter1(uVar3,uVar7);
  uVar3 = PackedFloatingReciprocalIter2(uVar3,uVar7);
  uVar3 = PackedFloatingMUL(uVar3,uVar8);
  uVar2 = PackedFloatingADD(uVar2,uVar3);
  uVar3 = PackedFloatingADD(~uVar5 & (ulonglong)DAT_005c8778 ^ DAT_005c8718 & in_MM0,
                            (ulonglong)DAT_005c8774);
  uVar2 = PackedFloatingADD(uVar2,~uVar5 & uVar2);
  uVar2 = PackedFloatingADD(uVar3,uVar2 | (ulonglong)(uVar1 * -0x80000000) ^ DAT_005c8718 & in_MM0);
  return (ulonglong)(DAT_005c8734 & uVar6) | ~(ulonglong)uVar6 & uVar2;
}

