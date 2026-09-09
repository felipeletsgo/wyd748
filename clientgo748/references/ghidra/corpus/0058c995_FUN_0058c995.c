// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058c995 | Name: FUN_0058c995


void FUN_0058c995(short *param_1,short *param_2,int *param_3,int param_4,int param_5,int param_6)

{
  undefined1 uVar1;
  undefined1 uVar2;
  undefined1 uVar3;
  undefined1 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int *piVar18;
  undefined4 *puVar19;
  short *local_48;
  short *local_44;
  int *local_40;
  int local_3c;
  int local_8;
  
  local_48 = param_1;
  local_44 = param_2;
  local_40 = param_3;
  local_3c = 8;
  do {
    iVar15 = ((int)local_48[0x10] * (int)local_44[0x10] + (int)local_48[0x30] * (int)local_44[0x30])
             * 0x1151;
    iVar8 = (int)local_48[0x30] * (int)local_44[0x30] * -0x3b21 + iVar15;
    iVar15 = (int)local_48[0x10] * (int)local_44[0x10] * 0x187e + iVar15;
    iVar9 = ((int)local_48[0x20] * (int)local_44[0x20] + (int)*local_48 * (int)*local_44) * 0x2000;
    iVar5 = ((int)*local_48 * (int)*local_44 - (int)local_48[0x20] * (int)local_44[0x20]) * 0x2000;
    iVar10 = iVar9 + iVar15;
    iVar6 = iVar5 + iVar8;
    iVar9 = iVar9 - iVar15;
    iVar5 = iVar5 - iVar8;
    iVar15 = (int)local_48[0x38] * (int)local_44[0x38];
    iVar11 = (int)local_48[0x18] * (int)local_44[0x18];
    iVar16 = (int)local_48[8] * (int)local_44[8];
    iVar8 = (int)local_48[0x28] * (int)local_44[0x28];
    iVar7 = (iVar15 + iVar16) * -0x1ccd;
    iVar13 = (iVar8 + iVar11) * -0x5203;
    iVar17 = (iVar15 + iVar11 + iVar8 + iVar16) * 0x25a1;
    iVar12 = (iVar15 + iVar11) * -0x3ec5 + iVar17;
    iVar17 = (iVar8 + iVar16) * -0xc7c + iVar17;
    iVar15 = iVar7 + iVar12 + iVar15 * 0x98e;
    iVar14 = iVar13 + iVar17 + iVar8 * 0x41b3;
    iVar8 = iVar12 + iVar13 + iVar11 * 0x6254;
    iVar7 = iVar7 + iVar17 + iVar16 * 0x300b;
    local_40[0x20] = (iVar9 - iVar15) + 0x400 >> 0xb;
    local_40[0x18] = iVar15 + iVar9 + 0x400 >> 0xb;
    local_40[0x28] = (iVar5 - iVar14) + 0x400 >> 0xb;
    local_40[0x30] = (iVar6 - iVar8) + 0x400 >> 0xb;
    local_40[0x10] = iVar14 + iVar5 + 0x400 >> 0xb;
    local_40[8] = iVar8 + iVar6 + 0x400 >> 0xb;
    local_40[0x38] = (iVar10 - iVar7) + 0x400 >> 0xb;
    *local_40 = iVar10 + iVar7 + 0x400 >> 0xb;
    local_48 = local_48 + 1;
    local_40 = local_40 + 1;
    local_44 = local_44 + 1;
    local_3c = local_3c + -1;
  } while (local_3c != 0);
  local_3c = 8;
  local_8 = 0;
  local_40 = param_3;
  do {
    piVar18 = (int *)(param_4 + local_8);
    local_8 = local_8 + 4;
    puVar19 = (undefined4 *)(*piVar18 + param_5);
    iVar15 = (local_40[2] + local_40[6]) * 0x1151;
    iVar5 = local_40[6] * -0x3b21 + iVar15;
    iVar15 = local_40[2] * 0x187e + iVar15;
    iVar11 = (local_40[4] + *local_40) * 0x2000;
    iVar7 = (*local_40 - local_40[4]) * 0x2000;
    iVar12 = iVar11 + iVar15;
    iVar9 = iVar7 + iVar5;
    iVar11 = iVar11 - iVar15;
    iVar7 = iVar7 - iVar5;
    iVar15 = local_40[7];
    iVar5 = local_40[3];
    iVar6 = local_40[1];
    iVar8 = local_40[5];
    iVar10 = (iVar15 + iVar6) * -0x1ccd;
    iVar14 = (iVar8 + iVar5) * -0x5203;
    iVar16 = (iVar15 + iVar5 + iVar8 + iVar6) * 0x25a1;
    iVar13 = (iVar15 + iVar5) * -0x3ec5 + iVar16;
    iVar16 = (iVar8 + iVar6) * -0xc7c + iVar16;
    iVar15 = iVar10 + iVar13 + iVar15 * 0x98e;
    iVar8 = iVar14 + iVar16 + iVar8 * 0x41b3;
    iVar5 = iVar13 + iVar14 + iVar5 * 0x6254;
    iVar6 = iVar10 + iVar16 + iVar6 * 0x300b;
    uVar1 = *(undefined1 *)(param_6 + ((iVar12 - iVar6) + 0x20000 >> 0x12 & 0x3ffU));
    uVar2 = *(undefined1 *)(param_6 + ((iVar9 - iVar5) + 0x20000 >> 0x12 & 0x3ffU));
    uVar3 = *(undefined1 *)(param_6 + ((iVar7 - iVar8) + 0x20000 >> 0x12 & 0x3ffU));
    uVar4 = *(undefined1 *)(param_6 + ((iVar11 - iVar15) + 0x20000 >> 0x12 & 0x3ffU));
    *puVar19 = CONCAT31(CONCAT21(CONCAT11(*(undefined1 *)
                                           (param_6 + (iVar11 + iVar15 + 0x20000 >> 0x12 & 0x3ffU)),
                                          *(undefined1 *)
                                           (param_6 + (iVar7 + iVar8 + 0x20000 >> 0x12 & 0x3ffU))),
                                 *(undefined1 *)
                                  (param_6 + (iVar9 + iVar5 + 0x20000 >> 0x12 & 0x3ffU))),
                        *(undefined1 *)(param_6 + (iVar12 + iVar6 + 0x20000 >> 0x12 & 0x3ffU)));
    puVar19[1] = CONCAT31(CONCAT21(CONCAT11(uVar1,uVar2),uVar3),uVar4);
    local_40 = local_40 + 8;
    local_3c = local_3c + -1;
  } while (local_3c != 0);
  return;
}

