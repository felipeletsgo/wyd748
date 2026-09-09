// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058d80a | Name: FUN_0058d80a


void FUN_0058d80a(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,int *param_4,
                 int param_5,int param_6)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  undefined2 uVar5;
  undefined2 uVar6;
  int iVar7;
  undefined1 *puVar8;
  short sVar9;
  undefined4 uVar10;
  undefined6 uVar11;
  short sVar13;
  short sVar14;
  short sVar15;
  undefined8 uVar12;
  short sVar16;
  short sVar19;
  short sVar20;
  short sVar21;
  undefined8 uVar17;
  undefined8 uVar18;
  short sVar22;
  short sVar24;
  undefined4 uVar23;
  short sVar25;
  short sVar26;
  short sVar27;
  short sVar28;
  short sVar29;
  short sVar30;
  short sVar31;
  short sVar32;
  short sVar33;
  short sVar34;
  undefined4 uVar35;
  undefined8 uVar36;
  short sVar37;
  ushort uVar38;
  short sVar39;
  short sVar40;
  short sVar41;
  short sVar42;
  short sVar43;
  short sVar44;
  short sVar45;
  ushort uVar46;
  undefined8 uVar47;
  short sVar48;
  undefined8 uVar49;
  
  uVar17 = param_1[0xc];
  uVar47 = param_3[0xc];
  sVar9 = (short)uVar17 * (short)uVar47;
  sVar13 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar14 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar15 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[4];
  uVar47 = param_3[4];
  sVar16 = (short)uVar17 * (short)uVar47;
  sVar19 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar20 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar21 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = *param_1;
  uVar47 = *param_3;
  sVar27 = (short)uVar17 * (short)uVar47;
  sVar29 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar31 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar33 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[8];
  uVar47 = param_3[8];
  sVar37 = (short)uVar17 * (short)uVar47;
  sVar39 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar40 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar42 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  sVar22 = sVar16 + sVar9;
  sVar24 = sVar19 + sVar13;
  sVar25 = sVar20 + sVar14;
  sVar26 = sVar21 + sVar15;
  uVar17 = pmaddwd(CONCAT44(CONCAT22((short)((uint)(undefined4)DAT_005a9c80 >> 0x10),sVar19 - sVar13
                                    ),CONCAT22((short)(undefined4)DAT_005a9c80,sVar16 - sVar9)),
                   DAT_005a9c60);
  uVar47 = pmaddwd(CONCAT26((short)((uint)DAT_005a9c80._4_4_ >> 0x10),
                            CONCAT24(sVar21 - sVar15,
                                     CONCAT22((short)DAT_005a9c80._4_4_,sVar20 - sVar14))),
                   DAT_005a9c60);
  sVar28 = sVar27 + sVar37;
  sVar30 = sVar29 + sVar39;
  sVar32 = sVar31 + sVar40;
  sVar34 = sVar33 + sVar42;
  sVar27 = sVar27 - sVar37;
  sVar29 = sVar29 - sVar39;
  sVar31 = sVar31 - sVar40;
  sVar33 = sVar33 - sVar42;
  uVar17 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  sVar9 = (short)uVar17 - sVar22;
  sVar13 = (short)((ulonglong)uVar17 >> 0x10) - sVar24;
  sVar14 = (short)((ulonglong)uVar17 >> 0x20) - sVar25;
  sVar15 = (short)((ulonglong)uVar17 >> 0x30) - sVar26;
  param_2[4] = CONCAT26(sVar34 + sVar26,
                        CONCAT24(sVar32 + sVar25,CONCAT22(sVar30 + sVar24,sVar28 + sVar22)));
  param_2[0xc] = CONCAT26(sVar15 + sVar33,
                          CONCAT24(sVar14 + sVar31,CONCAT22(sVar13 + sVar29,sVar9 + sVar27)));
  sVar28 = sVar28 - sVar22;
  sVar30 = sVar30 - sVar24;
  sVar32 = sVar32 - sVar25;
  sVar34 = sVar34 - sVar26;
  *param_2 = CONCAT26(sVar33 - sVar15,
                      CONCAT24(sVar31 - sVar14,CONCAT22(sVar29 - sVar13,sVar27 - sVar9)));
  uVar17 = param_1[10];
  uVar47 = param_3[10];
  sVar22 = (short)uVar17 * (short)uVar47;
  sVar24 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar25 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar26 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[6];
  uVar47 = param_3[6];
  sVar9 = (short)uVar17 * (short)uVar47;
  sVar14 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar16 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar20 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[2];
  uVar47 = param_3[2];
  sVar13 = (short)uVar17 * (short)uVar47;
  sVar15 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar19 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar21 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  sVar37 = sVar22 - sVar9;
  sVar39 = sVar24 - sVar14;
  sVar40 = sVar25 - sVar16;
  sVar42 = sVar26 - sVar20;
  uVar17 = param_1[0xe];
  uVar47 = param_3[0xe];
  sVar27 = (short)uVar17 * (short)uVar47;
  sVar29 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar31 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar33 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  sVar22 = sVar22 + sVar9;
  sVar24 = sVar24 + sVar14;
  sVar25 = sVar25 + sVar16;
  sVar26 = sVar26 + sVar20;
  uVar6 = (undefined2)((uint)(undefined4)DAT_005a9c80 >> 0x10);
  uVar5 = (undefined2)(undefined4)DAT_005a9c80;
  uVar47 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar39),CONCAT22(uVar5,sVar37)),DAT_005a9c68);
  sVar9 = sVar13 + sVar27;
  sVar14 = sVar15 + sVar29;
  sVar16 = sVar19 + sVar31;
  sVar20 = sVar21 + sVar33;
  uVar3 = (undefined2)DAT_005a9c80._4_4_;
  uVar4 = (undefined2)((uint)DAT_005a9c80._4_4_ >> 0x10);
  sVar13 = sVar13 - sVar27;
  sVar15 = sVar15 - sVar29;
  sVar19 = sVar19 - sVar31;
  sVar21 = sVar21 - sVar33;
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar42,CONCAT22(uVar3,sVar40))),DAT_005a9c68);
  uVar49 = packssdw(CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar15),CONCAT22(uVar5,sVar13)),DAT_005a9c70);
  uVar47 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar21,CONCAT22(uVar3,sVar19))),DAT_005a9c70);
  uVar12 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  uVar47 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar39 + sVar15),CONCAT22(uVar5,sVar37 + sVar13)),
                   DAT_005a9c78);
  sVar27 = sVar9 + sVar22;
  sVar29 = sVar14 + sVar24;
  sVar31 = sVar16 + sVar25;
  sVar33 = sVar20 + sVar26;
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar42 + sVar21,CONCAT22(uVar3,sVar40 + sVar19))),
                   DAT_005a9c78);
  uVar36 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar14 - sVar24),CONCAT22(uVar5,sVar9 - sVar22)),
                   DAT_005a9c60);
  uVar18 = packssdw(CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  uVar47 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar20 - sVar26,CONCAT22(uVar3,sVar16 - sVar25))),
                   DAT_005a9c60);
  sVar13 = (short)((ulonglong)uVar18 >> 0x10);
  sVar14 = (short)((ulonglong)uVar18 >> 0x20);
  sVar15 = (short)((ulonglong)uVar18 >> 0x30);
  uVar17 = param_2[4];
  uVar36 = packssdw(CONCAT44((int)((longlong)uVar36 >> 0x28),(int)uVar36 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  sVar16 = ((short)uVar18 + (short)uVar49) - sVar27;
  sVar19 = (sVar13 + (short)((ulonglong)uVar49 >> 0x10)) - sVar29;
  sVar20 = (sVar14 + (short)((ulonglong)uVar49 >> 0x20)) - sVar31;
  sVar21 = (sVar15 + (short)((ulonglong)uVar49 >> 0x30)) - sVar33;
  uVar47 = param_2[0xc];
  sVar22 = (short)uVar36 - sVar16;
  sVar24 = (short)((ulonglong)uVar36 >> 0x10) - sVar19;
  sVar25 = (short)((ulonglong)uVar36 >> 0x20) - sVar20;
  sVar26 = (short)((ulonglong)uVar36 >> 0x30) - sVar21;
  sVar9 = ((short)uVar12 - (short)uVar18) + sVar22;
  sVar13 = ((short)((ulonglong)uVar12 >> 0x10) - sVar13) + sVar24;
  sVar14 = ((short)((ulonglong)uVar12 >> 0x20) - sVar14) + sVar25;
  sVar15 = ((short)((ulonglong)uVar12 >> 0x30) - sVar15) + sVar26;
  param_2[8] = CONCAT26(sVar34 + sVar15,
                        CONCAT24(sVar32 + sVar14,CONCAT22(sVar30 + sVar13,sVar28 + sVar9)));
  sVar37 = (short)((ulonglong)uVar17 >> 0x10);
  sVar39 = (short)((ulonglong)uVar17 >> 0x20);
  sVar40 = (short)((ulonglong)uVar17 >> 0x30);
  param_2[6] = CONCAT26(sVar34 - sVar15,
                        CONCAT24(sVar32 - sVar14,CONCAT22(sVar30 - sVar13,sVar28 - sVar9)));
  uVar12 = *param_2;
  *param_2 = CONCAT26(sVar40 + sVar33,
                      CONCAT24(sVar39 + sVar31,CONCAT22(sVar37 + sVar29,(short)uVar17 + sVar27)));
  sVar9 = (short)((ulonglong)uVar47 >> 0x10);
  sVar13 = (short)((ulonglong)uVar47 >> 0x20);
  sVar14 = (short)((ulonglong)uVar47 >> 0x30);
  param_2[0xe] = CONCAT26(sVar40 - sVar33,
                          CONCAT24(sVar39 - sVar31,CONCAT22(sVar37 - sVar29,(short)uVar17 - sVar27))
                         );
  param_2[2] = CONCAT26(sVar14 + sVar21,
                        CONCAT24(sVar13 + sVar20,CONCAT22(sVar9 + sVar19,(short)uVar47 + sVar16)));
  param_2[0xc] = CONCAT26(sVar14 - sVar21,
                          CONCAT24(sVar13 - sVar20,CONCAT22(sVar9 - sVar19,(short)uVar47 - sVar16)))
  ;
  sVar16 = (short)((ulonglong)uVar12 >> 0x10);
  sVar19 = (short)((ulonglong)uVar12 >> 0x20);
  sVar20 = (short)((ulonglong)uVar12 >> 0x30);
  uVar17 = param_1[5];
  uVar47 = param_3[5];
  sVar9 = (short)uVar17 * (short)uVar47;
  sVar13 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar14 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar15 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  param_2[4] = CONCAT26(sVar20 + sVar26,
                        CONCAT24(sVar19 + sVar25,CONCAT22(sVar16 + sVar24,(short)uVar12 + sVar22)));
  param_2[10] = CONCAT26(sVar20 - sVar26,
                         CONCAT24(sVar19 - sVar25,CONCAT22(sVar16 - sVar24,(short)uVar12 - sVar22)))
  ;
  uVar17 = param_1[0xd];
  uVar47 = param_3[0xd];
  sVar16 = (short)uVar17 * (short)uVar47;
  sVar20 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar28 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar30 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[1];
  uVar47 = param_3[1];
  sVar27 = (short)uVar17 * (short)uVar47;
  sVar29 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar31 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar33 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[9];
  uVar47 = param_3[9];
  sVar19 = (short)uVar17 * (short)uVar47;
  sVar21 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar37 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar39 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  sVar22 = sVar9 + sVar16;
  sVar24 = sVar13 + sVar20;
  sVar25 = sVar14 + sVar28;
  sVar26 = sVar15 + sVar30;
  uVar12 = pmaddwd(CONCAT44(CONCAT22((short)((uint)(undefined4)DAT_005a9c80 >> 0x10),sVar13 - sVar20
                                    ),CONCAT22((short)(undefined4)DAT_005a9c80,sVar9 - sVar16)),
                   DAT_005a9c60);
  uVar17 = param_1[0xb];
  uVar18 = pmaddwd(CONCAT26((short)((uint)DAT_005a9c80._4_4_ >> 0x10),
                            CONCAT24(sVar15 - sVar30,
                                     CONCAT22((short)DAT_005a9c80._4_4_,sVar14 - sVar28))),
                   DAT_005a9c60);
  uVar47 = param_3[0xb];
  sVar9 = (short)uVar17 * (short)uVar47;
  sVar14 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar16 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar20 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = packssdw(CONCAT44((int)((longlong)uVar12 >> 0x28),(int)uVar12 >> 8),
                    CONCAT44((int)((longlong)uVar18 >> 0x28),(int)uVar18 >> 8));
  sVar28 = sVar27 + sVar19;
  sVar30 = sVar29 + sVar21;
  sVar32 = sVar31 + sVar37;
  sVar34 = sVar33 + sVar39;
  sVar27 = sVar27 - sVar19;
  sVar29 = sVar29 - sVar21;
  sVar31 = sVar31 - sVar37;
  sVar33 = sVar33 - sVar39;
  sVar13 = (short)uVar17 - sVar22;
  sVar15 = (short)((ulonglong)uVar17 >> 0x10) - sVar24;
  sVar19 = (short)((ulonglong)uVar17 >> 0x20) - sVar25;
  sVar21 = (short)((ulonglong)uVar17 >> 0x30) - sVar26;
  uVar17 = param_1[7];
  param_2[5] = CONCAT26(sVar34 + sVar26,
                        CONCAT24(sVar32 + sVar25,CONCAT22(sVar30 + sVar24,sVar28 + sVar22)));
  uVar47 = param_3[7];
  sVar37 = (short)uVar17 * (short)uVar47;
  sVar39 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar40 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar42 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  sVar28 = sVar28 - sVar22;
  sVar30 = sVar30 - sVar24;
  sVar32 = sVar32 - sVar25;
  sVar34 = sVar34 - sVar26;
  param_2[0xd] = CONCAT26(sVar33 + sVar21,
                          CONCAT24(sVar31 + sVar19,CONCAT22(sVar29 + sVar15,sVar27 + sVar13)));
  param_2[9] = CONCAT26(sVar33 - sVar21,
                        CONCAT24(sVar31 - sVar19,CONCAT22(sVar29 - sVar15,sVar27 - sVar13)));
  sVar13 = sVar9 + sVar37;
  sVar15 = sVar14 + sVar39;
  sVar19 = sVar16 + sVar40;
  sVar21 = sVar20 + sVar42;
  uVar17 = param_1[0xf];
  sVar9 = sVar9 - sVar37;
  sVar14 = sVar14 - sVar39;
  sVar16 = sVar16 - sVar40;
  sVar20 = sVar20 - sVar42;
  uVar47 = param_3[0xf];
  sVar22 = (short)uVar17 * (short)uVar47;
  sVar24 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar25 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar26 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  uVar17 = param_1[3];
  uVar47 = param_3[3];
  sVar27 = (short)uVar17 * (short)uVar47;
  sVar31 = (short)((ulonglong)uVar17 >> 0x10) * (short)((ulonglong)uVar47 >> 0x10);
  sVar37 = (short)((ulonglong)uVar17 >> 0x20) * (short)((ulonglong)uVar47 >> 0x20);
  sVar43 = (short)((ulonglong)uVar17 >> 0x30) * (short)((ulonglong)uVar47 >> 0x30);
  sVar29 = sVar27 + sVar22;
  sVar33 = sVar31 + sVar24;
  sVar42 = sVar37 + sVar25;
  sVar44 = sVar43 + sVar26;
  uVar6 = (undefined2)((uint)(undefined4)DAT_005a9c80 >> 0x10);
  uVar5 = (undefined2)(undefined4)DAT_005a9c80;
  sVar27 = sVar27 - sVar22;
  sVar31 = sVar31 - sVar24;
  sVar37 = sVar37 - sVar25;
  sVar43 = sVar43 - sVar26;
  uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar14),CONCAT22(uVar5,sVar9)),DAT_005a9c68);
  uVar3 = (undefined2)DAT_005a9c80._4_4_;
  uVar4 = (undefined2)((uint)DAT_005a9c80._4_4_ >> 0x10);
  sVar39 = sVar29 + sVar13;
  sVar40 = sVar33 + sVar15;
  sVar41 = sVar42 + sVar19;
  sVar45 = sVar44 + sVar21;
  uVar47 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar20,CONCAT22(uVar3,sVar16))),DAT_005a9c68);
  uVar47 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  uVar12 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar33 - sVar15),CONCAT22(uVar5,sVar29 - sVar13)),
                   DAT_005a9c60);
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar44 - sVar21,CONCAT22(uVar3,sVar42 - sVar19))),
                   DAT_005a9c60);
  uVar18 = packssdw(CONCAT44((int)((longlong)uVar12 >> 0x28),(int)uVar12 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar14 + sVar31),CONCAT22(uVar5,sVar9 + sVar27)),
                   DAT_005a9c78);
  uVar12 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar20 + sVar43,CONCAT22(uVar3,sVar16 + sVar37))),
                   DAT_005a9c78);
  uVar12 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar12 >> 0x28),(int)uVar12 >> 8));
  uVar36 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar31),CONCAT22(uVar5,sVar27)),DAT_005a9c70);
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar43,CONCAT22(uVar3,sVar37))),DAT_005a9c70);
  uVar36 = packssdw(CONCAT44((int)((longlong)uVar36 >> 0x28),(int)uVar36 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  uVar17 = param_2[5];
  sVar13 = (short)((ulonglong)uVar12 >> 0x10);
  sVar15 = (short)((ulonglong)uVar12 >> 0x20);
  sVar19 = (short)((ulonglong)uVar12 >> 0x30);
  sVar20 = ((short)uVar12 + (short)uVar47) - sVar39;
  sVar21 = (sVar13 + (short)((ulonglong)uVar47 >> 0x10)) - sVar40;
  sVar24 = (sVar15 + (short)((ulonglong)uVar47 >> 0x20)) - sVar41;
  sVar26 = (sVar19 + (short)((ulonglong)uVar47 >> 0x30)) - sVar45;
  sVar29 = (short)uVar18 - sVar20;
  sVar31 = (short)((ulonglong)uVar18 >> 0x10) - sVar21;
  sVar33 = (short)((ulonglong)uVar18 >> 0x20) - sVar24;
  sVar37 = (short)((ulonglong)uVar18 >> 0x30) - sVar26;
  sVar9 = (short)((ulonglong)uVar17 >> 0x10);
  sVar14 = (short)((ulonglong)uVar17 >> 0x20);
  sVar16 = (short)((ulonglong)uVar17 >> 0x30);
  uVar47 = param_2[0xd];
  sVar42 = ((short)uVar36 - (short)uVar12) + sVar29;
  sVar43 = ((short)((ulonglong)uVar36 >> 0x10) - sVar13) + sVar31;
  sVar44 = ((short)((ulonglong)uVar36 >> 0x20) - sVar15) + sVar33;
  sVar48 = ((short)((ulonglong)uVar36 >> 0x30) - sVar19) + sVar37;
  param_2[1] = CONCAT26(sVar16 + sVar45,
                        CONCAT24(sVar14 + sVar41,CONCAT22(sVar9 + sVar40,(short)uVar17 + sVar39)));
  uVar12 = param_2[9];
  param_2[7] = CONCAT26(sVar34 - sVar48,
                        CONCAT24(sVar32 - sVar44,CONCAT22(sVar30 - sVar43,sVar28 - sVar42)));
  sVar22 = (short)((ulonglong)uVar47 >> 0x10);
  sVar25 = (short)((ulonglong)uVar47 >> 0x20);
  sVar27 = (short)((ulonglong)uVar47 >> 0x30);
  param_2[3] = CONCAT26(sVar27 + sVar26,
                        CONCAT24(sVar25 + sVar24,CONCAT22(sVar22 + sVar21,(short)uVar47 + sVar20)));
  sVar13 = (short)((ulonglong)uVar12 >> 0x10);
  sVar15 = (short)((ulonglong)uVar12 >> 0x20);
  sVar19 = (short)((ulonglong)uVar12 >> 0x30);
  param_2[5] = CONCAT26(sVar19 + sVar37,
                        CONCAT24(sVar15 + sVar33,CONCAT22(sVar13 + sVar31,(short)uVar12 + sVar29)));
  uVar10 = CONCAT22(sVar15 - sVar33,sVar32 + sVar44);
  uVar18 = *param_2;
  uVar36 = param_2[2];
  uVar23 = CONCAT22(sVar14 - sVar41,sVar25 - sVar24);
  param_2[9] = CONCAT44(CONCAT22((short)uVar17 - sVar39,(short)uVar47 - sVar20),
                        CONCAT22((short)uVar12 - sVar29,sVar28 + sVar42));
  param_2[0xb] = CONCAT44(CONCAT22(sVar9 - sVar40,sVar22 - sVar21),
                          CONCAT22(sVar13 - sVar31,sVar30 + sVar43));
  param_2[0xd] = CONCAT44(uVar23,uVar10);
  param_2[0xf] = CONCAT44((int)(CONCAT26(sVar16 - sVar45,CONCAT24(sVar27 - sVar26,uVar23)) >> 0x20),
                          (int)(CONCAT26(sVar19 - sVar37,CONCAT24(sVar34 + sVar48,uVar10)) >> 0x20))
  ;
  uVar23 = CONCAT22((short)((ulonglong)uVar36 >> 0x20),(short)((ulonglong)uVar18 >> 0x20));
  uVar17 = param_2[4];
  uVar47 = param_2[6];
  uVar10 = CONCAT22((short)((ulonglong)uVar47 >> 0x20),(short)((ulonglong)uVar17 >> 0x20));
  *param_2 = CONCAT44(CONCAT22((short)uVar47,(short)uVar17),CONCAT22((short)uVar36,(short)uVar18));
  param_2[2] = CONCAT44(CONCAT22((short)((ulonglong)uVar47 >> 0x10),
                                 (short)((ulonglong)uVar17 >> 0x10)),
                        CONCAT22((short)((ulonglong)uVar36 >> 0x10),
                                 (short)((ulonglong)uVar18 >> 0x10)));
  param_2[4] = CONCAT44(uVar10,uVar23);
  param_2[6] = CONCAT44((int)(CONCAT26((short)((ulonglong)uVar47 >> 0x30),
                                       CONCAT24((short)((ulonglong)uVar17 >> 0x30),uVar10)) >> 0x20)
                        ,(int)(CONCAT26((short)((ulonglong)uVar36 >> 0x30),
                                        CONCAT24((short)((ulonglong)uVar18 >> 0x30),uVar23)) >> 0x20
                              ));
  uVar17 = param_2[1];
  uVar47 = param_2[3];
  uVar12 = param_2[5];
  uVar23 = CONCAT22((short)((ulonglong)uVar47 >> 0x20),(short)((ulonglong)uVar17 >> 0x20));
  uVar18 = param_2[7];
  uVar35 = CONCAT22((short)((ulonglong)uVar18 >> 0x20),(short)((ulonglong)uVar12 >> 0x20));
  uVar36 = param_2[8];
  param_2[8] = CONCAT44(CONCAT22((short)uVar18,(short)uVar12),CONCAT22((short)uVar47,(short)uVar17))
  ;
  uVar49 = param_2[10];
  param_2[10] = CONCAT44(CONCAT22((short)((ulonglong)uVar18 >> 0x10),
                                  (short)((ulonglong)uVar12 >> 0x10)),
                         CONCAT22((short)((ulonglong)uVar47 >> 0x10),
                                  (short)((ulonglong)uVar17 >> 0x10)));
  uVar1 = param_2[0xc];
  uVar10 = CONCAT22((short)((ulonglong)uVar49 >> 0x20),(short)((ulonglong)uVar36 >> 0x20));
  uVar2 = param_2[0xe];
  param_2[0xc] = CONCAT44(uVar35,uVar23);
  param_2[0xe] = CONCAT44((int)(CONCAT26((short)((ulonglong)uVar18 >> 0x30),
                                         CONCAT24((short)((ulonglong)uVar12 >> 0x30),uVar35)) >>
                               0x20),
                          (int)(CONCAT26((short)((ulonglong)uVar47 >> 0x30),
                                         CONCAT24((short)((ulonglong)uVar17 >> 0x30),uVar23)) >>
                               0x20));
  uVar23 = CONCAT22((short)((ulonglong)uVar2 >> 0x20),(short)((ulonglong)uVar1 >> 0x20));
  param_2[1] = CONCAT44(CONCAT22((short)uVar2,(short)uVar1),CONCAT22((short)uVar49,(short)uVar36));
  param_2[3] = CONCAT44(CONCAT22((short)((ulonglong)uVar2 >> 0x10),(short)((ulonglong)uVar1 >> 0x10)
                                ),
                        CONCAT22((short)((ulonglong)uVar49 >> 0x10),
                                 (short)((ulonglong)uVar36 >> 0x10)));
  param_2[5] = CONCAT44(uVar23,uVar10);
  param_2[7] = CONCAT44((int)(CONCAT26((short)((ulonglong)uVar2 >> 0x30),
                                       CONCAT24((short)((ulonglong)uVar1 >> 0x30),uVar23)) >> 0x20),
                        (int)(CONCAT26((short)((ulonglong)uVar49 >> 0x30),
                                       CONCAT24((short)((ulonglong)uVar36 >> 0x30),uVar10)) >> 0x20)
                       );
  uVar17 = param_2[0xc];
  uVar47 = param_2[4];
  uVar12 = *param_2;
  uVar18 = param_2[8];
  sVar9 = (short)((ulonglong)uVar17 >> 0x10);
  sVar15 = (short)((ulonglong)uVar47 >> 0x10);
  sVar13 = (short)((ulonglong)uVar17 >> 0x20);
  sVar16 = (short)((ulonglong)uVar47 >> 0x20);
  sVar19 = (short)((ulonglong)uVar47 >> 0x30);
  sVar14 = (short)((ulonglong)uVar17 >> 0x30);
  sVar20 = (short)uVar47 + (short)uVar17;
  sVar21 = sVar15 + sVar9;
  sVar22 = sVar16 + sVar13;
  sVar24 = sVar19 + sVar14;
  uVar17 = pmaddwd(CONCAT44(CONCAT22((short)((uint)(undefined4)DAT_005a9c80 >> 0x10),sVar15 - sVar9)
                            ,CONCAT22((short)(undefined4)DAT_005a9c80,(short)uVar47 - (short)uVar17)
                           ),DAT_005a9c60);
  uVar47 = pmaddwd(CONCAT26((short)((uint)DAT_005a9c80._4_4_ >> 0x10),
                            CONCAT24(sVar19 - sVar14,
                                     CONCAT22((short)DAT_005a9c80._4_4_,sVar16 - sVar13))),
                   DAT_005a9c60);
  uVar17 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  sVar25 = (short)uVar12 + (short)uVar18;
  sVar16 = (short)((ulonglong)uVar12 >> 0x10);
  sVar9 = (short)((ulonglong)uVar18 >> 0x10);
  sVar26 = sVar16 + sVar9;
  sVar19 = (short)((ulonglong)uVar12 >> 0x20);
  sVar28 = (short)((ulonglong)uVar12 >> 0x30);
  sVar13 = (short)((ulonglong)uVar18 >> 0x20);
  sVar27 = sVar19 + sVar13;
  sVar14 = (short)((ulonglong)uVar18 >> 0x30);
  sVar29 = sVar28 + sVar14;
  sVar30 = (short)uVar12 - (short)uVar18;
  sVar16 = sVar16 - sVar9;
  sVar19 = sVar19 - sVar13;
  sVar28 = sVar28 - sVar14;
  sVar9 = (short)uVar17 - sVar20;
  sVar13 = (short)((ulonglong)uVar17 >> 0x10) - sVar21;
  sVar14 = (short)((ulonglong)uVar17 >> 0x20) - sVar22;
  sVar15 = (short)((ulonglong)uVar17 >> 0x30) - sVar24;
  param_2[4] = CONCAT26(sVar29 + sVar24,
                        CONCAT24(sVar27 + sVar22,CONCAT22(sVar26 + sVar21,sVar25 + sVar20)));
  param_2[0xc] = CONCAT26(sVar15 + sVar28,
                          CONCAT24(sVar14 + sVar19,CONCAT22(sVar13 + sVar16,sVar9 + sVar30)));
  sVar25 = sVar25 - sVar20;
  sVar26 = sVar26 - sVar21;
  sVar27 = sVar27 - sVar22;
  sVar29 = sVar29 - sVar24;
  *param_2 = CONCAT26(sVar28 - sVar15,
                      CONCAT24(sVar19 - sVar14,CONCAT22(sVar16 - sVar13,sVar30 - sVar9)));
  uVar17 = param_2[10];
  uVar47 = param_2[6];
  uVar12 = param_2[2];
  sVar21 = (short)uVar17 - (short)uVar47;
  sVar9 = (short)((ulonglong)uVar47 >> 0x10);
  sVar24 = (short)((ulonglong)uVar17 >> 0x10);
  sVar41 = sVar24 - sVar9;
  sVar13 = (short)((ulonglong)uVar47 >> 0x20);
  sVar28 = (short)((ulonglong)uVar17 >> 0x20);
  sVar30 = (short)((ulonglong)uVar17 >> 0x30);
  sVar43 = sVar28 - sVar13;
  sVar14 = (short)((ulonglong)uVar47 >> 0x30);
  sVar44 = sVar30 - sVar14;
  uVar18 = param_2[0xe];
  sVar22 = (short)uVar17 + (short)uVar47;
  sVar24 = sVar24 + sVar9;
  sVar28 = sVar28 + sVar13;
  sVar30 = sVar30 + sVar14;
  uVar6 = (undefined2)((uint)(undefined4)DAT_005a9c80 >> 0x10);
  uVar5 = (undefined2)(undefined4)DAT_005a9c80;
  uVar47 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar41),CONCAT22(uVar5,sVar21)),DAT_005a9c68);
  uVar3 = (undefined2)DAT_005a9c80._4_4_;
  uVar4 = (undefined2)((uint)DAT_005a9c80._4_4_ >> 0x10);
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar44,CONCAT22(uVar3,sVar43))),DAT_005a9c68);
  uVar36 = packssdw(CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  sVar31 = (short)uVar12 + (short)uVar18;
  sVar16 = (short)((ulonglong)uVar18 >> 0x10);
  sVar13 = (short)((ulonglong)uVar12 >> 0x10);
  sVar33 = sVar13 + sVar16;
  sVar19 = (short)((ulonglong)uVar18 >> 0x20);
  sVar14 = (short)((ulonglong)uVar12 >> 0x20);
  sVar15 = (short)((ulonglong)uVar12 >> 0x30);
  sVar37 = sVar14 + sVar19;
  sVar20 = (short)((ulonglong)uVar18 >> 0x30);
  sVar40 = sVar15 + sVar20;
  sVar9 = (short)uVar12 - (short)uVar18;
  sVar13 = sVar13 - sVar16;
  sVar14 = sVar14 - sVar19;
  sVar15 = sVar15 - sVar20;
  uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar13),CONCAT22(uVar5,sVar9)),DAT_005a9c70);
  uVar47 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar15,CONCAT22(uVar3,sVar14))),DAT_005a9c70);
  uVar12 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  sVar32 = sVar31 + sVar22;
  sVar34 = sVar33 + sVar24;
  sVar39 = sVar37 + sVar28;
  sVar42 = sVar40 + sVar30;
  uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar41 + sVar13),CONCAT22(uVar5,sVar21 + sVar9)),
                   DAT_005a9c78);
  uVar47 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar44 + sVar15,CONCAT22(uVar3,sVar43 + sVar14))),
                   DAT_005a9c78);
  uVar18 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  sVar13 = (short)((ulonglong)uVar18 >> 0x10);
  sVar14 = (short)((ulonglong)uVar18 >> 0x20);
  sVar15 = (short)((ulonglong)uVar18 >> 0x30);
  sVar16 = ((short)uVar18 + (short)uVar36) - sVar32;
  sVar19 = (sVar13 + (short)((ulonglong)uVar36 >> 0x10)) - sVar34;
  sVar20 = (sVar14 + (short)((ulonglong)uVar36 >> 0x20)) - sVar39;
  sVar21 = (sVar15 + (short)((ulonglong)uVar36 >> 0x30)) - sVar42;
  uVar49 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar33 - sVar24),CONCAT22(uVar5,sVar31 - sVar22)),
                   DAT_005a9c60);
  uVar36 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar40 - sVar30,CONCAT22(uVar3,sVar37 - sVar28))),
                   DAT_005a9c60);
  uVar17 = param_2[0xc];
  uVar47 = param_2[4];
  uVar36 = packssdw(CONCAT44((int)((longlong)uVar49 >> 0x28),(int)uVar49 >> 8),
                    CONCAT44((int)((longlong)uVar36 >> 0x28),(int)uVar36 >> 8));
  sVar22 = (short)uVar36 - sVar16;
  sVar24 = (short)((ulonglong)uVar36 >> 0x10) - sVar19;
  sVar28 = (short)((ulonglong)uVar36 >> 0x20) - sVar20;
  sVar30 = (short)((ulonglong)uVar36 >> 0x30) - sVar21;
  sVar9 = ((short)uVar12 - (short)uVar18) + sVar22;
  sVar13 = ((short)((ulonglong)uVar12 >> 0x10) - sVar13) + sVar24;
  sVar14 = ((short)((ulonglong)uVar12 >> 0x20) - sVar14) + sVar28;
  sVar15 = ((short)((ulonglong)uVar12 >> 0x30) - sVar15) + sVar30;
  param_2[8] = CONCAT26(sVar29 + sVar15,
                        CONCAT24(sVar27 + sVar14,CONCAT22(sVar26 + sVar13,sVar25 + sVar9)));
  sVar31 = (short)((ulonglong)uVar47 >> 0x10);
  sVar33 = (short)((ulonglong)uVar47 >> 0x20);
  sVar37 = (short)((ulonglong)uVar47 >> 0x30);
  param_2[6] = CONCAT26(sVar29 - sVar15,
                        CONCAT24(sVar27 - sVar14,CONCAT22(sVar26 - sVar13,sVar25 - sVar9)));
  uVar12 = *param_2;
  *param_2 = CONCAT26(sVar37 + sVar42,
                      CONCAT24(sVar33 + sVar39,CONCAT22(sVar31 + sVar34,(short)uVar47 + sVar32)));
  sVar9 = (short)((ulonglong)uVar17 >> 0x10);
  sVar13 = (short)((ulonglong)uVar17 >> 0x20);
  sVar14 = (short)((ulonglong)uVar17 >> 0x30);
  param_2[0xe] = CONCAT26(sVar37 - sVar42,
                          CONCAT24(sVar33 - sVar39,CONCAT22(sVar31 - sVar34,(short)uVar47 - sVar32))
                         );
  param_2[2] = CONCAT26(sVar14 + sVar21,
                        CONCAT24(sVar13 + sVar20,CONCAT22(sVar9 + sVar19,(short)uVar17 + sVar16)));
  param_2[0xc] = CONCAT26(sVar14 - sVar21,
                          CONCAT24(sVar13 - sVar20,CONCAT22(sVar9 - sVar19,(short)uVar17 - sVar16)))
  ;
  sVar9 = (short)((ulonglong)uVar12 >> 0x10);
  sVar13 = (short)((ulonglong)uVar12 >> 0x20);
  sVar14 = (short)((ulonglong)uVar12 >> 0x30);
  uVar17 = param_2[5];
  param_2[4] = CONCAT26(sVar14 + sVar30,
                        CONCAT24(sVar13 + sVar28,CONCAT22(sVar9 + sVar24,(short)uVar12 + sVar22)));
  param_2[10] = CONCAT26(sVar14 - sVar30,
                         CONCAT24(sVar13 - sVar28,CONCAT22(sVar9 - sVar24,(short)uVar12 - sVar22)));
  uVar47 = param_2[0xd];
  uVar12 = param_2[1];
  sVar9 = (short)((ulonglong)uVar17 >> 0x10);
  sVar15 = (short)((ulonglong)uVar47 >> 0x10);
  sVar13 = (short)((ulonglong)uVar17 >> 0x20);
  sVar14 = (short)((ulonglong)uVar17 >> 0x30);
  sVar22 = (short)((ulonglong)uVar47 >> 0x20);
  sVar24 = (short)((ulonglong)uVar47 >> 0x30);
  uVar18 = param_2[9];
  sVar16 = (short)uVar17 + (short)uVar47;
  sVar19 = sVar9 + sVar15;
  sVar20 = sVar13 + sVar22;
  sVar21 = sVar14 + sVar24;
  uVar17 = pmaddwd(CONCAT44(CONCAT22((short)((uint)(undefined4)DAT_005a9c80 >> 0x10),sVar9 - sVar15)
                            ,CONCAT22((short)(undefined4)DAT_005a9c80,(short)uVar17 - (short)uVar47)
                           ),DAT_005a9c60);
  uVar47 = pmaddwd(CONCAT26((short)((uint)DAT_005a9c80._4_4_ >> 0x10),
                            CONCAT24(sVar14 - sVar24,
                                     CONCAT22((short)DAT_005a9c80._4_4_,sVar13 - sVar22))),
                   DAT_005a9c60);
  sVar22 = (short)uVar12 + (short)uVar18;
  sVar25 = (short)((ulonglong)uVar12 >> 0x10);
  sVar9 = (short)((ulonglong)uVar18 >> 0x10);
  sVar24 = sVar25 + sVar9;
  sVar27 = (short)((ulonglong)uVar12 >> 0x20);
  sVar28 = (short)((ulonglong)uVar12 >> 0x30);
  sVar13 = (short)((ulonglong)uVar18 >> 0x20);
  sVar26 = sVar27 + sVar13;
  sVar14 = (short)((ulonglong)uVar18 >> 0x30);
  sVar29 = sVar28 + sVar14;
  sVar30 = (short)uVar12 - (short)uVar18;
  sVar25 = sVar25 - sVar9;
  sVar27 = sVar27 - sVar13;
  sVar28 = sVar28 - sVar14;
  uVar17 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  sVar9 = (short)uVar17 - sVar16;
  sVar13 = (short)((ulonglong)uVar17 >> 0x10) - sVar19;
  sVar14 = (short)((ulonglong)uVar17 >> 0x20) - sVar20;
  sVar15 = (short)((ulonglong)uVar17 >> 0x30) - sVar21;
  param_2[5] = CONCAT26(sVar29 + sVar21,
                        CONCAT24(sVar26 + sVar20,CONCAT22(sVar24 + sVar19,sVar22 + sVar16)));
  param_2[0xd] = CONCAT26(sVar28 + sVar15,
                          CONCAT24(sVar27 + sVar14,CONCAT22(sVar25 + sVar13,sVar30 + sVar9)));
  sVar22 = sVar22 - sVar16;
  sVar24 = sVar24 - sVar19;
  sVar26 = sVar26 - sVar20;
  sVar29 = sVar29 - sVar21;
  param_2[9] = CONCAT26(sVar28 - sVar15,
                        CONCAT24(sVar27 - sVar14,CONCAT22(sVar25 - sVar13,sVar30 - sVar9)));
  uVar17 = param_2[0xb];
  uVar47 = param_2[7];
  uVar12 = param_2[0xf];
  uVar18 = param_2[3];
  sVar9 = (short)uVar17 + (short)uVar47;
  sVar14 = (short)((ulonglong)uVar17 >> 0x10);
  sVar25 = (short)((ulonglong)uVar47 >> 0x10);
  sVar13 = sVar14 + sVar25;
  sVar16 = (short)((ulonglong)uVar17 >> 0x20);
  sVar20 = (short)((ulonglong)uVar17 >> 0x30);
  sVar27 = (short)((ulonglong)uVar47 >> 0x20);
  sVar15 = sVar16 + sVar27;
  sVar28 = (short)((ulonglong)uVar47 >> 0x30);
  sVar19 = sVar20 + sVar28;
  sVar21 = (short)uVar17 - (short)uVar47;
  sVar14 = sVar14 - sVar25;
  sVar16 = sVar16 - sVar27;
  sVar20 = sVar20 - sVar28;
  uVar6 = (undefined2)((uint)(undefined4)DAT_005a9c80 >> 0x10);
  uVar5 = (undefined2)(undefined4)DAT_005a9c80;
  uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar14),CONCAT22(uVar5,sVar21)),DAT_005a9c68);
  uVar3 = (undefined2)DAT_005a9c80._4_4_;
  uVar4 = (undefined2)((uint)DAT_005a9c80._4_4_ >> 0x10);
  uVar47 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar20,CONCAT22(uVar3,sVar16))),DAT_005a9c68);
  sVar30 = (short)uVar18 + (short)uVar12;
  sVar25 = (short)((ulonglong)uVar12 >> 0x10);
  sVar32 = (short)((ulonglong)uVar18 >> 0x10);
  sVar31 = sVar32 + sVar25;
  sVar27 = (short)((ulonglong)uVar12 >> 0x20);
  sVar34 = (short)((ulonglong)uVar18 >> 0x20);
  sVar41 = (short)((ulonglong)uVar18 >> 0x30);
  sVar33 = sVar34 + sVar27;
  sVar28 = (short)((ulonglong)uVar12 >> 0x30);
  sVar37 = sVar41 + sVar28;
  sVar44 = (short)uVar18 - (short)uVar12;
  sVar32 = sVar32 - sVar25;
  sVar34 = sVar34 - sVar27;
  sVar41 = sVar41 - sVar28;
  sVar39 = sVar30 + sVar9;
  sVar40 = sVar31 + sVar13;
  sVar42 = sVar33 + sVar15;
  sVar43 = sVar37 + sVar19;
  uVar47 = packssdw(CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8),
                    CONCAT44((int)((longlong)uVar47 >> 0x28),(int)uVar47 >> 8));
  uVar12 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar31 - sVar13),CONCAT22(uVar5,sVar30 - sVar9)),
                   DAT_005a9c60);
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar37 - sVar19,CONCAT22(uVar3,sVar33 - sVar15))),
                   DAT_005a9c60);
  uVar36 = packssdw(CONCAT44((int)((longlong)uVar12 >> 0x28),(int)uVar12 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  uVar12 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar14 + sVar32),CONCAT22(uVar5,sVar21 + sVar44)),
                   DAT_005a9c78);
  uVar17 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar20 + sVar41,CONCAT22(uVar3,sVar16 + sVar34))),
                   DAT_005a9c78);
  uVar18 = packssdw(CONCAT44((int)((longlong)uVar12 >> 0x28),(int)uVar12 >> 8),
                    CONCAT44((int)((longlong)uVar17 >> 0x28),(int)uVar17 >> 8));
  uVar49 = pmaddwd(CONCAT44(CONCAT22(uVar6,sVar32),CONCAT22(uVar5,sVar44)),DAT_005a9c70);
  uVar12 = pmaddwd(CONCAT26(uVar4,CONCAT24(sVar41,CONCAT22(uVar3,sVar34))),DAT_005a9c70);
  uVar17 = param_2[5];
  uVar12 = packssdw(CONCAT44((int)((longlong)uVar49 >> 0x28),(int)uVar49 >> 8),
                    CONCAT44((int)((longlong)uVar12 >> 0x28),(int)uVar12 >> 8));
  sVar13 = (short)((ulonglong)uVar18 >> 0x10);
  sVar15 = (short)((ulonglong)uVar18 >> 0x20);
  sVar19 = (short)((ulonglong)uVar18 >> 0x30);
  sVar20 = ((short)uVar18 + (short)uVar47) - sVar39;
  sVar21 = (sVar13 + (short)((ulonglong)uVar47 >> 0x10)) - sVar40;
  sVar27 = (sVar15 + (short)((ulonglong)uVar47 >> 0x20)) - sVar42;
  sVar30 = (sVar19 + (short)((ulonglong)uVar47 >> 0x30)) - sVar43;
  sVar32 = (short)uVar36 - sVar20;
  sVar33 = (short)((ulonglong)uVar36 >> 0x10) - sVar21;
  sVar34 = (short)((ulonglong)uVar36 >> 0x20) - sVar27;
  sVar37 = (short)((ulonglong)uVar36 >> 0x30) - sVar30;
  sVar9 = (short)((ulonglong)uVar17 >> 0x10);
  sVar14 = (short)((ulonglong)uVar17 >> 0x20);
  sVar16 = (short)((ulonglong)uVar17 >> 0x30);
  uVar47 = param_2[0xd];
  sVar41 = ((short)uVar12 - (short)uVar18) + sVar32;
  sVar44 = ((short)((ulonglong)uVar12 >> 0x10) - sVar13) + sVar33;
  sVar45 = ((short)((ulonglong)uVar12 >> 0x20) - sVar15) + sVar34;
  sVar48 = ((short)((ulonglong)uVar12 >> 0x30) - sVar19) + sVar37;
  param_2[1] = CONCAT26(sVar16 + sVar43,
                        CONCAT24(sVar14 + sVar42,CONCAT22(sVar9 + sVar40,(short)uVar17 + sVar39)));
  uVar12 = param_2[9];
  param_2[7] = CONCAT26(sVar29 - sVar48,
                        CONCAT24(sVar26 - sVar45,CONCAT22(sVar24 - sVar44,sVar22 - sVar41)));
  sVar25 = (short)((ulonglong)uVar47 >> 0x10);
  sVar28 = (short)((ulonglong)uVar47 >> 0x20);
  sVar31 = (short)((ulonglong)uVar47 >> 0x30);
  param_2[3] = CONCAT26(sVar31 + sVar30,
                        CONCAT24(sVar28 + sVar27,CONCAT22(sVar25 + sVar21,(short)uVar47 + sVar20)));
  sVar13 = (short)((ulonglong)uVar12 >> 0x10);
  sVar15 = (short)((ulonglong)uVar12 >> 0x20);
  sVar19 = (short)((ulonglong)uVar12 >> 0x30);
  param_2[5] = CONCAT26(sVar19 + sVar37,
                        CONCAT24(sVar15 + sVar34,CONCAT22(sVar13 + sVar33,(short)uVar12 + sVar32)));
  iVar7 = param_4[4] + param_5;
  uVar18 = *param_2;
  uVar36 = param_2[2];
  uVar38 = (ushort)(sVar22 + sVar41) >> 5;
  uVar11 = CONCAT24((ushort)((short)uVar47 - sVar20) >> 5,
                    CONCAT22((ushort)((short)uVar12 - sVar32) >> 5,uVar38));
  uVar46 = (ushort)((short)uVar17 - sVar39) >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  iVar7 = param_4[5] + param_5;
  uVar38 = (ushort)(sVar24 + sVar44) >> 5;
  uVar11 = CONCAT24((ushort)(sVar25 - sVar21) >> 5,CONCAT22((ushort)(sVar13 - sVar33) >> 5,uVar38));
  uVar46 = (ushort)(sVar9 - sVar40) >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  iVar7 = param_4[6] + param_5;
  uVar38 = (ushort)(sVar26 + sVar45) >> 5;
  uVar11 = CONCAT24((ushort)(sVar28 - sVar27) >> 5,CONCAT22((ushort)(sVar15 - sVar34) >> 5,uVar38));
  uVar46 = (ushort)(sVar14 - sVar42) >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  iVar7 = param_4[7] + param_5;
  uVar38 = (ushort)(sVar29 + sVar48) >> 5;
  uVar11 = CONCAT24((ushort)(sVar31 - sVar30) >> 5,CONCAT22((ushort)(sVar19 - sVar37) >> 5,uVar38));
  uVar46 = (ushort)(sVar16 - sVar43) >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(*param_4 + param_5);
  uVar17 = param_2[6];
  uVar47 = param_2[4];
  uVar38 = (ushort)uVar18 >> 5;
  uVar11 = CONCAT24((ushort)uVar47 >> 5,CONCAT22((ushort)uVar36 >> 5,uVar38));
  uVar46 = (ushort)uVar17 >> 5;
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[1] + param_5);
  uVar38 = (ushort)((ulonglong)uVar18 >> 0x10) >> 5;
  uVar11 = CONCAT24((ushort)((ulonglong)uVar47 >> 0x10) >> 5,
                    CONCAT22((ushort)((ulonglong)uVar36 >> 0x10) >> 5,uVar38));
  uVar46 = (ushort)((ulonglong)uVar17 >> 0x10) >> 5;
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[2] + param_5);
  uVar38 = (ushort)((ulonglong)uVar18 >> 0x20) >> 5;
  uVar11 = CONCAT24((ushort)((ulonglong)uVar47 >> 0x20) >> 5,
                    CONCAT22((ushort)((ulonglong)uVar36 >> 0x20) >> 5,uVar38));
  uVar46 = (ushort)((ulonglong)uVar17 >> 0x20) >> 5;
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[3] + param_5);
  uVar38 = (ushort)((ulonglong)uVar18 >> 0x35);
  uVar11 = CONCAT24((ushort)((ulonglong)uVar47 >> 0x35),
                    CONCAT22((ushort)((ulonglong)uVar36 >> 0x35),uVar38));
  uVar46 = (ushort)((ulonglong)uVar17 >> 0x35);
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[4] + param_5);
  uVar17 = param_2[1];
  uVar47 = param_2[3];
  uVar12 = param_2[5];
  uVar18 = param_2[7];
  uVar36 = param_2[8];
  uVar38 = (ushort)uVar17 >> 5;
  uVar11 = CONCAT24((ushort)uVar12 >> 5,CONCAT22((ushort)uVar47 >> 5,uVar38));
  uVar46 = (ushort)uVar18 >> 5;
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[5] + param_5);
  uVar49 = param_2[10];
  uVar38 = (ushort)((ulonglong)uVar17 >> 0x10) >> 5;
  uVar11 = CONCAT24((ushort)((ulonglong)uVar12 >> 0x10) >> 5,
                    CONCAT22((ushort)((ulonglong)uVar47 >> 0x10) >> 5,uVar38));
  uVar46 = (ushort)((ulonglong)uVar18 >> 0x10) >> 5;
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[6] + param_5);
  uVar1 = param_2[0xc];
  uVar2 = param_2[0xe];
  uVar38 = (ushort)((ulonglong)uVar17 >> 0x20) >> 5;
  uVar11 = CONCAT24((ushort)((ulonglong)uVar12 >> 0x20) >> 5,
                    CONCAT22((ushort)((ulonglong)uVar47 >> 0x20) >> 5,uVar38));
  uVar46 = (ushort)((ulonglong)uVar18 >> 0x20) >> 5;
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  puVar8 = (undefined1 *)(param_4[7] + param_5);
  uVar38 = (ushort)((ulonglong)uVar17 >> 0x35);
  uVar11 = CONCAT24((ushort)((ulonglong)uVar12 >> 0x35),
                    CONCAT22((ushort)((ulonglong)uVar47 >> 0x35),uVar38));
  uVar46 = (ushort)((ulonglong)uVar18 >> 0x35);
  *puVar8 = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  puVar8[1] = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  puVar8[2] = *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  puVar8[3] = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  iVar7 = *param_4 + param_5;
  uVar38 = (ushort)uVar36 >> 5;
  uVar11 = CONCAT24((ushort)uVar1 >> 5,CONCAT22((ushort)uVar49 >> 5,uVar38));
  uVar46 = (ushort)uVar2 >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  iVar7 = param_4[1] + param_5;
  uVar38 = (ushort)((ulonglong)uVar36 >> 0x10) >> 5;
  uVar11 = CONCAT24((ushort)((ulonglong)uVar1 >> 0x10) >> 5,
                    CONCAT22((ushort)((ulonglong)uVar49 >> 0x10) >> 5,uVar38));
  uVar46 = (ushort)((ulonglong)uVar2 >> 0x10) >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  iVar7 = param_4[2] + param_5;
  uVar38 = (ushort)((ulonglong)uVar36 >> 0x20) >> 5;
  uVar11 = CONCAT24((ushort)((ulonglong)uVar1 >> 0x20) >> 5,
                    CONCAT22((ushort)((ulonglong)uVar49 >> 0x20) >> 5,uVar38));
  uVar46 = (ushort)((ulonglong)uVar2 >> 0x20) >> 5;
  *(undefined1 *)(iVar7 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(iVar7 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(iVar7 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(iVar7 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  param_5 = param_4[3] + param_5;
  uVar38 = (ushort)((ulonglong)uVar36 >> 0x35);
  uVar11 = CONCAT24((ushort)((ulonglong)uVar1 >> 0x35),
                    CONCAT22((ushort)((ulonglong)uVar49 >> 0x35),uVar38));
  uVar46 = (ushort)((ulonglong)uVar2 >> 0x35);
  *(undefined1 *)(param_5 + 4) = *(undefined1 *)(param_6 + (uVar38 & 0x3ff));
  *(undefined1 *)(param_5 + 5) = *(undefined1 *)(param_6 + ((uint)((uint6)uVar11 >> 0x10) & 0x3ff));
  *(undefined1 *)(param_5 + 6) =
       *(undefined1 *)(param_6 + ((uint)(CONCAT26(uVar46,uVar11) >> 0x20) & 0x3ff));
  *(undefined1 *)(param_5 + 7) = *(undefined1 *)(param_6 + (uVar46 & 0x3ff));
  return;
}

