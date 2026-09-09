// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058a370 | Name: FUN_0058a370


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0058a370(uint *param_1,uint *param_2,uint *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  uint uVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  uint uVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  uint uVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  uint uVar21;
  uint uVar22;
  uint uVar23;
  uint uVar24;
  uint uVar25;
  uint uVar26;
  uint uVar27;
  uint uVar28;
  
  fVar6 = (float)(*param_1 & _DAT_005c8420);
  fVar10 = (float)(param_1[1] & uRam005c8424);
  fVar14 = (float)(param_1[2] & uRam005c8428);
  fVar18 = (float)(param_1[3] & uRam005c842c);
  uVar22 = param_1[1] & uRam005c8414;
  uVar23 = param_1[2] & uRam005c8418;
  uVar24 = param_1[3] & uRam005c841c;
  fVar7 = fVar6 * _DAT_005c8400 + _DAT_005c83b0;
  fVar11 = fVar10 * fRam005c8404 + fRam005c83b4;
  fVar15 = fVar14 * fRam005c8408 + fRam005c83b8;
  fVar19 = fVar18 * fRam005c840c + fRam005c83bc;
  fVar8 = fVar7 - _DAT_005c83b0;
  fVar12 = fVar11 - fRam005c83b4;
  fVar16 = fVar15 - fRam005c83b8;
  fVar20 = fVar19 - fRam005c83bc;
  fVar6 = (((fVar6 - _DAT_005c83f0 * fVar8) - _DAT_005c83e0 * fVar8) - _DAT_005c83d0 * fVar8) -
          _DAT_005c83c0 * fVar8;
  fVar8 = (((fVar10 - fRam005c83f4 * fVar12) - fRam005c83e4 * fVar12) - fRam005c83d4 * fVar12) -
          fRam005c83c4 * fVar12;
  fVar10 = (((fVar14 - fRam005c83f8 * fVar16) - fRam005c83e8 * fVar16) - fRam005c83d8 * fVar16) -
           fRam005c83c8 * fVar16;
  fVar12 = (((fVar18 - fRam005c83fc * fVar20) - fRam005c83ec * fVar20) - fRam005c83dc * fVar20) -
           fRam005c83cc * fVar20;
  fVar14 = fVar6 * fVar6;
  fVar16 = fVar8 * fVar8;
  fVar18 = fVar10 * fVar10;
  fVar20 = fVar12 * fVar12;
  uVar25 = (uint)fVar7 & (uint)PTR_DAT_005c8380;
  uVar26 = (uint)fVar11 & (uint)PTR_DAT_005c8384;
  uVar27 = (uint)fVar15 & (uint)PTR_DAT_005c8388;
  uVar28 = (uint)fVar19 & (uint)PTR_DAT_005c838c;
  uVar9 = uVar25 & _DAT_005c8370;
  uVar13 = uVar26 & uRam005c8374;
  uVar17 = uVar27 & uRam005c8378;
  uVar21 = uVar28 & uRam005c837c;
  uVar1 = uVar26 - uVar13 & _DAT_005c8360;
  uVar2 = uVar27 - uVar17 & _DAT_005c8360;
  uVar3 = uVar28 - uVar21 & _DAT_005c8360;
  uVar4 = uVar25 + uVar9 & _DAT_005c8360;
  uVar26 = uVar26 + uVar13 & _DAT_005c8360;
  uVar27 = uVar27 + uVar17 & _DAT_005c8360;
  uVar28 = uVar28 + uVar21 & _DAT_005c8360;
  uVar5 = -(uint)((float)(uVar9 << 0x1e) != 0.0);
  uVar13 = -(uint)((float)(uVar13 << 0x1e) != 0.0);
  uVar17 = -(uint)((float)(uVar17 << 0x1e) != 0.0);
  uVar21 = -(uint)((float)(uVar21 << 0x1e) != 0.0);
  fVar6 = (((_DAT_005c83a0 * fVar14 + _DAT_005c8350) * fVar14 + _DAT_005c8330) * fVar14 +
          _DAT_005c8310) * fVar6;
  fVar8 = (((fRam005c83a4 * fVar16 + fRam005c8354) * fVar16 + fRam005c8334) * fVar16 + fRam005c8314)
          * fVar8;
  fVar10 = (((fRam005c83a8 * fVar18 + fRam005c8358) * fVar18 + fRam005c8338) * fVar18 + fRam005c8318
           ) * fVar10;
  fVar12 = (((fRam005c83ac * fVar20 + fRam005c835c) * fVar20 + fRam005c833c) * fVar20 + fRam005c831c
           ) * fVar12;
  fVar7 = (((_DAT_005c8390 * fVar14 + _DAT_005c8340) * fVar14 + _DAT_005c8320) * fVar14 +
          _DAT_005c8300) * fVar14 + _DAT_005c82f0;
  fVar11 = (((fRam005c8394 * fVar16 + fRam005c8344) * fVar16 + fRam005c8324) * fVar16 + fRam005c8304
           ) * fVar16 + fRam005c82f4;
  fVar14 = (((fRam005c8398 * fVar18 + fRam005c8348) * fVar18 + fRam005c8328) * fVar18 + fRam005c8308
           ) * fVar18 + fRam005c82f8;
  fVar15 = (((fRam005c839c * fVar20 + fRam005c834c) * fVar20 + fRam005c832c) * fVar20 + fRam005c830c
           ) * fVar20 + fRam005c82fc;
  *param_2 = (~uVar5 & (uint)fVar6 | uVar5 & (uint)fVar7) ^
             (uVar25 - uVar9 & _DAT_005c8360) << 0x1e ^ *param_1 & _DAT_005c8410;
  param_2[1] = (~uVar13 & (uint)fVar8 | uVar13 & (uint)fVar11) ^ uVar1 << 0x1e ^ uVar22;
  param_2[2] = (~uVar17 & (uint)fVar10 | uVar17 & (uint)fVar14) ^ uVar2 << 0x1e ^ uVar23;
  param_2[3] = (~uVar21 & (uint)fVar12 | uVar21 & (uint)fVar15) ^ uVar3 << 0x1e ^ uVar24;
  *param_3 = (uVar5 & (uint)fVar6 | ~uVar5 & (uint)fVar7) ^ uVar4 << 0x1e;
  param_3[1] = (uVar13 & (uint)fVar8 | ~uVar13 & (uint)fVar11) ^ uVar26 << 0x1e;
  param_3[2] = (uVar17 & (uint)fVar10 | ~uVar17 & (uint)fVar14) ^ uVar27 << 0x1e;
  param_3[3] = (uVar21 & (uint)fVar12 | ~uVar21 & (uint)fVar15) ^ uVar28 << 0x1e;
  return;
}

