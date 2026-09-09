// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005711e3 | Name: FUN_005711e3


/* WARNING: Removing unreachable block (ram,0x0057120d) */

void FUN_005711e3(void)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int unaff_EBP;
  
  FUN_00592900();
  *(undefined1 **)(unaff_EBP + -0x10) = &stack0xffffffc4;
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(undefined4 *)(unaff_EBP + -0x24) = s_GenuineIntel_005a6cb8._0_4_;
  *(undefined4 *)(unaff_EBP + -0x20) = s_GenuineIntel_005a6cb8._4_4_;
  *(undefined4 *)(unaff_EBP + -0x1c) = s_GenuineIntel_005a6cb8._8_4_;
  *(char *)(unaff_EBP + -0x18) = s_GenuineIntel_005a6cb8[0xc];
  *(undefined4 *)(unaff_EBP + -4) = 0;
  puVar1 = (undefined4 *)cpuid_basic_info(0);
  uVar4 = puVar1[1];
  uVar3 = puVar1[2];
  uVar2 = puVar1[3];
  *(undefined4 *)(unaff_EBP + -0x2c) = *puVar1;
  *(undefined4 *)(unaff_EBP + -0x38) = uVar4;
  *(undefined4 *)(unaff_EBP + -0x34) = uVar3;
  *(undefined4 *)(unaff_EBP + -0x30) = uVar2;
  FUN_00571229();
  return;
}

