// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056b152 | Name: FUN_0056b152


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0056b152(int param_1,int param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  undefined4 *puVar4;
  float *pfVar5;
  int iVar6;
  uint uVar7;
  
  fVar3 = _DAT_005a680c;
  fVar2 = _DAT_005a67d8;
  fVar1 = _DAT_005a67d4;
  iVar6 = *(int *)(param_1 + 8);
  if (iVar6 == 1) {
    if (*(int *)(param_1 + 0x1048) == 2) {
      uVar7 = 0;
      if (*(int *)(param_1 + 0x1058) != 0) {
        pfVar5 = (float *)(param_2 + 8);
        iVar6 = 0;
        do {
          uVar7 = uVar7 + 1;
          *(float *)(iVar6 + *(int *)(param_1 + 0x104c)) = (pfVar5[-2] + fVar2) * fVar1;
          *(float *)(iVar6 + 4 + *(int *)(param_1 + 0x104c)) = (pfVar5[-1] + fVar2) * fVar1;
          *(float *)(iVar6 + 8 + *(int *)(param_1 + 0x104c)) = (*pfVar5 + fVar2) * fVar1;
          *(float *)(iVar6 + 0xc + *(int *)(param_1 + 0x104c)) = pfVar5[1];
          pfVar5 = pfVar5 + 4;
          iVar6 = iVar6 + 0x10;
        } while (uVar7 < *(uint *)(param_1 + 0x1058));
      }
    }
    else if ((*(int *)(param_1 + 0x1048) == 3) && (uVar7 = 0, *(int *)(param_1 + 0x1058) != 0)) {
      pfVar5 = (float *)(param_2 + 8);
      iVar6 = 0;
      do {
        uVar7 = uVar7 + 1;
        *(float *)(iVar6 + *(int *)(param_1 + 0x104c)) = (pfVar5[-2] + fVar2) * fVar1;
        *(float *)(iVar6 + 4 + *(int *)(param_1 + 0x104c)) = (pfVar5[-1] + fVar2) * fVar1;
        *(float *)(iVar6 + 8 + *(int *)(param_1 + 0x104c)) = (*pfVar5 + fVar2) * fVar1;
        *(float *)(iVar6 + 0xc + *(int *)(param_1 + 0x104c)) = (pfVar5[1] + fVar2) * fVar1;
        pfVar5 = pfVar5 + 4;
        iVar6 = iVar6 + 0x10;
      } while (uVar7 < *(uint *)(param_1 + 0x1058));
    }
  }
  else if (iVar6 == 2) {
    if (*(int *)(param_1 + 0x1048) == 1) {
      uVar7 = 0;
      if (*(int *)(param_1 + 0x1058) != 0) {
        pfVar5 = (float *)(param_2 + 8);
        iVar6 = 0;
        do {
          uVar7 = uVar7 + 1;
          *(float *)(iVar6 + *(int *)(param_1 + 0x104c)) = pfVar5[-2] * fVar3 - fVar2;
          *(float *)(iVar6 + 4 + *(int *)(param_1 + 0x104c)) = pfVar5[-1] * fVar3 - fVar2;
          *(float *)(iVar6 + 8 + *(int *)(param_1 + 0x104c)) = *pfVar5 * fVar3 - fVar2;
          *(float *)(iVar6 + 0xc + *(int *)(param_1 + 0x104c)) = pfVar5[1];
          pfVar5 = pfVar5 + 4;
          iVar6 = iVar6 + 0x10;
        } while (uVar7 < *(uint *)(param_1 + 0x1058));
      }
    }
    else if ((*(int *)(param_1 + 0x1048) == 3) && (uVar7 = 0, *(int *)(param_1 + 0x1058) != 0)) {
      puVar4 = (undefined4 *)(param_2 + 8);
      iVar6 = 0;
      do {
        *(undefined4 *)(iVar6 + *(int *)(param_1 + 0x104c)) = puVar4[-2];
        uVar7 = uVar7 + 1;
        *(undefined4 *)(iVar6 + 4 + *(int *)(param_1 + 0x104c)) = puVar4[-1];
        *(undefined4 *)(iVar6 + 8 + *(int *)(param_1 + 0x104c)) = *puVar4;
        pfVar5 = (float *)(puVar4 + 1);
        puVar4 = puVar4 + 4;
        *(float *)(iVar6 + 0xc + *(int *)(param_1 + 0x104c)) =
             (*pfVar5 + _DAT_005a67d8) * _DAT_005a67d4;
        iVar6 = iVar6 + 0x10;
      } while (uVar7 < *(uint *)(param_1 + 0x1058));
    }
  }
  else if (iVar6 == 3) {
    if (*(int *)(param_1 + 0x1048) == 1) {
      uVar7 = 0;
      if (*(int *)(param_1 + 0x1058) != 0) {
        pfVar5 = (float *)(param_2 + 8);
        iVar6 = 0;
        do {
          uVar7 = uVar7 + 1;
          *(float *)(iVar6 + *(int *)(param_1 + 0x104c)) = pfVar5[-2] * fVar3 - fVar2;
          *(float *)(iVar6 + 4 + *(int *)(param_1 + 0x104c)) = pfVar5[-1] * fVar3 - fVar2;
          *(float *)(iVar6 + 8 + *(int *)(param_1 + 0x104c)) = *pfVar5 * fVar3 - fVar2;
          *(float *)(iVar6 + 0xc + *(int *)(param_1 + 0x104c)) = pfVar5[1] * fVar3 - fVar2;
          pfVar5 = pfVar5 + 4;
          iVar6 = iVar6 + 0x10;
        } while (uVar7 < *(uint *)(param_1 + 0x1058));
      }
    }
    else if ((*(int *)(param_1 + 0x1048) == 2) && (uVar7 = 0, *(int *)(param_1 + 0x1058) != 0)) {
      puVar4 = (undefined4 *)(param_2 + 8);
      iVar6 = 0;
      do {
        *(undefined4 *)(iVar6 + *(int *)(param_1 + 0x104c)) = puVar4[-2];
        uVar7 = uVar7 + 1;
        *(undefined4 *)(iVar6 + 4 + *(int *)(param_1 + 0x104c)) = puVar4[-1];
        *(undefined4 *)(iVar6 + 8 + *(int *)(param_1 + 0x104c)) = *puVar4;
        pfVar5 = (float *)(puVar4 + 1);
        puVar4 = puVar4 + 4;
        *(float *)(iVar6 + 0xc + *(int *)(param_1 + 0x104c)) = (*pfVar5 + *pfVar5) - _DAT_005a67d8;
        iVar6 = iVar6 + 0x10;
      } while (uVar7 < *(uint *)(param_1 + 0x1058));
    }
  }
  return *(undefined4 *)(param_1 + 0x104c);
}

