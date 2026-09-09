// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00534ebe | Name: FUN_00534ebe


undefined4 * __thiscall FUN_00534ebe(int param_1,undefined4 *param_2,int param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  undefined4 uVar6;
  undefined1 *puVar7;
  undefined4 uVar8;
  undefined1 *puVar9;
  undefined4 uVar10;
  undefined1 *puVar11;
  undefined4 uVar12;
  undefined1 local_134 [12];
  undefined1 local_128 [12];
  undefined1 local_11c [12];
  undefined1 local_110 [12];
  undefined1 local_104 [24];
  undefined1 local_ec [24];
  undefined1 local_d4 [24];
  undefined1 local_bc [72];
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  float local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 local_34 [12];
  undefined1 local_28 [12];
  undefined1 local_1c [12];
  undefined1 local_10 [12];
  
  FUN_00430f50(0,0x3f800000,0);
  FUN_00421910(local_34,0xc,4,CSimpleArray<>);
  FUN_00421910(&local_64,0xc,4,CSimpleArray<>);
  local_68 = (float)(int)*(char *)(param_1 + 0xcc + (param_3 + param_4 * 0x40) * 0xc);
  if ((((0 < param_3) && (param_3 < 0x40)) && (0 < param_4)) && (param_4 < 0x40)) {
    uVar1 = FUN_00430f50(0xbf800000,
                         (float)(int)*(char *)(param_1 + 0xcc +
                                              (param_3 + -1 + param_4 * 0x40) * 0xc),0);
    FUN_004310a0(uVar1);
    uVar1 = FUN_00430f50(0,(float)(int)*(char *)(param_1 + 0xcc +
                                                (param_3 + (param_4 + 1) * 0x40) * 0xc),0x3f800000);
    FUN_004310a0(uVar1);
    uVar1 = FUN_00430f50(0x3f800000,
                         (float)(int)*(char *)(param_1 + 0xcc + (param_3 + 1 + param_4 * 0x40) * 0xc
                                              ),0);
    FUN_004310a0(uVar1);
    uVar1 = FUN_00430f50(0,(float)(int)*(char *)(param_1 + 0xcc +
                                                (param_3 + (param_4 + -1) * 0x40) * 0xc),0xbf800000)
    ;
    FUN_004310a0(uVar1);
    uVar1 = local_64;
    uVar4 = local_60;
    uVar6 = local_5c;
    uVar8 = local_58;
    uVar10 = local_54;
    uVar12 = local_50;
    puVar2 = (undefined4 *)FUN_00430f50(0,local_68,0);
    uVar1 = FUN_004b1ce6(local_bc,*puVar2,puVar2[1],puVar2[2],uVar1,uVar4,uVar6,uVar8,uVar10,uVar12)
    ;
    FUN_004310a0(uVar1);
    uVar1 = local_4c;
    uVar4 = local_48;
    uVar6 = local_44;
    puVar2 = (undefined4 *)FUN_00430f50(0,local_68,0);
    uVar1 = FUN_004b1ce6(local_d4,*puVar2,puVar2[1],puVar2[2],local_58,local_54,local_50,uVar1,uVar4
                         ,uVar6);
    FUN_004310a0(uVar1);
    uVar1 = local_40;
    uVar4 = local_3c;
    uVar6 = local_38;
    puVar2 = (undefined4 *)FUN_00430f50(0,local_68,0);
    uVar1 = FUN_004b1ce6(local_ec,*puVar2,puVar2[1],puVar2[2],local_4c,local_48,local_44,uVar1,uVar4
                         ,uVar6);
    FUN_004310a0(uVar1);
    puVar2 = (undefined4 *)FUN_00430f50(0,local_68,0);
    uVar1 = FUN_004b1ce6(local_104,*puVar2,puVar2[1],puVar2[2],local_40,local_3c,local_38,local_64,
                         local_60,local_5c);
    FUN_004310a0(uVar1);
    uVar1 = 0x40800000;
    puVar11 = local_134;
    puVar9 = local_10;
    puVar7 = local_128;
    puVar5 = local_1c;
    puVar3 = local_11c;
    FUN_00493d50(local_110,local_28);
    FUN_00493d50(puVar3,puVar5);
    FUN_00493d50(puVar7,puVar9);
    uVar1 = FUN_00493e10(puVar11,uVar1);
    FUN_004310a0(uVar1);
  }
  *param_2 = local_74;
  param_2[1] = local_70;
  param_2[2] = local_6c;
  return param_2;
}

