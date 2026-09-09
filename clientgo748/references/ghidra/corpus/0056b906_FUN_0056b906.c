// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056b906 | Name: FUN_0056b906


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0056b906(undefined4 *param_1,undefined4 *param_2,uint param_3,undefined4 param_4)

{
  float fVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined *puVar4;
  float *pfVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined4 *puVar9;
  
  *param_1 = &PTR_FUN_005a694c;
  _vector_constructor_iterator_(param_1 + 0xc,0x10,0x100,FUN_00430f10);
  param_1[6] = *param_2;
  param_1[1] = param_2[1];
  param_1[0x414] = param_2[2];
  param_1[0x415] = param_2[3];
  puVar2 = param_2 + 10;
  puVar9 = param_1 + 0x40c;
  for (iVar3 = 6; fVar1 = _DAT_005a68b0, iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar9 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar9 = puVar9 + 1;
  }
  puVar4 = &DAT_005c5848;
  if (param_2[0x10] == 0) {
    puVar4 = &DAT_005c57c8;
  }
  param_1[0xb] = puVar4;
  param_1[0x41a] = param_3 >> 3;
  param_1[0x412] = 0;
  param_1[0x413] = 0;
  param_1[3] = (uint)(param_3 != 0);
  param_1[2] = param_4;
  iVar3 = param_2[0x11];
  param_1[4] = iVar3;
  if (iVar3 != 0) {
    uVar7 = param_2[0x11];
    param_1[7] = (float)(uVar7 >> 0x10 & 0xff) * fVar1;
    param_1[8] = (float)(uVar7 >> 8 & 0xff) * fVar1;
    param_1[9] = (float)(uVar7 & 0xff) * fVar1;
    param_1[10] = (float)(uVar7 >> 0x18) * fVar1;
  }
  if ((param_1[1] == 0x29) || (param_1[1] == 0x28)) {
    param_1[5] = 1;
    if (param_2[0x12] == 0) {
      iVar3 = 0x100;
      puVar2 = param_1 + 0xe;
      do {
        puVar2[1] = 0x3f800000;
        *puVar2 = 0x3f800000;
        iVar3 = iVar3 + -1;
        puVar2[-1] = 0x3f800000;
        puVar2[-2] = 0x3f800000;
        puVar2 = puVar2 + 4;
      } while (iVar3 != 0);
    }
    else {
      pfVar5 = (float *)(param_1 + 0xd);
      uVar7 = 0;
      do {
        uVar8 = uVar7 + 4;
        pfVar5[-1] = (float)*(byte *)(uVar7 + param_2[0x12]) * fVar1;
        *pfVar5 = (float)*(byte *)(uVar7 + 1 + param_2[0x12]) * fVar1;
        pfVar5[1] = (float)*(byte *)(uVar7 + 2 + param_2[0x12]) * fVar1;
        pfVar5[2] = (float)*(byte *)(uVar7 + 3 + param_2[0x12]) * fVar1;
        pfVar5 = pfVar5 + 4;
        uVar7 = uVar8;
      } while (uVar8 < 0x400);
    }
  }
  else {
    param_1[5] = 0;
  }
  param_1[0x417] = param_1[0x40f] - param_1[0x40d];
  iVar3 = param_1[0x40c];
  iVar6 = param_1[0x40e] - iVar3;
  param_1[0x418] = param_1[0x411] - param_1[0x410];
  param_1[0x416] = iVar6;
  param_1[0x419] = param_1[0x41a] * iVar6;
  if (param_1[3] != 0) {
    param_1[6] = param_1[6] +
                 param_1[0x41a] * iVar3 + param_1[0x414] * param_1[0x40d] +
                 param_1[0x415] * param_1[0x410];
    param_1[0x40e] = param_1[0x416];
    param_1[0x40f] = param_1[0x417];
    param_1[0x40c] = 0;
    param_1[0x40d] = 0;
    param_1[0x410] = 0;
    param_1[0x411] = param_1[0x418];
  }
  return param_1;
}

