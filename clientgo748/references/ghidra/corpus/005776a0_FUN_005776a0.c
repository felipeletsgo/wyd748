// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005776a0 | Name: FUN_005776a0


/* WARNING: Removing unreachable block (ram,0x0057771d) */
/* WARNING: Removing unreachable block (ram,0x0057770b) */
/* WARNING: Removing unreachable block (ram,0x005776f0) */
/* WARNING: Removing unreachable block (ram,0x005776d4) */
/* WARNING: Removing unreachable block (ram,0x005776c9) */

undefined8 FUN_005776a0(void)

{
  char cVar1;
  undefined4 *puVar2;
  uint *puVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  char *pcVar7;
  int *piVar8;
  char local_2c [4];
  char acStack_28 [4];
  char acStack_24 [4];
  char cStack_20;
  int local_1c;
  uint local_18;
  int local_14;
  char cStack_10;
  undefined4 local_c;
  uint local_8;
  
  cStack_10 = s_UnknownVendr_005a87b0[0xc];
  local_2c = (char  [4])s_AuthenticAMD_005a87a0._0_4_;
  acStack_28 = (char  [4])s_AuthenticAMD_005a87a0._4_4_;
  acStack_24 = (char  [4])s_AuthenticAMD_005a87a0._8_4_;
  cStack_20 = s_AuthenticAMD_005a87a0[0xc];
  cpuid_basic_info(0);
  local_8 = 1;
  piVar8 = (int *)cpuid_basic_info(0);
  local_18 = piVar8[2];
  local_1c = piVar8[1];
  local_14 = piVar8[3];
  if (*piVar8 != 0) {
    puVar2 = (undefined4 *)cpuid_Version_info(1);
    local_c = *puVar2;
    uVar5 = -(uint)((puVar2[2] & 0x800000) != 0) & 0x20;
    local_8 = uVar5 | 3;
    puVar3 = (uint *)cpuid(0x80000000);
    local_18 = puVar3[2];
    if (0x80000000 < *puVar3) {
      iVar6 = cpuid(0x80000001);
      local_18 = *(uint *)(iVar6 + 8);
      local_8 = uVar5 | 7 | -(uint)((local_18 & 0x80000000) != 0) & 0x80;
      iVar6 = 0xc;
      pcVar7 = local_2c;
      piVar8 = &local_1c;
      do {
        if (iVar6 == 0) break;
        iVar6 = iVar6 + -1;
        iVar4 = *piVar8;
        cVar1 = *pcVar7;
        pcVar7 = pcVar7 + 1;
        piVar8 = (int *)((int)piVar8 + 1);
      } while (cVar1 == (char)iVar4);
      local_8 = local_8 | -(uint)((local_18 & 0x40000000) != 0) & 0x100 |
                -(uint)((local_18 & 0x400000) != 0) & 0x200;
    }
  }
  return CONCAT44(local_18,local_8);
}

