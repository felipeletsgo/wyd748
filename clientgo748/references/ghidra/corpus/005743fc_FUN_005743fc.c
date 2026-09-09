// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005743fc | Name: FUN_005743fc


void FUN_005743fc(uint *param_1,int param_2,uint param_3,byte param_4)

{
  int iVar1;
  uint *puVar2;
  uint uVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  undefined1 uVar6;
  byte bVar7;
  uint uVar8;
  undefined1 *puVar9;
  
  puVar2 = param_1;
  uVar3 = *param_1;
  uVar8 = param_3 >> 8;
  bVar7 = (byte)(param_3 >> 8);
  uVar6 = (undefined1)param_3;
  if ((char)param_1[2] == '\0') {
    if (*(char *)((int)param_1 + 9) == '\b') {
      puVar4 = (undefined1 *)(param_2 + uVar3);
      puVar9 = puVar4 + uVar3;
      uVar8 = uVar3;
      if ((param_4 & 0x80) == 0) {
        for (; uVar8 != 0; uVar8 = uVar8 - 1) {
          puVar4 = puVar4 + -1;
          puVar9[-1] = *puVar4;
          puVar9 = puVar9 + -2;
          *puVar9 = uVar6;
        }
      }
      else {
        if (1 < uVar3) {
          param_1 = (uint *)(uVar3 - 1);
          do {
            puVar4 = puVar4 + -1;
            puVar9[-1] = uVar6;
            puVar9 = puVar9 + -2;
            param_1 = (uint *)((int)param_1 + -1);
            *puVar9 = *puVar4;
          } while (param_1 != (uint *)0x0);
        }
        puVar9[-1] = uVar6;
      }
      *(undefined1 *)((int)puVar2 + 10) = 2;
      *(undefined1 *)((int)puVar2 + 0xb) = 0x10;
      uVar3 = uVar3 * 2;
    }
    else {
      if (*(char *)((int)param_1 + 9) != '\x10') {
        return;
      }
      if ((param_4 & 0x80) == 0) {
        puVar9 = (undefined1 *)(uVar3 + param_2);
        puVar4 = puVar9 + uVar3;
        for (uVar8 = uVar3; uVar8 != 0; uVar8 = uVar8 - 1) {
          puVar5 = puVar9 + -1;
          puVar9 = puVar9 + -2;
          puVar4[-1] = *puVar5;
          puVar4[-2] = *puVar9;
          puVar4[-3] = bVar7;
          puVar4 = puVar4 + -4;
          *puVar4 = uVar6;
        }
      }
      else {
        puVar9 = (undefined1 *)(param_2 + uVar3);
        puVar4 = puVar9 + uVar3;
        if (1 < uVar3) {
          param_3 = uVar3 - 1;
          do {
            puVar4[-1] = (char)uVar8;
            puVar4[-2] = uVar6;
            puVar4[-3] = puVar9[-1];
            puVar9 = puVar9 + -2;
            puVar4 = puVar4 + -4;
            param_3 = param_3 + -1;
            *puVar4 = *puVar9;
            uVar8 = (uint)bVar7;
          } while (param_3 != 0);
        }
        puVar4[-1] = bVar7;
        puVar4[-2] = uVar6;
      }
      *(undefined1 *)((int)param_1 + 10) = 2;
      *(undefined1 *)((int)param_1 + 0xb) = 0x20;
      uVar3 = uVar3 << 2;
    }
  }
  else {
    if ((char)param_1[2] != '\x02') {
      return;
    }
    if (*(char *)((int)param_1 + 9) == '\b') {
      if ((param_4 & 0x80) == 0) {
        puVar9 = (undefined1 *)(param_2 + uVar3 * 3);
        puVar4 = puVar9 + uVar3;
        for (uVar8 = uVar3; uVar8 != 0; uVar8 = uVar8 - 1) {
          puVar4[-1] = puVar9[-1];
          puVar4[-2] = puVar9[-2];
          puVar9 = puVar9 + -3;
          puVar4[-3] = *puVar9;
          puVar4 = puVar4 + -4;
          *puVar4 = uVar6;
        }
      }
      else {
        puVar9 = (undefined1 *)(param_2 + uVar3 * 3);
        puVar4 = puVar9 + uVar3;
        if (1 < uVar3) {
          param_1 = (uint *)(uVar3 - 1);
          do {
            puVar4[-1] = uVar6;
            puVar4[-2] = puVar9[-1];
            puVar5 = puVar9 + -2;
            puVar9 = puVar9 + -3;
            puVar4[-3] = *puVar5;
            puVar4 = puVar4 + -4;
            param_1 = (uint *)((int)param_1 + -1);
            *puVar4 = *puVar9;
          } while (param_1 != (uint *)0x0);
        }
        puVar4[-1] = uVar6;
      }
      *(undefined1 *)((int)puVar2 + 0xb) = 0x20;
      iVar1 = 2;
    }
    else {
      if (*(char *)((int)param_1 + 9) != '\x10') {
        return;
      }
      puVar9 = (undefined1 *)(param_2 + uVar3 * 3);
      puVar4 = puVar9 + uVar3;
      uVar8 = uVar3;
      if ((param_4 & 0x80) == 0) {
        for (; uVar8 != 0; uVar8 = uVar8 - 1) {
          puVar4[-1] = puVar9[-1];
          puVar4[-2] = puVar9[-2];
          puVar4[-3] = puVar9[-3];
          puVar4[-4] = puVar9[-4];
          puVar4[-5] = puVar9[-5];
          puVar9 = puVar9 + -6;
          puVar4[-6] = *puVar9;
          puVar4[-7] = bVar7;
          puVar4 = puVar4 + -8;
          *puVar4 = uVar6;
        }
      }
      else {
        if (1 < uVar3) {
          param_3 = uVar3 - 1;
          do {
            puVar4[-1] = bVar7;
            puVar4[-2] = uVar6;
            puVar4[-3] = puVar9[-1];
            puVar4[-4] = puVar9[-2];
            puVar4[-5] = puVar9[-3];
            puVar4[-6] = puVar9[-4];
            puVar5 = puVar9 + -5;
            puVar9 = puVar9 + -6;
            puVar4[-7] = *puVar5;
            puVar4 = puVar4 + -8;
            param_3 = param_3 + -1;
            *puVar4 = *puVar9;
          } while (param_3 != 0);
        }
        puVar4[-1] = bVar7;
        puVar4[-2] = uVar6;
      }
      *(undefined1 *)((int)param_1 + 0xb) = 0x40;
      iVar1 = 3;
    }
    uVar3 = uVar3 << iVar1;
    *(undefined1 *)((int)puVar2 + 10) = 4;
  }
  puVar2[1] = uVar3;
  return;
}

