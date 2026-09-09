// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059aff6 | Name: FUN_0059aff6


void FUN_0059aff6(int param_1,int *param_2,ushort *param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_5c [40];
  undefined8 local_34;
  uint local_24;
  
  param_3 = (ushort *)(uint)*param_3;
  iVar1 = *param_2;
  if (iVar1 == 1) {
LAB_0059b03b:
    uVar2 = 8;
  }
  else if (iVar1 == 2) {
    uVar2 = 4;
  }
  else if (iVar1 == 3) {
    uVar2 = 0x11;
  }
  else if (iVar1 == 4) {
    uVar2 = 0x12;
  }
  else {
    if (iVar1 == 5) goto LAB_0059b03b;
    if (iVar1 == 7) {
      *param_2 = 1;
      goto LAB_0059b091;
    }
    if (iVar1 != 8) goto LAB_0059b091;
    uVar2 = 0x10;
  }
  iVar1 = FUN_00598533(uVar2,param_2 + 6,param_3);
  if (iVar1 == 0) {
    if (((param_1 == 0x10) || (param_1 == 0x16)) || (param_1 == 0x1d)) {
      local_34 = *(undefined8 *)(param_2 + 4);
      local_24 = local_24 & 0xffffffe3 | 3;
    }
    else {
      local_24 = local_24 & 0xfffffffe;
    }
    FUN_00598280(local_5c,&param_3,uVar2,param_1,param_2 + 2,param_2 + 6);
  }
LAB_0059b091:
  FUN_005989ad(param_3,0xffff);
  if (((*param_2 != 8) && (DAT_005cc790 == 0)) && (iVar1 = FUN_005840ee(param_2), iVar1 != 0)) {
    return;
  }
  FUN_005987d2(*param_2);
  return;
}

