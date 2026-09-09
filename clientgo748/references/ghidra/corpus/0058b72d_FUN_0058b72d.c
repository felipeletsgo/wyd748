// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058b72d | Name: FUN_0058b72d


void FUN_0058b72d(int param_1,int param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  int *piVar2;
  undefined1 uVar3;
  short *psVar4;
  int iVar5;
  short *psVar6;
  undefined1 *puVar7;
  int local_4c;
  int local_48 [2];
  int local_40 [2];
  int local_38 [2];
  int local_30 [9];
  int local_c;
  int *local_8;
  
  iVar5 = *(int *)(param_1 + 0x11c) + 0x80;
  piVar2 = &local_4c;
  psVar6 = (short *)(param_3 + 0x30);
  local_c = iVar5;
  param_1 = 8;
  psVar4 = (short *)(*(int *)(param_2 + 0x50) + 0x30);
  do {
    if (((param_1 != 6) && (param_1 != 4)) && (param_1 != 2)) {
      if (((psVar6[0x20] == 0 && psVar6[0x10] == 0) && psVar6[-0x10] == 0) && *psVar6 == 0) {
        iVar1 = (int)psVar4[-0x18] * (int)psVar6[-0x18] * 4;
        *piVar2 = iVar1;
      }
      else {
        iVar5 = (int)psVar4[-0x18] * (int)psVar6[-0x18] * 0x8000;
        iVar1 = (short)(psVar4[0x10] * psVar6[0x10]) * 0x1b37 +
                (short)(psVar4[0x20] * psVar6[0x20]) * -0x1712 +
                (short)(psVar4[-0x10] * psVar6[-0x10]) * 0x73fc +
                (short)(*psVar4 * *psVar6) * -0x28ba;
        *piVar2 = iVar1 + 0x1000 + iVar5 >> 0xd;
        iVar1 = (iVar5 - iVar1) + 0x1000 >> 0xd;
        iVar5 = local_c;
      }
      piVar2[8] = iVar1;
    }
    psVar6 = psVar6 + 1;
    piVar2 = piVar2 + 1;
    psVar4 = psVar4 + 1;
    param_1 = param_1 + -1;
    local_8 = piVar2;
  } while (0 < param_1);
  param_1 = 0;
  piVar2 = &local_4c;
  do {
    puVar7 = (undefined1 *)(*(int *)(param_4 + param_1 * 4) + param_5);
    if (((piVar2[7] == 0 && piVar2[3] == 0) && piVar2[1] == 0) && piVar2[5] == 0) {
      uVar3 = *(undefined1 *)((*piVar2 + 0x10 >> 5 & 0x3ffU) + iVar5);
      *puVar7 = uVar3;
    }
    else {
      iVar5 = *piVar2 * 0x8000;
      iVar1 = (short)piVar2[3] * -0x28ba + (short)piVar2[7] * -0x1712 + (short)piVar2[1] * 0x73fc +
              (short)piVar2[5] * 0x1b37;
      *puVar7 = *(undefined1 *)((iVar1 + 0x80000 + iVar5 >> 0x14 & 0x3ffU) + local_c);
      uVar3 = *(undefined1 *)(((iVar5 - iVar1) + 0x80000 >> 0x14 & 0x3ffU) + local_c);
      iVar5 = local_c;
    }
    piVar2 = piVar2 + 8;
    param_1 = param_1 + 1;
    puVar7[1] = uVar3;
  } while (param_1 < 2);
  return;
}

