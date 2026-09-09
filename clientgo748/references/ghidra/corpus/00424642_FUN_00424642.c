// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00424642 | Name: FUN_00424642


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_00424642(int param_1,undefined4 *param_2,uint *param_3)

{
  byte bVar1;
  char cVar2;
  ushort uVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  char cVar7;
  uint local_54;
  int local_4c;
  char local_48;
  byte local_40;
  uint local_38;
  uint local_30;
  uint local_28;
  
  *param_2 = 0;
  if (*(int *)(param_1 + 0x10) <= *(int *)(param_1 + 0x14)) {
    *(undefined4 *)(param_1 + 0x10) = 0;
    *(undefined4 *)(param_1 + 0x14) = 0;
    return 0;
  }
  if (*(int *)(param_1 + 0x1c) == 0) {
    if (*(int *)(param_1 + 0x10) - *(int *)(param_1 + 0x14) < 4) {
      return 0;
    }
    uVar6 = *(uint *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14));
    if (uVar6 != 0x1f11f311) {
      *param_2 = 2;
      *param_3 = uVar6;
      return 0;
    }
    *(undefined4 *)(param_1 + 0x1c) = 1;
    *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + 4;
  }
  if ((uint)(*(int *)(param_1 + 0x10) - *(int *)(param_1 + 0x14)) < 0xc) {
    iVar5 = 0;
  }
  else {
    uVar3 = *(ushort *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14));
    bVar1 = *(byte *)(*(int *)(param_1 + 8) + 2 + *(int *)(param_1 + 0x14));
    local_38 = (uint)(byte)(&DAT_005b1d10)[(uint)bVar1 * 2];
    cVar2 = *(char *)(*(int *)(param_1 + 8) + 3 + *(int *)(param_1 + 0x14));
    if (*(char *)(param_1 + 0x30) != '\0') {
      if (*(int *)(param_1 + 0x44) < 0x10) {
        local_40 = *(byte *)(param_1 + 0x30 + *(int *)(param_1 + 0x44));
        *(int *)(param_1 + 0x44) = *(int *)(param_1 + 0x44) + 1;
      }
      else if (_DAT_005ccf88 == 0) {
        uVar6 = (int)*(char *)(param_1 + 0x3f) & 0x80000001;
        if ((int)uVar6 < 0) {
          uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
        }
        if (uVar6 == 0) {
          local_40 = *(char *)(param_1 + 0x31) + *(char *)(param_1 + 0x33) + -0x57 +
                     *(char *)(param_1 + 0x35);
        }
        else {
          local_40 = ((*(char *)(param_1 + 0x3d) + *(char *)(param_1 + 0x3b)) -
                     *(char *)(param_1 + 0x39)) + 4;
        }
      }
      else {
        local_48 = DAT_005ccf88;
        if (DAT_005ccf88 == '\0') {
          local_48 = DAT_005ccf8b;
        }
        if (local_48 == '\0') {
          local_48 = '\r';
        }
        local_4c = ((int)DAT_005ccf8a + (int)DAT_005ccf8b) - (int)DAT_005ccf89 * (int)local_48;
        if (local_4c == 0) {
          local_4c = (int)DAT_005ccf88;
        }
        local_40 = (byte)local_4c;
      }
      if ((local_40 ^ 0xff) != bVar1) {
        *param_2 = 3;
        *param_3 = (uint)uVar3;
        return 0;
      }
    }
    if ((uVar3 < 0x1b59) && (0xb < uVar3)) {
      if ((ushort)((short)*(undefined4 *)(param_1 + 0x10) - (short)*(undefined4 *)(param_1 + 0x14))
          < uVar3) {
        iVar5 = 0;
      }
      else {
        iVar5 = *(int *)(param_1 + 8) + *(int *)(param_1 + 0x14);
        *(uint *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + (uint)uVar3;
        if (*(int *)(param_1 + 0x10) <= *(int *)(param_1 + 0x14)) {
          *(undefined4 *)(param_1 + 0x10) = 0;
          *(undefined4 *)(param_1 + 0x14) = 0;
        }
        cVar4 = '\0';
        cVar7 = '\0';
        local_30 = local_38;
        for (local_28 = 4; (int)local_28 < (int)(uint)uVar3; local_28 = local_28 + 1) {
          uVar6 = local_30 & 0x800000ff;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xffffff00) + 1;
          }
          cVar7 = cVar7 + *(char *)(iVar5 + local_28);
          bVar1 = (&DAT_005b1d11)[uVar6 * 2];
          local_54 = (uint)bVar1;
          uVar6 = local_28 & 3;
          if (uVar6 == 0) {
            *(byte *)(iVar5 + local_28) = *(char *)(iVar5 + local_28) + bVar1 * -2;
          }
          if (uVar6 == 1) {
            *(char *)(iVar5 + local_28) = *(char *)(iVar5 + local_28) + (char)((int)local_54 >> 3);
          }
          if (uVar6 == 2) {
            *(byte *)(iVar5 + local_28) = *(char *)(iVar5 + local_28) + bVar1 * -4;
          }
          if (uVar6 == 3) {
            *(char *)(iVar5 + local_28) = *(char *)(iVar5 + local_28) + (char)((int)local_54 >> 5);
          }
          cVar4 = cVar4 + *(char *)(iVar5 + local_28);
          local_30 = local_30 + 1;
        }
        if ((char)(cVar7 - cVar4) != cVar2) {
          *param_2 = 1;
          *param_3 = (uint)uVar3;
        }
      }
    }
    else {
      *(undefined4 *)(param_1 + 0x10) = 0;
      *(undefined4 *)(param_1 + 0x14) = 0;
      *param_2 = 2;
      *param_3 = (uint)uVar3;
      iVar5 = 0;
    }
  }
  return iVar5;
}

