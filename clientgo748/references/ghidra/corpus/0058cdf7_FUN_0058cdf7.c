// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058cdf7 | Name: FUN_0058cdf7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0058cdf7(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,int param_4,
                 int param_5,int param_6)

{
  undefined2 uVar1;
  int *piVar2;
  short sVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  short sVar8;
  short sVar9;
  short sVar11;
  undefined8 uVar7;
  int iVar10;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  short sVar17;
  int iVar18;
  int iVar19;
  int iVar20;
  short sVar24;
  short sVar25;
  short sVar29;
  ulonglong uVar21;
  int iVar26;
  int iVar27;
  ulonglong uVar22;
  ulonglong uVar23;
  int iVar28;
  int iVar30;
  int iVar31;
  int iVar32;
  int iVar33;
  ulonglong uVar34;
  int iVar38;
  int iVar39;
  ulonglong uVar35;
  ulonglong uVar36;
  int iVar40;
  ulonglong uVar37;
  short sVar41;
  int iVar42;
  int iVar43;
  int iVar44;
  short sVar50;
  undefined8 uVar45;
  short sVar49;
  int iVar51;
  int iVar52;
  short sVar54;
  ulonglong uVar47;
  int iVar53;
  undefined8 uVar48;
  int iVar55;
  int iVar56;
  int iVar57;
  undefined8 uVar58;
  int iVar60;
  int iVar61;
  int iVar62;
  undefined8 uVar59;
  short sVar63;
  int iVar64;
  short sVar67;
  short sVar68;
  undefined8 uVar65;
  int iVar69;
  short sVar70;
  int iVar71;
  int iVar72;
  ulonglong uVar73;
  int iVar74;
  int iVar75;
  undefined8 *local_18;
  undefined8 *local_14;
  undefined8 *local_10;
  int local_c;
  int local_8;
  ulonglong uVar46;
  ulonglong uVar66;
  
  local_18 = param_1;
  local_10 = param_2;
  local_8 = 2;
  local_14 = param_3;
  do {
    uVar7 = local_18[4];
    uVar45 = local_10[4];
    uVar58 = local_18[0xc];
    sVar3 = (short)uVar7 * (short)uVar45;
    sVar8 = (short)((ulonglong)uVar7 >> 0x10) * (short)((ulonglong)uVar45 >> 0x10);
    sVar9 = (short)((ulonglong)uVar7 >> 0x20) * (short)((ulonglong)uVar45 >> 0x20);
    sVar11 = (short)((ulonglong)uVar7 >> 0x30) * (short)((ulonglong)uVar45 >> 0x30);
    uVar37 = CONCAT26(sVar11,CONCAT24(sVar9,CONCAT22(sVar8,sVar3)));
    uVar7 = local_10[0xc];
    sVar17 = (short)uVar58 * (short)uVar7;
    sVar24 = (short)((ulonglong)uVar58 >> 0x10) * (short)((ulonglong)uVar7 >> 0x10);
    sVar25 = (short)((ulonglong)uVar58 >> 0x20) * (short)((ulonglong)uVar7 >> 0x20);
    sVar29 = (short)((ulonglong)uVar58 >> 0x30) * (short)((ulonglong)uVar7 >> 0x30);
    uVar34 = CONCAT26(sVar29,CONCAT24(sVar25,CONCAT22(sVar24,sVar17)));
    uVar45 = pmaddwd(uVar37,DAT_005a9ba0);
    uVar58 = pmaddwd(uVar37 >> 0x10,DAT_005a9ba0);
    uVar37 = CONCAT26(sVar11 + sVar29,
                      CONCAT24(sVar9 + sVar25,CONCAT22(sVar8 + sVar24,sVar3 + sVar17)));
    uVar21 = pmaddwd(uVar34,DAT_005a9bc0);
    uVar34 = pmaddwd(uVar34 >> 0x10,DAT_005a9bc0);
    uVar7 = pmaddwd(uVar37,DAT_005a9b98);
    uVar65 = pmaddwd(uVar37 >> 0x10,DAT_005a9b98);
    iVar5 = (int)DAT_005a9bf0;
    iVar6 = (int)((ulonglong)DAT_005a9bf0 >> 0x20);
    iVar18 = (int)(uVar21 ^ DAT_005a9be8) + iVar5 + (int)uVar7;
    iVar51 = (int)((ulonglong)uVar7 >> 0x20);
    iVar26 = (int)((uVar21 ^ DAT_005a9be8) >> 0x20) + iVar6 + iVar51;
    iVar42 = (int)uVar45 + (int)uVar7;
    iVar51 = (int)((ulonglong)uVar45 >> 0x20) + iVar51;
    uVar7 = *local_18;
    iVar30 = (int)(uVar34 ^ DAT_005a9be8) + iVar5 + (int)uVar65;
    iVar60 = (int)((ulonglong)uVar65 >> 0x20);
    iVar38 = (int)((uVar34 ^ DAT_005a9be8) >> 0x20) + iVar6 + iVar60;
    uVar45 = *local_10;
    iVar55 = (int)uVar58 + (int)uVar65;
    iVar60 = (int)((ulonglong)uVar58 >> 0x20) + iVar60;
    uVar58 = local_18[8];
    sVar3 = (short)uVar7 * (short)uVar45;
    sVar8 = (short)((ulonglong)uVar7 >> 0x10) * (short)((ulonglong)uVar45 >> 0x10);
    sVar9 = (short)((ulonglong)uVar7 >> 0x20) * (short)((ulonglong)uVar45 >> 0x20);
    sVar11 = (short)((ulonglong)uVar7 >> 0x30) * (short)((ulonglong)uVar45 >> 0x30);
    uVar7 = local_10[8];
    sVar17 = (short)uVar58 * (short)uVar7;
    sVar24 = (short)((ulonglong)uVar58 >> 0x10) * (short)((ulonglong)uVar7 >> 0x10);
    sVar25 = (short)((ulonglong)uVar58 >> 0x20) * (short)((ulonglong)uVar7 >> 0x20);
    sVar29 = (short)((ulonglong)uVar58 >> 0x30) * (short)((ulonglong)uVar7 >> 0x30);
    uVar37 = CONCAT26(sVar11 + sVar29,
                      CONCAT24(sVar9 + sVar25,CONCAT22(sVar8 + sVar24,sVar3 + sVar17)));
    uVar34 = CONCAT26(sVar11 - sVar29,
                      CONCAT24(sVar9 - sVar25,CONCAT22(sVar8 - sVar24,sVar3 - sVar17)));
    uVar7 = pmaddwd(uVar37,DAT_005a9bf8);
    uVar65 = pmaddwd(uVar37 >> 0x10,DAT_005a9bf8);
    uVar45 = pmaddwd(uVar34,DAT_005a9bf8);
    uVar58 = pmaddwd(uVar34 >> 0x10,DAT_005a9bf8);
    iVar4 = (int)uVar7 * 0x2000;
    iVar10 = (int)((ulonglong)uVar7 >> 0x20) * 0x2000;
    iVar64 = (int)uVar65 * 0x2000;
    iVar69 = (int)((ulonglong)uVar65 >> 0x20) * 0x2000;
    iVar43 = iVar42 + iVar4;
    iVar52 = iVar51 + iVar10;
    iVar4 = iVar4 - iVar42;
    iVar10 = iVar10 - iVar51;
    iVar19 = (int)uVar58 * 0x2000;
    iVar27 = (int)((ulonglong)uVar58 >> 0x20) * 0x2000;
    iVar56 = iVar55 + iVar64;
    iVar61 = iVar60 + iVar69;
    iVar64 = iVar64 - iVar55;
    iVar69 = iVar69 - iVar60;
    iVar31 = iVar30 + iVar19;
    iVar39 = iVar38 + iVar27;
    iVar60 = (int)uVar45 * 0x2000;
    iVar14 = (int)((ulonglong)uVar45 >> 0x20) * 0x2000;
    iVar19 = iVar19 - iVar30;
    iVar27 = iVar27 - iVar38;
    uVar7 = local_18[2];
    iVar30 = iVar18 + iVar60;
    iVar38 = iVar26 + iVar14;
    uVar45 = local_10[2];
    iVar60 = iVar60 - iVar18;
    iVar14 = iVar14 - iVar26;
    uVar58 = local_18[0xe];
    sVar63 = (short)uVar7 * (short)uVar45;
    sVar67 = (short)((ulonglong)uVar7 >> 0x10) * (short)((ulonglong)uVar45 >> 0x10);
    sVar68 = (short)((ulonglong)uVar7 >> 0x20) * (short)((ulonglong)uVar45 >> 0x20);
    sVar70 = (short)((ulonglong)uVar7 >> 0x30) * (short)((ulonglong)uVar45 >> 0x30);
    uVar66 = CONCAT26(sVar70,CONCAT24(sVar68,CONCAT22(sVar67,sVar63)));
    uVar7 = local_10[0xe];
    uVar45 = local_18[10];
    sVar3 = (short)uVar58 * (short)uVar7;
    sVar8 = (short)((ulonglong)uVar58 >> 0x10) * (short)((ulonglong)uVar7 >> 0x10);
    sVar9 = (short)((ulonglong)uVar58 >> 0x20) * (short)((ulonglong)uVar7 >> 0x20);
    sVar11 = (short)((ulonglong)uVar58 >> 0x30) * (short)((ulonglong)uVar7 >> 0x30);
    uVar37 = CONCAT26(sVar11,CONCAT24(sVar9,CONCAT22(sVar8,sVar3)));
    uVar7 = local_10[10];
    uVar58 = local_18[6];
    sVar17 = (short)uVar45 * (short)uVar7;
    sVar24 = (short)((ulonglong)uVar45 >> 0x10) * (short)((ulonglong)uVar7 >> 0x10);
    sVar25 = (short)((ulonglong)uVar45 >> 0x20) * (short)((ulonglong)uVar7 >> 0x20);
    sVar29 = (short)((ulonglong)uVar45 >> 0x30) * (short)((ulonglong)uVar7 >> 0x30);
    uVar22 = CONCAT26(sVar29,CONCAT24(sVar25,CONCAT22(sVar24,sVar17)));
    uVar7 = local_10[6];
    sVar41 = (short)uVar58 * (short)uVar7;
    sVar49 = (short)((ulonglong)uVar58 >> 0x10) * (short)((ulonglong)uVar7 >> 0x10);
    sVar50 = (short)((ulonglong)uVar58 >> 0x20) * (short)((ulonglong)uVar7 >> 0x20);
    sVar54 = (short)((ulonglong)uVar58 >> 0x30) * (short)((ulonglong)uVar7 >> 0x30);
    uVar46 = CONCAT26(sVar54,CONCAT24(sVar50,CONCAT22(sVar49,sVar41)));
    uVar34 = CONCAT26(sVar11 + sVar70,
                      CONCAT24(sVar9 + sVar68,CONCAT22(sVar8 + sVar67,sVar3 + sVar63)));
    uVar47 = CONCAT26(sVar29 + sVar54,
                      CONCAT24(sVar25 + sVar50,CONCAT22(sVar24 + sVar49,sVar17 + sVar41)));
    uVar36 = CONCAT26(sVar11 + sVar54,
                      CONCAT24(sVar9 + sVar50,CONCAT22(sVar8 + sVar49,sVar3 + sVar41)));
    uVar35 = CONCAT26(sVar29 + sVar70,
                      CONCAT24(sVar25 + sVar68,CONCAT22(sVar24 + sVar67,sVar17 + sVar63)));
    uVar23 = CONCAT26(sVar11 + sVar54 + sVar29 + sVar70,
                      CONCAT24(sVar9 + sVar50 + sVar25 + sVar68,
                               CONCAT22(sVar8 + sVar49 + sVar24 + sVar67,
                                        sVar3 + sVar41 + sVar17 + sVar63)));
    uVar7 = pmaddwd(uVar23,DAT_005a9bb0);
    uVar21 = pmaddwd(uVar34,_DAT_005a9ba8);
    uVar45 = pmaddwd(uVar23 >> 0x10,DAT_005a9bb0);
    iVar26 = (int)(uVar21 ^ DAT_005a9be8) + iVar5;
    iVar42 = (int)((uVar21 ^ DAT_005a9be8) >> 0x20) + iVar6;
    uVar23 = pmaddwd(uVar47,DAT_005a9bd8);
    uVar47 = pmaddwd(uVar47 >> 0x10,DAT_005a9bd8);
    uVar73 = pmaddwd(uVar34 >> 0x10,_DAT_005a9ba8);
    uVar21 = pmaddwd(uVar36,DAT_005a9bc8);
    iVar20 = (int)(uVar23 ^ DAT_005a9be8) + iVar5;
    iVar28 = (int)((uVar23 ^ DAT_005a9be8) >> 0x20) + iVar6;
    uVar23 = pmaddwd(uVar36 >> 0x10,DAT_005a9bc8);
    uVar36 = pmaddwd(uVar35,DAT_005a9b90);
    iVar44 = (int)(uVar47 ^ DAT_005a9be8) + iVar5;
    iVar53 = (int)((uVar47 ^ DAT_005a9be8) >> 0x20) + iVar6;
    uVar34 = pmaddwd(uVar35 >> 0x10,DAT_005a9b90);
    iVar71 = (int)(uVar73 ^ DAT_005a9be8) + iVar5;
    iVar74 = (int)((uVar73 ^ DAT_005a9be8) >> 0x20) + iVar6;
    iVar12 = (int)(uVar21 ^ DAT_005a9be8) + iVar5 + (int)uVar7;
    iVar51 = (int)((ulonglong)uVar7 >> 0x20);
    iVar15 = (int)((uVar21 ^ DAT_005a9be8) >> 0x20) + iVar6 + iVar51;
    iVar57 = (int)(uVar23 ^ DAT_005a9be8) + iVar5 + (int)uVar45;
    iVar18 = (int)((ulonglong)uVar45 >> 0x20);
    iVar62 = (int)((uVar23 ^ DAT_005a9be8) >> 0x20) + iVar6 + iVar18;
    iVar32 = (int)(uVar36 ^ DAT_005a9be8) + iVar5 + (int)uVar7;
    iVar51 = (int)((uVar36 ^ DAT_005a9be8) >> 0x20) + iVar6 + iVar51;
    iVar5 = (int)(uVar34 ^ DAT_005a9be8) + iVar5 + (int)uVar45;
    iVar18 = (int)((uVar34 ^ DAT_005a9be8) >> 0x20) + iVar6 + iVar18;
    uVar7 = pmaddwd(uVar37,DAT_005a9b88);
    uVar45 = pmaddwd(uVar37 >> 0x10,DAT_005a9b88);
    uVar65 = pmaddwd(uVar22,DAT_005a9bd0);
    uVar58 = pmaddwd(uVar22 >> 0x10,DAT_005a9bd0);
    uVar59 = pmaddwd(uVar46,DAT_005a9be0);
    uVar48 = pmaddwd(uVar46 >> 0x10,DAT_005a9be0);
    iVar6 = (int)uVar7 + iVar26 + iVar12;
    iVar55 = (int)((ulonglong)uVar7 >> 0x20) + iVar42 + iVar15;
    iVar13 = (int)uVar45 + iVar71 + iVar57;
    iVar16 = (int)((ulonglong)uVar45 >> 0x20) + iVar74 + iVar62;
    uVar45 = pmaddwd(uVar66,DAT_005a9bb8);
    iVar33 = (int)uVar65 + iVar20 + iVar32;
    iVar40 = (int)((ulonglong)uVar65 >> 0x20) + iVar28 + iVar51;
    uVar7 = pmaddwd(uVar66 >> 0x10,DAT_005a9bb8);
    iVar72 = (int)uVar45 + iVar32 + iVar26;
    iVar75 = (int)((ulonglong)uVar45 >> 0x20) + iVar51 + iVar42;
    iVar32 = (int)uVar7 + iVar5 + iVar71;
    iVar71 = (int)((ulonglong)uVar7 >> 0x20) + iVar18 + iVar74;
    iVar51 = (int)uVar58 + iVar44 + iVar5;
    iVar18 = (int)((ulonglong)uVar58 >> 0x20) + iVar53 + iVar18;
    iVar26 = (int)DAT_005a9c00;
    iVar5 = (int)((ulonglong)DAT_005a9c00 >> 0x20);
    iVar20 = (int)uVar59 + iVar12 + iVar20;
    iVar15 = (int)((ulonglong)uVar59 >> 0x20) + iVar15 + iVar28;
    iVar42 = (int)uVar48 + iVar57 + iVar44;
    iVar12 = (int)((ulonglong)uVar48 >> 0x20) + iVar62 + iVar53;
    local_14[0xe] =
         CONCAT44(CONCAT22((short)((iVar61 - iVar71) + iVar5 >> 0xb),
                           (short)((iVar52 - iVar75) + iVar5 >> 0xb)),
                  CONCAT22((short)((iVar56 - iVar32) + iVar26 >> 0xb),
                           (short)((iVar43 - iVar72) + iVar26 >> 0xb)));
    *local_14 = CONCAT44(CONCAT22((short)(iVar71 + iVar61 + iVar5 >> 0xb),
                                  (short)(iVar75 + iVar52 + iVar5 >> 0xb)),
                         CONCAT22((short)(iVar32 + iVar56 + iVar26 >> 0xb),
                                  (short)(iVar72 + iVar43 + iVar26 >> 0xb)));
    local_14[0xc] =
         CONCAT44(CONCAT22((short)((iVar39 - iVar12) + iVar5 >> 0xb),
                           (short)((iVar38 - iVar15) + iVar5 >> 0xb)),
                  CONCAT22((short)((iVar31 - iVar42) + iVar26 >> 0xb),
                           (short)((iVar30 - iVar20) + iVar26 >> 0xb)));
    local_14[2] = CONCAT44(CONCAT22((short)(iVar12 + iVar39 + iVar5 >> 0xb),
                                    (short)(iVar15 + iVar38 + iVar5 >> 0xb)),
                           CONCAT22((short)(iVar42 + iVar31 + iVar26 >> 0xb),
                                    (short)(iVar20 + iVar30 + iVar26 >> 0xb)));
    local_14[4] = CONCAT44(CONCAT22((short)(iVar27 + iVar18 + iVar5 >> 0xb),
                                    (short)(iVar14 + iVar40 + iVar5 >> 0xb)),
                           CONCAT22((short)(iVar19 + iVar51 + iVar26 >> 0xb),
                                    (short)(iVar60 + iVar33 + iVar26 >> 0xb)));
    local_14[10] = CONCAT44(CONCAT22((short)((iVar27 - iVar18) + iVar5 >> 0xb),
                                     (short)((iVar14 - iVar40) + iVar5 >> 0xb)),
                            CONCAT22((short)((iVar19 - iVar51) + iVar26 >> 0xb),
                                     (short)((iVar60 - iVar33) + iVar26 >> 0xb)));
    local_14[6] = CONCAT44(CONCAT22((short)(iVar69 + iVar16 + iVar5 >> 0xb),
                                    (short)(iVar10 + iVar55 + iVar5 >> 0xb)),
                           CONCAT22((short)(iVar64 + iVar13 + iVar26 >> 0xb),
                                    (short)(iVar4 + iVar6 + iVar26 >> 0xb)));
    local_18 = local_18 + 1;
    local_10 = local_10 + 1;
    local_14[8] = CONCAT44(CONCAT22((short)((iVar69 - iVar16) + iVar5 >> 0xb),
                                    (short)((iVar10 - iVar55) + iVar5 >> 0xb)),
                           CONCAT22((short)((iVar64 - iVar13) + iVar26 >> 0xb),
                                    (short)((iVar4 - iVar6) + iVar26 >> 0xb)));
    local_14 = local_14 + 1;
    local_8 = local_8 + -1;
  } while (local_8 != 0);
  local_8 = 8;
  local_14 = param_3;
  local_c = 0;
  do {
    uVar7 = *local_14;
    uVar45 = local_14[1];
    sVar3 = (short)uVar45;
    sVar17 = (short)uVar7;
    sVar8 = (short)((ulonglong)uVar45 >> 0x10);
    sVar24 = (short)((ulonglong)uVar7 >> 0x10);
    iVar18 = CONCAT22(sVar24 + sVar8,sVar17 + sVar3);
    sVar9 = (short)((ulonglong)uVar45 >> 0x20);
    sVar25 = (short)((ulonglong)uVar7 >> 0x20);
    sVar29 = (short)((ulonglong)uVar7 >> 0x30);
    sVar11 = (short)((ulonglong)uVar45 >> 0x30);
    uVar65 = pmaddwd(CONCAT44((int)((ulonglong)uVar45 >> 0x20),(int)((ulonglong)uVar7 >> 0x20)),
                     _DAT_005a9c18);
    uVar58 = pmaddwd(CONCAT26(sVar29 + sVar11,CONCAT24(sVar25 + sVar9,iVar18)),_DAT_005a9c20);
    iVar4 = (int)((ulonglong)uVar58 >> 0x20);
    iVar51 = (int)((ulonglong)DAT_005a9bf0 >> 0x20);
    iVar5 = (iVar18 << 0x10) >> 3;
    iVar10 = (int)((uint)(ushort)(sVar17 - sVar3) << 0x10) >> 3;
    iVar18 = (int)uVar65 + iVar4;
    iVar4 = ((uint)((ulonglong)uVar65 >> 0x20) ^ (uint)(DAT_005a9be8 >> 0x20)) + iVar51 + iVar4;
    sVar41 = sVar17 + sVar3;
    sVar49 = sVar24 + sVar8;
    iVar6 = iVar5 + iVar18;
    iVar38 = iVar10 + iVar4;
    iVar5 = iVar5 - iVar18;
    iVar10 = iVar10 - iVar4;
    uVar7 = pmaddwd(uVar7,_DAT_005a9c28);
    uVar58 = pmaddwd(CONCAT26(sVar49 + sVar29 + sVar11,
                              CONCAT24(sVar41 + sVar25 + sVar9,CONCAT22(sVar49 * 2,sVar41 * 2))),
                     _DAT_005a9c48);
    uVar37 = pmaddwd(CONCAT26(sVar29 + sVar11,CONCAT24(sVar25 + sVar9,CONCAT22(sVar49,sVar41))),
                     _DAT_005a9c40);
    uVar34 = pmaddwd(CONCAT26(sVar24 + sVar11,
                              CONCAT24(sVar17 + sVar9,CONCAT22(sVar29 + sVar8,sVar25 + sVar3))),
                     _DAT_005a9c38);
    uVar45 = pmaddwd(uVar45,_DAT_005a9c30);
    iVar26 = (int)((ulonglong)uVar58 >> 0x20);
    iVar42 = (int)(uVar34 ^ DAT_005a9be8) + (int)DAT_005a9bf0;
    iVar55 = (int)((uVar34 ^ DAT_005a9be8) >> 0x20) + iVar51;
    iVar18 = (int)(uVar37 ^ DAT_005a9be8) + (int)DAT_005a9bf0 + iVar26;
    iVar26 = (int)((uVar37 ^ DAT_005a9be8) >> 0x20) + iVar51 + iVar26;
    iVar4 = (int)uVar45 + iVar42 + iVar18;
    iVar30 = (int)((ulonglong)uVar45 >> 0x20) + iVar55 + iVar26;
    iVar51 = (int)uVar7 + iVar55 + iVar18;
    iVar18 = (int)((ulonglong)uVar7 >> 0x20) + iVar42 + iVar26;
    iVar26 = (int)DAT_005a9c08;
    iVar42 = (int)((ulonglong)DAT_005a9c08 >> 0x20);
    uVar34 = CONCAT44(iVar4 + iVar10 + iVar42 >> 0x12,iVar30 + iVar5 + iVar26 >> 0x12) &
             DAT_005a9c10;
    piVar2 = (int *)(param_4 + local_c);
    uVar37 = CONCAT44(iVar18 + iVar38 + iVar42 >> 0x12,iVar51 + iVar6 + iVar26 >> 0x12) &
             DAT_005a9c10;
    uVar1 = CONCAT11(*(undefined1 *)(param_6 + (int)uVar34),
                     *(undefined1 *)(param_6 + (int)(uVar34 >> 0x20)));
    local_c = local_c + 4;
    uVar21 = CONCAT44((iVar38 - iVar18) + iVar42 >> 0x12,(iVar6 - iVar51) + iVar26 >> 0x12) &
             DAT_005a9c10;
    uVar34 = CONCAT44((iVar10 - iVar4) + iVar42 >> 0x12,(iVar5 - iVar30) + iVar26 >> 0x12) &
             DAT_005a9c10;
    local_14 = local_14 + 2;
    *(ulonglong *)(*piVar2 + param_5) =
         CONCAT44(CONCAT22((short)CONCAT31(CONCAT21(uVar1,*(undefined1 *)(param_6 + (int)uVar21)),
                                           *(undefined1 *)(param_6 + (int)(uVar21 >> 0x20))),
                           CONCAT11(*(undefined1 *)(param_6 + (int)(uVar34 >> 0x20)),
                                    *(undefined1 *)(param_6 + (int)uVar34))),
                  CONCAT22(uVar1,CONCAT11(*(undefined1 *)(param_6 + (int)(uVar37 >> 0x20)),
                                          *(undefined1 *)(param_6 + (int)uVar37))));
    local_8 = local_8 + -1;
  } while (local_8 != 0);
  return;
}

