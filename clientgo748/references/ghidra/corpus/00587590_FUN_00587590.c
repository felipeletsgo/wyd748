// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587590 | Name: FUN_00587590


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00587590(uint param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,
                 uint *param_5)

{
  undefined4 uVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  ushort uVar6;
  undefined7 uVar7;
  byte bVar8;
  undefined2 uVar9;
  undefined8 uVar10;
  undefined4 uVar11;
  undefined8 uVar12;
  undefined2 uVar14;
  undefined8 uVar13;
  uint uVar15;
  undefined2 uVar20;
  undefined8 uVar16;
  undefined2 uVar19;
  undefined2 uVar21;
  undefined8 uVar17;
  ulonglong uVar18;
  undefined8 uVar22;
  undefined2 uVar24;
  undefined2 uVar25;
  undefined8 uVar23;
  undefined8 uVar26;
  
  param_1 = param_1 >> 2;
  do {
    uVar11 = *param_2;
    uVar1 = *param_3;
    uVar6 = (ushort)(((uint7)(byte)((uint)uVar11 >> 0x18) << 0x30) >> 0x28);
    uVar7 = CONCAT25(uVar6,CONCAT14((char)((uint)uVar11 >> 0x10),uVar11));
    uVar18 = (ulonglong)
             CONCAT41((int)(CONCAT34((int3)((uint7)uVar7 >> 0x20),uVar11) >> 0x18),
                      (char)((uint)uVar11 >> 8)) & 0xffffffff00ff;
    bVar8 = (byte)uVar11;
    uVar11 = *param_4;
    uVar12 = psubsw((ulonglong)
                    CONCAT52((int5)(CONCAT43((int)(CONCAT34((int3)(CONCAT25((short)(((uint7)(byte)((
                                                  uint)uVar1 >> 0x18) << 0x30) >> 0x28),
                                                  CONCAT14((char)((uint)uVar1 >> 0x10),uVar1)) >>
                                                  0x20),uVar1) >> 0x18),
                                             CONCAT12((char)((uint)uVar1 >> 8),(short)uVar1)) >>
                                   0x10),(short)uVar1) & 0xffffffff00ff00ff,_PTR_DAT_005c8100);
    uVar16 = psubsw((ulonglong)
                    CONCAT52(CONCAT41((int)(CONCAT34((int3)(CONCAT25((short)(((uint7)(byte)((uint)
                                                  uVar11 >> 0x18) << 0x30) >> 0x28),
                                                  CONCAT14((char)((uint)uVar11 >> 0x10),uVar11)) >>
                                                  0x20),uVar11) >> 0x18),(char)((uint)uVar11 >> 8)),
                             (ushort)(byte)uVar11) & 0xffffffff00ffffff,_PTR_DAT_005c8100);
    uVar19 = (undefined2)((ulonglong)uVar16 >> 0x10);
    uVar14 = (undefined2)((ulonglong)uVar12 >> 0x10);
    uVar13 = pmaddwd(CONCAT44(CONCAT22(uVar19,uVar14),CONCAT22((short)uVar16,(short)uVar12)),
                     _DAT_005c8108);
    uVar10 = psllw(CONCAT62((int6)uVar18,(ushort)bVar8),8);
    uVar24 = (undefined2)((ulonglong)uVar12 >> 0x20);
    uVar20 = (undefined2)((ulonglong)uVar16 >> 0x20);
    uVar25 = (undefined2)((ulonglong)uVar12 >> 0x30);
    uVar21 = (undefined2)((ulonglong)uVar16 >> 0x30);
    uVar23 = pmaddwd(CONCAT26(uVar21,CONCAT24(uVar25,CONCAT22(uVar20,uVar24))),_DAT_005c8108);
    uVar9 = (undefined2)uVar18;
    uVar16 = pmaddwd(CONCAT44(CONCAT22(uVar9,uVar19),(uint)CONCAT12(bVar8,(short)uVar16)),
                     _PTR_DAT_005c8110);
    uVar22 = pmaddwd(CONCAT44(CONCAT22(uVar9,uVar14),(uint)CONCAT12(bVar8,(short)uVar12)),
                     _PTR_DAT_005c8118);
    uVar9 = (undefined2)((uint7)uVar7 >> 0x20);
    uVar6 = uVar6 >> 8;
    uVar26 = pmaddwd(CONCAT26(uVar6,CONCAT24(uVar25,CONCAT22(uVar9,uVar24))),_PTR_DAT_005c8118);
    uVar12 = pmaddwd(CONCAT26(uVar6,CONCAT24(uVar21,CONCAT22(uVar9,uVar20))),_PTR_DAT_005c8110);
    uVar17 = packssdw(CONCAT44((int)((int)uVar13 + (uint)(ushort)uVar10) >> 8,(int)uVar16 >> 8),
                      CONCAT44(DAT_013b7fa0,(int)uVar22 >> 8));
    uVar16 = packssdw(CONCAT44((int)((int)((ulonglong)uVar13 >> 0x20) +
                                    (uint)(ushort)((ulonglong)uVar10 >> 0x10)) >> 8,
                               (int)((longlong)uVar16 >> 0x28)),
                      CONCAT44(DAT_013b7fa4,(int)((longlong)uVar22 >> 0x28)));
    sVar2 = (short)uVar17;
    sVar3 = (short)((ulonglong)uVar17 >> 0x10);
    sVar4 = (short)((ulonglong)uVar17 >> 0x20);
    sVar5 = (short)((ulonglong)uVar17 >> 0x30);
    uVar15 = CONCAT13((0 < sVar5) * (sVar5 < 0x100) * (char)((ulonglong)uVar17 >> 0x30) -
                      (0xff < sVar5),
                      CONCAT12((0 < sVar4) * (sVar4 < 0x100) * (char)((ulonglong)uVar17 >> 0x20) -
                               (0xff < sVar4),
                               CONCAT11((0 < sVar3) * (sVar3 < 0x100) *
                                        (char)((ulonglong)uVar17 >> 0x10) - (0xff < sVar3),
                                        (0 < sVar2) * (sVar2 < 0x100) * (char)uVar17 -
                                        (0xff < sVar2))));
    sVar2 = (short)uVar16;
    sVar3 = (short)((ulonglong)uVar16 >> 0x10);
    sVar4 = (short)((ulonglong)uVar16 >> 0x20);
    uVar13 = packssdw(CONCAT44((int)((int)uVar23 + (uint)(ushort)((ulonglong)uVar10 >> 0x20)) >> 8,
                               (int)uVar12 >> 8),CONCAT44(DAT_013b7fa0,(int)uVar26 >> 8));
    uVar18 = CONCAT16((0 < sVar4) * (sVar4 < 0x100) * (char)((ulonglong)uVar16 >> 0x20) -
                      (0xff < sVar4),
                      CONCAT15((0 < sVar3) * (sVar3 < 0x100) * (char)((ulonglong)uVar16 >> 0x10) -
                               (0xff < sVar3),
                               CONCAT14((0 < sVar2) * (sVar2 < 0x100) * (char)uVar16 -
                                        (0xff < sVar2),uVar15))) & _DAT_005c8120;
    uVar10 = packssdw(CONCAT44((int)((int)((ulonglong)uVar23 >> 0x20) +
                                    (uint)(ushort)((ulonglong)uVar10 >> 0x30)) >> 8,
                               (int)((longlong)uVar12 >> 0x28)),
                      CONCAT44(DAT_013b7fa4,(int)((longlong)uVar26 >> 0x28)));
    sVar2 = (short)uVar13;
    sVar3 = (short)((ulonglong)uVar13 >> 0x10);
    uVar9 = CONCAT11((0 < sVar3) * (sVar3 < 0x100) * (char)((ulonglong)uVar13 >> 0x10) -
                     (0xff < sVar3),(0 < sVar2) * (sVar2 < 0x100) * (char)uVar13 - (0xff < sVar2));
    sVar2 = (short)((ulonglong)uVar13 >> 0x20);
    sVar3 = (short)((ulonglong)uVar13 >> 0x30);
    uVar11 = CONCAT13((0 < sVar3) * (sVar3 < 0x100) * (char)((ulonglong)uVar13 >> 0x30) -
                      (0xff < sVar3),
                      CONCAT12((0 < sVar2) * (sVar2 < 0x100) * (char)((ulonglong)uVar13 >> 0x20) -
                               (0xff < sVar2),uVar9));
    sVar2 = (short)uVar10;
    sVar3 = (short)((ulonglong)uVar10 >> 0x10);
    sVar4 = (short)((ulonglong)uVar10 >> 0x20);
    param_2 = param_2 + 1;
    *param_5 = uVar15 | (uint)(uVar18 >> 8);
    param_4 = param_4 + 1;
    param_5[1] = CONCAT22(uVar9,(short)(uVar18 >> 0x28));
    param_3 = param_3 + 1;
    param_5[2] = (uint)(CONCAT16((0 < sVar4) * (sVar4 < 0x100) * (char)((ulonglong)uVar10 >> 0x20) -
                                 (0xff < sVar4),
                                 CONCAT15((0 < sVar3) * (sVar3 < 0x100) *
                                          (char)((ulonglong)uVar10 >> 0x10) - (0xff < sVar3),
                                          CONCAT14((0 < sVar2) * (sVar2 < 0x100) * (char)uVar10 -
                                                   (0xff < sVar2),uVar11))) >> 0x18) |
                 (uint)(((ulonglong)(ushort)((uint)uVar11 >> 0x10) << 0x30) >> 0x30);
    param_1 = param_1 - 1;
    param_5 = param_5 + 3;
  } while (param_1 != 0);
  return;
}

