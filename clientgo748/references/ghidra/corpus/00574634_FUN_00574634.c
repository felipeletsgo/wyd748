// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574634 | Name: FUN_00574634


void FUN_00574634(int *param_1,byte *param_2,int param_3,int param_4,byte param_5)

{
  byte bVar1;
  char cVar2;
  undefined2 uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  int local_c;
  byte *local_8;
  
  bVar1 = *(byte *)((int)param_1 + 9);
  iVar8 = *param_1;
  if (((bVar1 < 9) && (param_3 != 0)) || ((bVar1 == 0x10 && (param_4 != 0)))) {
    cVar2 = (char)param_1[2];
    if (cVar2 == '\0') {
      if ((bVar1 == 2) && (local_8 = param_2, iVar8 != 0)) {
        local_c = (iVar8 - 1U >> 2) + 1;
        do {
          uVar4 = (uint)*local_8;
          uVar6 = uVar4 & 0xc0;
          uVar5 = uVar4 & 3;
          uVar7 = uVar4 & 0xc;
          uVar4 = uVar4 & 0x30;
          local_c = local_c + -1;
          *local_8 = (byte)((byte)(*(byte *)(((uVar7 << 2 | uVar7) << 2 | (int)uVar7 >> 2 | uVar7) +
                                            param_3) & 0xcf |
                                  *(byte *)((((uVar5 << 2 | uVar5) << 2 | uVar5) << 2 | uVar5) +
                                           param_3) >> 2) >> 2 |
                           *(byte *)(((int)((int)uVar4 >> 2 | uVar4) >> 2 | uVar4 << 2 | uVar4) +
                                    param_3) & 0xc3) >> 2 |
                     *(byte *)(((int)((int)((int)uVar6 >> 2 | uVar6) >> 2 | uVar6) >> 2 | uVar6) +
                              param_3) & 0xc0;
          local_8 = local_8 + 1;
        } while (local_c != 0);
      }
      cVar2 = *(char *)((int)param_1 + 9);
      if (cVar2 == '\x04') {
        if (iVar8 != 0) {
          iVar8 = (iVar8 - 1U >> 1) + 1;
          do {
            uVar5 = *param_2 & 0xf0;
            uVar4 = *param_2 & 0xf;
            *param_2 = *(byte *)(((int)uVar5 >> 4 | uVar5) + param_3) & 0xf0 |
                       *(byte *)((uVar4 << 4 | uVar4) + param_3) >> 4;
            param_2 = param_2 + 1;
            iVar8 = iVar8 + -1;
          } while (iVar8 != 0);
        }
      }
      else if (cVar2 == '\b') {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          *param_2 = *(byte *)((uint)*param_2 + param_3);
          param_2 = param_2 + 1;
        }
      }
      else if (cVar2 == '\x10') {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[1] >> (param_5 & 0x1f)) * 4) +
                   (uint)*param_2 * 2);
          *param_2 = (byte)((ushort)uVar3 >> 8);
          param_2[1] = (byte)uVar3;
          param_2 = param_2 + 2;
        }
      }
    }
    else if (cVar2 == '\x02') {
      if (bVar1 == 8) {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          *param_2 = *(byte *)((uint)*param_2 + param_3);
          param_2[1] = *(byte *)((uint)param_2[1] + param_3);
          param_2[2] = *(byte *)((uint)param_2[2] + param_3);
          param_2 = param_2 + 3;
        }
      }
      else {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[1] >> (param_5 & 0x1f)) * 4) +
                   (uint)*param_2 * 2);
          param_2[1] = (byte)uVar3;
          *param_2 = (byte)((ushort)uVar3 >> 8);
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[3] >> (param_5 & 0x1f)) * 4) +
                   (uint)param_2[2] * 2);
          param_2[3] = (byte)uVar3;
          param_2[2] = (byte)((ushort)uVar3 >> 8);
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[5] >> (param_5 & 0x1f)) * 4) +
                   (uint)param_2[4] * 2);
          param_2[4] = (byte)((ushort)uVar3 >> 8);
          param_2[5] = (byte)uVar3;
          param_2 = param_2 + 6;
        }
      }
    }
    else if (cVar2 == '\x04') {
      if (bVar1 == 8) {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          *param_2 = *(byte *)((uint)*param_2 + param_3);
          param_2 = param_2 + 2;
        }
      }
      else {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[1] >> (param_5 & 0x1f)) * 4) +
                   (uint)*param_2 * 2);
          *param_2 = (byte)((ushort)uVar3 >> 8);
          param_2[1] = (byte)uVar3;
          param_2 = param_2 + 4;
        }
      }
    }
    else if (cVar2 == '\x06') {
      if (bVar1 == 8) {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          *param_2 = *(byte *)((uint)*param_2 + param_3);
          param_2[1] = *(byte *)((uint)param_2[1] + param_3);
          param_2[2] = *(byte *)((uint)param_2[2] + param_3);
          param_2 = param_2 + 4;
        }
      }
      else {
        for (; iVar8 != 0; iVar8 = iVar8 + -1) {
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[1] >> (param_5 & 0x1f)) * 4) +
                   (uint)*param_2 * 2);
          param_2[1] = (byte)uVar3;
          *param_2 = (byte)((ushort)uVar3 >> 8);
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[3] >> (param_5 & 0x1f)) * 4) +
                   (uint)param_2[2] * 2);
          param_2[3] = (byte)uVar3;
          param_2[2] = (byte)((ushort)uVar3 >> 8);
          uVar3 = *(undefined2 *)
                   (*(int *)(param_4 + (uint)(param_2[5] >> (param_5 & 0x1f)) * 4) +
                   (uint)param_2[4] * 2);
          param_2[4] = (byte)((ushort)uVar3 >> 8);
          param_2[5] = (byte)uVar3;
          param_2 = param_2 + 8;
        }
      }
    }
  }
  return;
}

