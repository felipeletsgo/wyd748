// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005835c5 | Name: FUN_005835c5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ulonglong *
FUN_005835c5(ulonglong *param_1,ulonglong *param_2,undefined8 *param_3,undefined1 *param_4,
            undefined1 *param_5,undefined1 *param_6)

{
  ulonglong uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined4 uVar6;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 in_MM5;
  undefined8 uVar7;
  undefined8 in_MM6;
  undefined1 local_44 [64];
  
  switch(((param_6 != (undefined1 *)0x0) << 1 | param_5 != (undefined1 *)0x0) << 1 |
         param_4 != (undefined1 *)0x0) {
  case '\0':
    FUN_0057f592(local_44);
    goto switchD_005835fb_default;
  case '\x01':
    param_6 = param_4;
    goto LAB_0058363f;
  case '\x02':
    param_6 = param_5;
    goto LAB_0058363f;
  case '\x03':
    param_6 = param_5;
    break;
  case '\x04':
    goto LAB_0058363f;
  case '\x05':
    break;
  case '\x06':
    param_4 = param_5;
    break;
  case '\a':
    FUN_0058304b(local_44,param_6,param_5);
    param_6 = local_44;
    break;
  default:
    goto switchD_005835fb_default;
  }
  FUN_0058304b(local_44,param_6,param_4);
  param_6 = local_44;
LAB_0058363f:
  FUN_00582230(local_44,0,param_6);
switchD_005835fb_default:
  if (param_3 != (undefined8 *)0x0) {
    FastExitMediaState();
    uVar2 = PackedIntToFloatingDwordConv(*param_3,*param_3);
    uVar5 = param_3[2];
    uVar3 = PackedIntToFloatingDwordConv(param_3[1],param_3[1]);
    uVar6 = (undefined4)((ulonglong)uVar3 >> 0x20);
    uVar4 = CONCAT44(uVar6,uVar6);
    uVar7 = FloatingReciprocalAprox(in_MM5,uVar3);
    uVar4 = FloatingReciprocalAprox(uVar4,uVar4);
    uVar2 = PackedFloatingSUB(*param_2,uVar2);
    uVar4 = CONCAT44((int)uVar4,(int)uVar7);
    uVar3 = PackedFloatingReciprocalIter1(uVar3,uVar4);
    uVar3 = PackedFloatingReciprocalIter2(uVar3,uVar4);
    uVar3 = PackedFloatingMUL(uVar3,DAT_005a8738);
    uVar2 = PackedFloatingMUL(uVar2,uVar3);
    uVar6 = (undefined4)((ulonglong)uVar5 >> 0x20);
    uVar1 = PackedFloatingSUB(uVar2,_DAT_005a8728);
    uVar2 = PackedFloatingSUB(CONCAT44(uVar6,uVar6),uVar5);
    uVar3 = FloatingReciprocalAprox(in_MM6,uVar2);
    uVar2 = PackedFloatingReciprocalIter1(uVar2,uVar3);
    uVar2 = PackedFloatingReciprocalIter2(uVar2,uVar3);
    uVar5 = PackedFloatingSUB((ulonglong)(uint)param_2[1],uVar5);
    uVar5 = PackedFloatingMUL(uVar5,uVar2);
    *param_1 = uVar1 ^ DAT_005a8720;
    *(int *)(param_1 + 1) = (int)uVar5;
    FastExitMediaState();
    param_2 = param_1;
  }
  FUN_00582fc0(param_1,param_2,local_44);
  return param_1;
}

