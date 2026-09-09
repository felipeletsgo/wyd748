// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574241 | Name: FUN_00574241


byte * FUN_00574241(byte *param_1,byte *param_2,byte *param_3)

{
  bool bVar1;
  byte bVar2;
  ushort uVar3;
  byte *pbVar4;
  uint uVar5;
  byte bVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  int local_18 [4];
  byte *local_8;
  
  bVar2 = *(byte *)((int)param_1 + 8);
  pbVar4 = (byte *)(uint)bVar2;
  if (bVar2 != 3) {
    bVar1 = false;
    iVar10 = *(int *)param_1;
    if ((bVar2 & 2) == 0) {
      iVar8 = (uint)*(byte *)((int)param_1 + 9) - (uint)param_3[3];
      uVar9 = 1;
    }
    else {
      iVar8 = (uint)*(byte *)((int)param_1 + 9) - (uint)*param_3;
      local_18[1] = (uint)*(byte *)((int)param_1 + 9) - (uint)param_3[1];
      local_18[2] = (uint)*(byte *)((int)param_1 + 9) - (uint)param_3[2];
      uVar9 = 3;
    }
    local_18[0] = iVar8;
    pbVar4 = param_3;
    if ((*(byte *)((int)param_1 + 8) & 4) != 0) {
      pbVar4 = (byte *)((uint)*(byte *)((int)param_1 + 9) - (uint)param_3[4]);
      local_18[uVar9] = (int)pbVar4;
      uVar9 = uVar9 + 1;
    }
    iVar7 = 0;
    if (uVar9 != 0) {
      do {
        pbVar4 = (byte *)(local_18 + iVar7);
        if (*(int *)pbVar4 < 1) {
          pbVar4[0] = 0;
          pbVar4[1] = 0;
          pbVar4[2] = 0;
          pbVar4[3] = 0;
          iVar8 = local_18[0];
        }
        else {
          bVar1 = true;
        }
        iVar7 = iVar7 + 1;
      } while (iVar7 < (int)uVar9);
      if (bVar1) {
        uVar5 = (uint)*(byte *)((int)param_1 + 9);
        if (uVar5 == 2) {
          pbVar4 = param_2;
          for (iVar10 = *(int *)((int)param_1 + 4); iVar10 != 0; iVar10 = iVar10 + -1) {
            *pbVar4 = *pbVar4 >> 1 & 0x55;
            pbVar4 = pbVar4 + 1;
          }
        }
        else if (uVar5 == 4) {
          bVar6 = (byte)iVar8;
          bVar2 = (byte)(0xf0 >> (bVar6 & 0x1f)) & 0xf0 | (byte)(0xf >> (bVar6 & 0x1f));
          pbVar4 = (byte *)(uint)bVar2;
          for (iVar10 = *(int *)((int)param_1 + 4); iVar10 != 0; iVar10 = iVar10 + -1) {
            *param_2 = *param_2 >> (bVar6 & 0x1f) & bVar2;
            param_2 = param_2 + 1;
          }
        }
        else if (uVar5 == 8) {
          uVar5 = iVar10 * uVar9;
          uVar11 = 0;
          pbVar4 = (byte *)0x0;
          if (uVar5 != 0) {
            do {
              pbVar4 = (byte *)(uVar11 / uVar9);
              *param_2 = *param_2 >> (*(byte *)(local_18 + uVar11 % uVar9) & 0x1f);
              param_2 = param_2 + 1;
              uVar11 = uVar11 + 1;
            } while (uVar11 < uVar5);
          }
        }
        else {
          pbVar4 = (byte *)(uVar5 - 0x10);
          if (pbVar4 == (byte *)0x0) {
            local_8 = (byte *)(iVar10 * uVar9);
            param_1 = (byte *)0x0;
            if (local_8 != (byte *)0x0) {
              do {
                uVar3 = (ushort)((ushort)*param_2 * 0x100 + (ushort)param_2[1]) >>
                        ((byte)(short)local_18[(uint)param_1 % uVar9] & 0x1f);
                *param_2 = (byte)(uVar3 >> 8);
                param_2[1] = (byte)uVar3;
                param_2 = param_2 + 2;
                pbVar4 = param_1 + 1;
                param_1 = pbVar4;
              } while (pbVar4 < local_8);
            }
          }
        }
      }
    }
  }
  return pbVar4;
}

