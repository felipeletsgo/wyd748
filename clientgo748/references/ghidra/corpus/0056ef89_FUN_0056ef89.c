// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056ef89 | Name: FUN_0056ef89


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_0056ef89(int param_1,uint param_2,uint param_3,uint param_4)

{
  undefined2 uVar1;
  ushort uVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  int iVar13;
  float *pfVar14;
  ushort *puVar15;
  
  if (*(int *)(param_1 + 0x1090) == 0) {
    iVar13 = -0x7ff8fff2;
  }
  else {
    if ((((param_2 < *(uint *)(param_1 + 0x1074)) || (*(uint *)(param_1 + 0x107c) <= param_2)) ||
        (param_3 < *(uint *)(param_1 + 0x1080))) || (*(uint *)(param_1 + 0x1084) <= param_3)) {
      iVar13 = FUN_0056ed49();
      if (iVar13 < 0) {
        return iVar13;
      }
      *(uint *)(param_1 + 0x107c) = param_2 + 1;
      *(uint *)(param_1 + 0x1074) = param_2;
      *(uint *)(param_1 + 0x1080) = param_3;
      *(uint *)(param_1 + 0x1084) = param_3 + 1;
      fVar12 = _DAT_005a6990;
      fVar11 = _DAT_005a67ec;
      fVar10 = _DAT_005a67d8;
      if (param_4 != 0) {
        param_4 = *(uint *)(param_1 + 0x1070);
        pfVar14 = *(float **)(param_1 + 0x106c);
        puVar15 = (ushort *)
                  (*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   param_4 * 2 + *(int *)(param_1 + 0x18));
        if (param_4 < *(uint *)(param_1 + 0x1078)) {
          do {
            uVar1 = *(undefined2 *)(param_1 + 0x1094);
            uVar2 = puVar15[1];
            fVar8 = (float)(*puVar15 >> ((byte)*(undefined2 *)(param_1 + 0x1098) & 0x1f) & 0xff) -
                    _DAT_005a698c;
            fVar5 = (float)(uVar2 >> ((byte)*(undefined2 *)(param_1 + 0x1098) & 0x1f) & 0xff) -
                    _DAT_005a698c;
            fVar6 = fVar5 * _DAT_005a6a50;
            fVar4 = ((float)(*puVar15 >> ((byte)*(undefined2 *)(param_1 + 0x1094) & 0x1f) & 0xff) -
                    fVar12) * _DAT_005a6a4c;
            fVar7 = fVar4 + fVar6;
            *pfVar14 = fVar7;
            fVar9 = fVar8 * _DAT_005a6a48;
            fVar5 = fVar5 * _DAT_005a6a44;
            fVar3 = (fVar4 - fVar9) - fVar5;
            pfVar14[1] = fVar3;
            fVar8 = fVar8 * _DAT_005a6a40;
            fVar4 = fVar8 + fVar4;
            pfVar14[2] = fVar4;
            pfVar14[3] = 1.0;
            if (fVar11 <= fVar7) {
              if (fVar10 < fVar7) {
                fVar7 = 1.0;
              }
            }
            else {
              fVar7 = 0.0;
            }
            *pfVar14 = fVar7;
            if (fVar11 <= fVar3) {
              if (fVar10 < fVar3) {
                fVar3 = 1.0;
              }
            }
            else {
              fVar3 = 0.0;
            }
            pfVar14[1] = fVar3;
            if (fVar11 <= fVar4) {
              if (fVar10 < fVar4) {
                fVar4 = 1.0;
              }
            }
            else {
              fVar4 = 0.0;
            }
            pfVar14[2] = fVar4;
            fVar3 = ((float)(uVar2 >> ((byte)uVar1 & 0x1f) & 0xff) - fVar12) * _DAT_005a6a4c;
            fVar6 = fVar3 + fVar6;
            pfVar14[4] = fVar6;
            fVar5 = (fVar3 - fVar9) - fVar5;
            pfVar14[5] = fVar5;
            fVar3 = fVar3 + fVar8;
            pfVar14[6] = fVar3;
            pfVar14[7] = 1.0;
            if (fVar11 <= fVar6) {
              if (fVar10 < fVar6) {
                fVar6 = 1.0;
              }
            }
            else {
              fVar6 = 0.0;
            }
            pfVar14[4] = fVar6;
            if (fVar11 <= fVar5) {
              if (fVar10 < fVar5) {
                fVar5 = 1.0;
              }
            }
            else {
              fVar5 = 0.0;
            }
            pfVar14[5] = fVar5;
            if (fVar11 <= fVar3) {
              if (fVar10 < fVar3) {
                fVar3 = 1.0;
              }
            }
            else {
              fVar3 = 0.0;
            }
            param_4 = param_4 + 2;
            pfVar14[6] = fVar3;
            puVar15 = puVar15 + 2;
            pfVar14 = pfVar14 + 8;
          } while (param_4 < *(uint *)(param_1 + 0x1078));
        }
      }
    }
    iVar13 = 0;
  }
  return iVar13;
}

