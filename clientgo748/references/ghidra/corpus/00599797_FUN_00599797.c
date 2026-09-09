// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00599797 | Name: FUN_00599797


undefined4 FUN_00599797(undefined4 param_1)

{
  BYTE *pBVar1;
  byte *pbVar2;
  byte bVar3;
  byte bVar4;
  UINT CodePage;
  UINT *pUVar5;
  BOOL BVar6;
  uint uVar7;
  uint uVar8;
  BYTE *pBVar9;
  int iVar10;
  byte *pbVar11;
  int iVar12;
  byte *pbVar13;
  undefined4 uVar14;
  undefined4 *puVar15;
  _cpinfo local_1c;
  uint local_8;
  
  FUN_00596aad(0x19);
  CodePage = FUN_00599944(param_1);
  if (CodePage != DAT_013b83b4) {
    if (CodePage != 0) {
      iVar12 = 0;
      pUVar5 = &DAT_005cc568;
LAB_005997d4:
      if (*pUVar5 != CodePage) goto code_r0x005997d8;
      local_8 = 0;
      puVar15 = &DAT_013b84e0;
      for (iVar10 = 0x40; iVar10 != 0; iVar10 = iVar10 + -1) {
        *puVar15 = 0;
        puVar15 = puVar15 + 1;
      }
      iVar12 = iVar12 * 0x30;
      *(undefined1 *)puVar15 = 0;
      pbVar13 = (byte *)(iVar12 + 0x5cc578);
      do {
        bVar3 = *pbVar13;
        pbVar11 = pbVar13;
        while ((bVar3 != 0 && (bVar3 = pbVar11[1], bVar3 != 0))) {
          uVar8 = (uint)*pbVar11;
          if (uVar8 <= bVar3) {
            bVar4 = (&DAT_005cc560)[local_8];
            do {
              pbVar2 = (byte *)((int)&DAT_013b84e0 + uVar8 + 1);
              *pbVar2 = *pbVar2 | bVar4;
              uVar8 = uVar8 + 1;
            } while (uVar8 <= bVar3);
          }
          pbVar11 = pbVar11 + 2;
          bVar3 = *pbVar11;
        }
        local_8 = local_8 + 1;
        pbVar13 = pbVar13 + 8;
      } while (local_8 < 4);
      DAT_013b83cc = 1;
      DAT_013b83b4 = CodePage;
      DAT_013b85e4 = FUN_0059998e(CodePage);
      DAT_013b83c0 = *(undefined4 *)(iVar12 + 0x5cc56c);
      DAT_013b83c4 = *(undefined4 *)(iVar12 + 0x5cc570);
      DAT_013b83c8 = *(undefined4 *)(iVar12 + 0x5cc574);
      goto LAB_00599928;
    }
    goto LAB_00599923;
  }
  goto LAB_005997be;
code_r0x005997d8:
  pUVar5 = pUVar5 + 0xc;
  iVar12 = iVar12 + 1;
  if ((UINT *)0x5cc657 < pUVar5) goto code_r0x005997e3;
  goto LAB_005997d4;
code_r0x005997e3:
  BVar6 = GetCPInfo(CodePage,&local_1c);
  uVar8 = 1;
  if (BVar6 == 1) {
    DAT_013b85e4 = 0;
    puVar15 = &DAT_013b84e0;
    for (iVar12 = 0x40; iVar12 != 0; iVar12 = iVar12 + -1) {
      *puVar15 = 0;
      puVar15 = puVar15 + 1;
    }
    *(undefined1 *)puVar15 = 0;
    if (local_1c.MaxCharSize < 2) {
      DAT_013b83cc = 0;
      DAT_013b83b4 = CodePage;
    }
    else {
      DAT_013b83b4 = CodePage;
      if (local_1c.LeadByte[0] != '\0') {
        pBVar9 = local_1c.LeadByte + 1;
        do {
          bVar3 = *pBVar9;
          if (bVar3 == 0) break;
          for (uVar7 = (uint)pBVar9[-1]; uVar7 <= bVar3; uVar7 = uVar7 + 1) {
            pbVar13 = (byte *)((int)&DAT_013b84e0 + uVar7 + 1);
            *pbVar13 = *pbVar13 | 4;
          }
          pBVar1 = pBVar9 + 1;
          pBVar9 = pBVar9 + 2;
        } while (*pBVar1 != 0);
      }
      do {
        pbVar13 = (byte *)((int)&DAT_013b84e0 + uVar8 + 1);
        *pbVar13 = *pbVar13 | 8;
        uVar8 = uVar8 + 1;
      } while (uVar8 < 0xff);
      DAT_013b85e4 = FUN_0059998e(CodePage);
      DAT_013b83cc = 1;
    }
    DAT_013b83c0 = 0;
    DAT_013b83c4 = 0;
    DAT_013b83c8 = 0;
  }
  else {
    if (DAT_013b819c == 0) {
      uVar14 = 0xffffffff;
      goto LAB_00599935;
    }
LAB_00599923:
    FUN_005999c1();
  }
LAB_00599928:
  FUN_005999ea();
LAB_005997be:
  uVar14 = 0;
LAB_00599935:
  FUN_00596b0e(0x19);
  return uVar14;
}

