// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574982 | Name: FUN_00574982


void FUN_00574982(uint *param_1,int param_2,int param_3,int param_4,int param_5)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  byte *pbVar5;
  byte *pbVar6;
  uint uVar7;
  
  uVar2 = *param_1;
  if ((char)param_1[2] == '\x03') {
    bVar1 = *(byte *)((int)param_1 + 9);
    if (bVar1 < 8) {
      if (bVar1 == 1) {
        pbVar6 = (byte *)((uVar2 - 1) + param_2);
        iVar3 = 7 - (uVar2 - 1 & 7);
        pbVar5 = (byte *)((uVar2 - 1 >> 3) + param_2);
        for (uVar7 = uVar2; uVar7 != 0; uVar7 = uVar7 - 1) {
          *pbVar6 = *pbVar5 >> ((byte)iVar3 & 0x1f) & 1;
          if (iVar3 == 7) {
            iVar3 = 0;
            pbVar5 = pbVar5 + -1;
          }
          else {
            iVar3 = iVar3 + 1;
          }
          pbVar6 = pbVar6 + -1;
        }
      }
      else if (bVar1 == 2) {
        pbVar6 = (byte *)((uVar2 - 1) + param_2);
        iVar3 = (3 - (uVar2 - 1 & 3)) * 2;
        pbVar5 = (byte *)((uVar2 - 1 >> 2) + param_2);
        for (uVar7 = uVar2; uVar7 != 0; uVar7 = uVar7 - 1) {
          *pbVar6 = *pbVar5 >> ((byte)iVar3 & 0x1f) & 3;
          if (iVar3 == 6) {
            iVar3 = 0;
            pbVar5 = pbVar5 + -1;
          }
          else {
            iVar3 = iVar3 + 2;
          }
          pbVar6 = pbVar6 + -1;
        }
      }
      else if (bVar1 == 4) {
        pbVar6 = (byte *)((uVar2 - 1) + param_2);
        iVar3 = (uVar2 & 1) << 2;
        pbVar5 = (byte *)((uVar2 - 1 >> 1) + param_2);
        for (uVar7 = uVar2; uVar7 != 0; uVar7 = uVar7 - 1) {
          *pbVar6 = *pbVar5 >> ((byte)iVar3 & 0x1f) & 0xf;
          if (iVar3 == 4) {
            iVar3 = 0;
            pbVar5 = pbVar5 + -1;
          }
          else {
            iVar3 = iVar3 + 4;
          }
          pbVar6 = pbVar6 + -1;
        }
      }
      *(undefined1 *)((int)param_1 + 9) = 8;
      *(undefined1 *)((int)param_1 + 0xb) = 8;
      param_1[1] = uVar2;
    }
    if (*(char *)((int)param_1 + 9) == '\b') {
      pbVar6 = (byte *)((uVar2 - 1) + param_2);
      if (param_4 == 0) {
        uVar7 = uVar2 * 3;
        puVar4 = (undefined1 *)((uVar7 - 1) + param_2);
        for (; uVar2 != 0; uVar2 = uVar2 - 1) {
          *puVar4 = *(undefined1 *)((uint)*pbVar6 * 3 + 2 + param_3);
          puVar4[-1] = *(undefined1 *)((uint)*pbVar6 * 3 + 1 + param_3);
          puVar4[-2] = *(undefined1 *)(param_3 + (uint)*pbVar6 * 3);
          puVar4 = puVar4 + -3;
          pbVar6 = pbVar6 + -1;
        }
        *(undefined1 *)((int)param_1 + 0xb) = 0x18;
        *(undefined1 *)(param_1 + 2) = 2;
        *(undefined1 *)((int)param_1 + 10) = 3;
      }
      else {
        uVar7 = uVar2 * 4;
        puVar4 = (undefined1 *)((uVar7 - 1) + param_2);
        for (; uVar2 != 0; uVar2 = uVar2 - 1) {
          if ((int)(uint)*pbVar6 < param_5) {
            *puVar4 = *(undefined1 *)((uint)*pbVar6 + param_4);
          }
          else {
            *puVar4 = 0xff;
          }
          puVar4[-1] = *(undefined1 *)((uint)*pbVar6 * 3 + 2 + param_3);
          puVar4[-2] = *(undefined1 *)((uint)*pbVar6 * 3 + 1 + param_3);
          puVar4[-3] = *(undefined1 *)(param_3 + (uint)*pbVar6 * 3);
          puVar4 = puVar4 + -4;
          pbVar6 = pbVar6 + -1;
        }
        *(undefined1 *)((int)param_1 + 0xb) = 0x20;
        *(undefined1 *)(param_1 + 2) = 6;
        *(undefined1 *)((int)param_1 + 10) = 4;
      }
      *(undefined1 *)((int)param_1 + 9) = 8;
      param_1[1] = uVar7;
    }
  }
  return;
}

