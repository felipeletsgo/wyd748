// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058b418 | Name: FUN_0058b418


void FUN_0058b418(int param_1,int param_2,int param_3,int param_4,int param_5)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  undefined1 *puVar5;
  short *psVar6;
  int *piVar7;
  int iVar8;
  int iVar9;
  undefined1 uVar10;
  ushort *puVar11;
  int iVar12;
  int local_9c;
  int local_98;
  int local_94;
  int local_90 [3];
  int local_84;
  int local_80 [25];
  uint local_1c;
  ushort *local_18;
  int *local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  iVar8 = *(int *)(param_1 + 0x11c) + 0x80;
  puVar11 = (ushort *)(param_3 + 0x60);
  piVar7 = local_80 + 1;
  local_10 = iVar8;
  param_1 = 8;
  psVar6 = (short *)(*(int *)(param_2 + 0x50) + 0x60);
  do {
    if (param_1 != 4) {
      uVar1 = *puVar11;
      uVar2 = puVar11[-0x18];
      local_8 = CONCAT22(local_8._2_2_,uVar2);
      uVar3 = puVar11[-0x28];
      local_1c = (uint)uVar1;
      local_c = (uint)uVar3;
      if (((((uVar1 == 0 && uVar3 == 0) && uVar2 == 0) && puVar11[-8] == 0) && puVar11[8] == 0) &&
          puVar11[-0x20] == 0) {
        iVar4 = (int)psVar6[-0x30] * (int)(short)puVar11[-0x30] * 4;
        piVar7[-8] = iVar4;
        *piVar7 = iVar4;
        piVar7[0x10] = iVar4;
      }
      else {
        iVar12 = (short)(psVar6[-0x20] * puVar11[-0x20]) * 0x3b21 +
                 (short)(*psVar6 * uVar1) * -0x187e;
        iVar4 = (int)psVar6[-0x30] * (int)(short)puVar11[-0x30] * 0x4000;
        iVar8 = iVar12 + iVar4;
        iVar4 = iVar4 - iVar12;
        param_2._0_2_ = psVar6[-8] * puVar11[-8];
        local_c = (int)psVar6[-0x28] * (int)(short)uVar3;
        local_c._0_2_ = (short)((int)psVar6[-0x28] * (int)(short)uVar3);
        local_8 = (int)(short)local_c;
        iVar12 = (int)(short)(psVar6[8] * puVar11[8]);
        iVar9 = (short)param_2 * 0x2e75 + local_8 * 0x21f9 + iVar12 * -0x6c2 +
                (short)(psVar6[-0x18] * uVar2) * -0x4587;
        iVar12 = (short)(psVar6[-0x18] * uVar2) * 0x1ccd + (short)param_2 * -0x133e +
                 iVar12 * -0x1050 + local_8 * 0x5203;
        local_18 = puVar11;
        piVar7[-8] = iVar8 + 0x800 + iVar12 >> 0xc;
        piVar7[0x10] = (iVar8 - iVar12) + 0x800 >> 0xc;
        iVar8 = local_10;
        *piVar7 = iVar4 + 0x800 + iVar9 >> 0xc;
        iVar4 = (iVar4 - iVar9) + 0x800 >> 0xc;
        puVar11 = local_18;
      }
      piVar7[8] = iVar4;
    }
    puVar11 = puVar11 + 1;
    piVar7 = piVar7 + 1;
    psVar6 = psVar6 + 1;
    param_1 = param_1 + -1;
    local_18 = puVar11;
    local_14 = piVar7;
  } while (0 < param_1);
  param_1 = 0;
  piVar7 = &local_9c;
  do {
    puVar5 = (undefined1 *)(*(int *)(param_4 + param_1 * 4) + param_5);
    if (((((piVar7[1] == 0 && piVar7[3] == 0) && piVar7[5] == 0) && piVar7[7] == 0) &&
        piVar7[6] == 0) && piVar7[2] == 0) {
      uVar10 = *(undefined1 *)((*piVar7 + 0x10 >> 5 & 0x3ffU) + iVar8);
      *puVar5 = uVar10;
      puVar5[1] = uVar10;
      puVar5[3] = uVar10;
    }
    else {
      iVar4 = (short)piVar7[2] * 0x3b21 + (short)piVar7[6] * -0x187e;
      iVar8 = iVar4 + *piVar7 * 0x4000;
      local_c = *piVar7 * 0x4000 - iVar4;
      iVar4 = (int)(short)piVar7[5];
      local_8 = (int)(short)piVar7[1];
      iVar12 = iVar4 * 0x2e75 + local_8 * 0x21f9 + (short)piVar7[7] * -0x6c2 +
               (short)piVar7[3] * -0x4587;
      iVar4 = (short)piVar7[3] * 0x1ccd + iVar4 * -0x133e + (short)piVar7[7] * -0x1050 +
              local_8 * 0x5203;
      *puVar5 = *(undefined1 *)((iVar8 + 0x40000 + iVar4 >> 0x13 & 0x3ffU) + local_10);
      puVar5[3] = *(undefined1 *)(((iVar8 - iVar4) + 0x40000 >> 0x13 & 0x3ffU) + local_10);
      puVar5[1] = *(undefined1 *)(((int)(local_c + 0x40000 + iVar12) >> 0x13 & 0x3ffU) + local_10);
      uVar10 = *(undefined1 *)(((int)((local_c - iVar12) + 0x40000) >> 0x13 & 0x3ffU) + local_10);
      iVar8 = local_10;
    }
    piVar7 = piVar7 + 8;
    param_1 = param_1 + 1;
    puVar5[2] = uVar10;
  } while (param_1 < 4);
  return;
}

