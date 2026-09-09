// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574fea | Name: FUN_00574fea


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00574fea(uint param_1)

{
  ushort *puVar1;
  byte bVar2;
  int iVar3;
  undefined1 uVar4;
  undefined2 uVar5;
  undefined4 uVar6;
  uint uVar7;
  uint uVar8;
  byte bVar9;
  int iVar10;
  uint uVar11;
  int iVar12;
  uint uVar13;
  ushort local_10;
  uint local_8;
  
  iVar3 = param_1;
  if (*(float *)(param_1 + 0x130) != (float)_DAT_005a68b8) {
    if (*(byte *)(param_1 + 0x117) < 9) {
      uVar6 = FUN_005799ef(param_1,0x100);
      param_1 = 0;
      *(undefined4 *)(iVar3 + 0x138) = uVar6;
      do {
        FUN_00592cf0();
        uVar4 = __ftol();
        iVar10 = param_1 + 1;
        *(undefined1 *)(param_1 + *(int *)(iVar3 + 0x138)) = uVar4;
        param_1 = iVar10;
      } while (iVar10 < 0x100);
    }
    else {
      if ((*(byte *)(param_1 + 0x116) & 2) == 0) {
        local_8 = (uint)*(byte *)(param_1 + 0x153);
      }
      else {
        local_8 = (uint)*(byte *)(param_1 + 0x150);
        if ((uint)*(byte *)(param_1 + 0x150) < (uint)*(byte *)(param_1 + 0x151)) {
          local_8 = (uint)*(byte *)(param_1 + 0x151);
        }
        if (local_8 < *(byte *)(param_1 + 0x152)) {
          local_8 = (uint)*(byte *)(param_1 + 0x152);
        }
      }
      if (local_8 == 0) {
        local_8 = 0;
      }
      else {
        local_8 = 0x10 - local_8;
      }
      if (((*(byte *)(param_1 + 0x61) & 4) != 0) && ((int)local_8 < 5)) {
        local_8 = 5;
      }
      if (8 < (int)local_8) {
        local_8 = 8;
      }
      if ((int)local_8 < 0) {
        local_8 = 0;
      }
      bVar2 = (byte)local_8;
      bVar9 = 8 - bVar2;
      iVar10 = 1 << (bVar9 & 0x1f);
      *(uint *)(param_1 + 300) = local_8 & 0xff;
      uVar6 = FUN_005799ef(param_1,iVar10 << 2);
      puVar1 = (ushort *)(param_1 + 0x60);
      *(undefined4 *)(param_1 + 0x144) = uVar6;
      param_1 = 0;
      if ((*puVar1 & 0x480) == 0) {
        if (0 < iVar10) {
          do {
            uVar6 = FUN_005799ef(iVar3,0x200);
            *(undefined4 *)(*(int *)(iVar3 + 0x144) + param_1 * 4) = uVar6;
            iVar12 = 0;
            do {
              FUN_00592cf0();
              uVar5 = __ftol();
              *(undefined2 *)(iVar12 + *(int *)(*(int *)(iVar3 + 0x144) + param_1 * 4)) = uVar5;
              iVar12 = iVar12 + 2;
            } while (iVar12 < 0x200);
            param_1 = param_1 + 1;
          } while ((int)param_1 < iVar10);
        }
      }
      else {
        if (0 < iVar10) {
          do {
            uVar6 = FUN_005799ef(iVar3,0x200);
            iVar12 = param_1 + 1;
            *(undefined4 *)(*(int *)(iVar3 + 0x144) + param_1 * 4) = uVar6;
            param_1 = iVar12;
          } while (iVar12 < iVar10);
        }
        uVar13 = 0;
        param_1 = 0;
        do {
          FUN_00592cf0();
          uVar7 = __ftol();
          if (uVar13 <= uVar7) {
            local_10 = (ushort)((param_1 & 0xff) << 8) | (ushort)param_1;
            do {
              uVar11 = uVar13 >> (bVar9 & 0x1f);
              uVar8 = 0xff >> (bVar2 & 0x1f) & uVar13;
              uVar13 = uVar13 + 1;
              *(ushort *)(*(int *)(*(int *)(iVar3 + 0x144) + uVar8 * 4) + uVar11 * 2) = local_10;
            } while (uVar13 <= uVar7);
          }
          param_1 = param_1 + 1;
        } while ((int)param_1 < 0x100);
        if (uVar13 < (uint)(iVar10 << 8)) {
          do {
            *(undefined2 *)
             (*(int *)(*(int *)(iVar3 + 0x144) + (0xff >> (bVar2 & 0x1f) & uVar13) * 4) +
             (uVar13 >> (bVar9 & 0x1f)) * 2) = 0xffff;
            uVar13 = uVar13 + 1;
          } while (uVar13 < (uint)(iVar10 << 8));
        }
      }
    }
  }
  return;
}

