// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058d4af | Name: FUN_0058d4af


void FUN_0058d4af(short *param_1,short *param_2,short *param_3,int param_4,int param_5,int param_6)

{
  short sVar1;
  undefined1 uVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  short sVar7;
  short sVar8;
  short sVar9;
  short sVar10;
  short sVar11;
  undefined1 *puVar12;
  short *local_30;
  short *local_2c;
  short *local_28;
  int local_24;
  short local_20;
  
  local_30 = param_1;
  local_2c = param_3;
  local_28 = param_2;
  local_20 = 8;
  do {
    sVar3 = *local_30 * *local_2c + local_30[0x20] * local_2c[0x20];
    sVar1 = *local_30 * *local_2c - local_30[0x20] * local_2c[0x20];
    sVar8 = local_30[0x10] * local_2c[0x10] + local_30[0x30] * local_2c[0x30];
    sVar4 = sVar3 + sVar8;
    sVar3 = sVar3 - sVar8;
    sVar8 = (short)((uint)((short)(local_30[0x10] * local_2c[0x10] - local_30[0x30] * local_2c[0x30]
                                  ) * 0x16a) >> 8) - sVar8;
    sVar5 = sVar1 + sVar8;
    sVar1 = sVar1 - sVar8;
    sVar6 = local_30[8] * local_2c[8] + local_30[0x38] * local_2c[0x38];
    sVar8 = local_30[8] * local_2c[8] - local_30[0x38] * local_2c[0x38];
    sVar9 = local_30[0x28] * local_2c[0x28] + local_30[0x18] * local_2c[0x18];
    sVar10 = local_30[0x28] * local_2c[0x28] - local_30[0x18] * local_2c[0x18];
    sVar7 = sVar6 + sVar9;
    sVar11 = (short)((uint)((short)(sVar10 + sVar8) * 0x1d9) >> 8);
    sVar10 = ((short)((uint)(sVar10 * -0x29d) >> 8) + sVar11) - sVar7;
    sVar6 = (short)((uint)((short)(sVar6 - sVar9) * 0x16a) >> 8) - sVar10;
    sVar8 = ((short)((uint)(sVar8 * 0x115) >> 8) - sVar11) + sVar6;
    *local_28 = sVar7 + sVar4;
    local_28[0x38] = sVar4 - sVar7;
    local_28[8] = sVar10 + sVar5;
    local_28[0x30] = sVar5 - sVar10;
    local_28[0x10] = sVar6 + sVar1;
    local_28[0x28] = sVar1 - sVar6;
    local_28[0x20] = sVar8 + sVar3;
    local_28[0x18] = sVar3 - sVar8;
    local_30 = local_30 + 1;
    local_2c = local_2c + 1;
    local_28 = local_28 + 1;
    local_20 = local_20 + -1;
  } while (local_20 != 0);
  local_24 = 0;
  local_20 = 8;
  local_28 = param_2;
  do {
    puVar12 = (undefined1 *)(*(int *)(param_4 + local_24) + param_5);
    local_24 = local_24 + 4;
    sVar3 = *local_28 + local_28[4];
    sVar1 = *local_28 - local_28[4];
    sVar8 = local_28[2] + local_28[6];
    sVar4 = sVar3 + sVar8;
    sVar3 = sVar3 - sVar8;
    sVar8 = (short)((uint)((short)(local_28[2] - local_28[6]) * 0x16a) >> 8) - sVar8;
    sVar5 = sVar1 + sVar8;
    sVar1 = sVar1 - sVar8;
    sVar6 = local_28[1] + local_28[7];
    sVar8 = local_28[1] - local_28[7];
    sVar9 = local_28[5] + local_28[3];
    sVar10 = local_28[5] - local_28[3];
    sVar7 = sVar6 + sVar9;
    sVar11 = (short)((uint)((short)(sVar10 + sVar8) * 0x1d9) >> 8);
    sVar10 = ((short)((uint)(sVar10 * -0x29d) >> 8) + sVar11) - sVar7;
    sVar6 = (short)((uint)((short)(sVar6 - sVar9) * 0x16a) >> 8) - sVar10;
    sVar8 = ((short)((uint)(sVar8 * 0x115) >> 8) - sVar11) + sVar6;
    uVar2 = *(undefined1 *)(param_6 + ((ushort)((short)(sVar4 - sVar7) >> 5) & 0x3ff));
    *puVar12 = *(undefined1 *)(param_6 + ((ushort)((short)(sVar7 + sVar4) >> 5) & 0x3ff));
    puVar12[7] = uVar2;
    uVar2 = *(undefined1 *)(param_6 + ((ushort)((short)(sVar5 - sVar10) >> 5) & 0x3ff));
    puVar12[1] = *(undefined1 *)(param_6 + ((ushort)((short)(sVar10 + sVar5) >> 5) & 0x3ff));
    puVar12[6] = uVar2;
    uVar2 = *(undefined1 *)(param_6 + ((ushort)((short)(sVar1 - sVar6) >> 5) & 0x3ff));
    puVar12[2] = *(undefined1 *)(param_6 + ((ushort)((short)(sVar6 + sVar1) >> 5) & 0x3ff));
    puVar12[5] = uVar2;
    uVar2 = *(undefined1 *)(param_6 + ((ushort)((short)(sVar3 - sVar8) >> 5) & 0x3ff));
    puVar12[4] = *(undefined1 *)(param_6 + ((ushort)((short)(sVar8 + sVar3) >> 5) & 0x3ff));
    puVar12[3] = uVar2;
    local_28 = local_28 + 8;
    local_20 = local_20 + -1;
  } while (local_20 != 0);
  return;
}

