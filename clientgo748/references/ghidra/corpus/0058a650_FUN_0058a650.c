// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058a650 | Name: FUN_0058a650


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0058a650(uint *param_1,uint *param_2,uint *param_3)

{
  float fVar1;
  float fVar2;
  uint uVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  float fVar7;
  float fVar8;
  uint uVar9;
  float fVar10;
  float fVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  uint uVar15;
  float fVar16;
  float fVar17;
  uint uVar18;
  uint uVar19;
  uint uVar20;
  uint uVar21;
  uint uVar22;
  uint uVar23;
  uint uVar24;
  float fVar25;
  float fVar26;
  uint uVar27;
  uint uVar28;
  
  fVar16 = (float)(*param_1 & _DAT_005c8640);
  fVar17 = (float)(param_1[1] & uRam005c8644);
  fVar25 = (float)(param_1[2] & uRam005c8648);
  fVar26 = (float)(param_1[3] & uRam005c864c);
  uVar13 = param_1[1] & uRam005c8634;
  uVar14 = param_1[2] & uRam005c8638;
  uVar15 = param_1[3] & uRam005c863c;
  fVar1 = fVar16 * _DAT_005c8620 + _DAT_005c85d0;
  fVar4 = fVar17 * fRam005c8624 + fRam005c85d4;
  fVar7 = fVar25 * fRam005c8628 + fRam005c85d8;
  fVar10 = fVar26 * fRam005c862c + fRam005c85dc;
  fVar2 = fVar1 - _DAT_005c85d0;
  fVar5 = fVar4 - fRam005c85d4;
  fVar8 = fVar7 - fRam005c85d8;
  fVar11 = fVar10 - fRam005c85dc;
  fVar16 = (((fVar16 - _DAT_005c8610 * fVar2) - _DAT_005c8600 * fVar2) - _DAT_005c85f0 * fVar2) -
           _DAT_005c85e0 * fVar2;
  fVar2 = (((fVar17 - fRam005c8614 * fVar5) - fRam005c8604 * fVar5) - fRam005c85f4 * fVar5) -
          fRam005c85e4 * fVar5;
  fVar17 = (((fVar25 - fRam005c8618 * fVar8) - fRam005c8608 * fVar8) - fRam005c85f8 * fVar8) -
           fRam005c85e8 * fVar8;
  fVar5 = (((fVar26 - fRam005c861c * fVar11) - fRam005c860c * fVar11) - fRam005c85fc * fVar11) -
          fRam005c85ec * fVar11;
  fVar25 = fVar16 * fVar16;
  fVar8 = fVar2 * fVar2;
  fVar26 = fVar17 * fVar17;
  fVar11 = fVar5 * fVar5;
  uVar18 = (uint)fVar1 & (uint)PTR_DAT_005c85a0;
  uVar19 = (uint)fVar4 & (uint)PTR_DAT_005c85a4;
  uVar21 = (uint)fVar7 & (uint)PTR_DAT_005c85a8;
  uVar23 = (uint)fVar10 & (uint)PTR_DAT_005c85ac;
  uVar3 = uVar18 & _DAT_005c8590;
  uVar6 = uVar19 & uRam005c8594;
  uVar9 = uVar21 & uRam005c8598;
  uVar12 = uVar23 & uRam005c859c;
  uVar20 = uVar19 - uVar6 & uRam005c8584;
  uVar22 = uVar21 - uVar9 & uRam005c8588;
  uVar24 = uVar23 - uVar12 & uRam005c858c;
  uVar27 = uVar18 + uVar3 & _DAT_005c8580;
  uVar28 = uVar19 + uVar6 & uRam005c8584;
  uVar21 = uVar21 + uVar9 & uRam005c8588;
  uVar23 = uVar23 + uVar12 & uRam005c858c;
  uVar19 = -(uint)((float)(uVar3 << 0x1e) != 0.0);
  uVar6 = -(uint)((float)(uVar6 << 0x1e) != 0.0);
  uVar9 = -(uint)((float)(uVar9 << 0x1e) != 0.0);
  uVar12 = -(uint)((float)(uVar12 << 0x1e) != 0.0);
  fVar16 = (((_DAT_005c85c0 * fVar25 + _DAT_005c8570) * fVar25 + _DAT_005c8550) * fVar25 +
           _DAT_005c8530) * fVar16;
  fVar2 = (((fRam005c85c4 * fVar8 + fRam005c8574) * fVar8 + fRam005c8554) * fVar8 + fRam005c8534) *
          fVar2;
  fVar17 = (((fRam005c85c8 * fVar26 + fRam005c8578) * fVar26 + fRam005c8558) * fVar26 + fRam005c8538
           ) * fVar17;
  fVar5 = (((fRam005c85cc * fVar11 + fRam005c857c) * fVar11 + fRam005c855c) * fVar11 + fRam005c853c)
          * fVar5;
  fVar1 = (((_DAT_005c85b0 * fVar25 + _DAT_005c8560) * fVar25 + _DAT_005c8540) * fVar25 +
          _DAT_005c8520) * fVar25 + _DAT_005c8510;
  fVar4 = (((fRam005c85b4 * fVar8 + fRam005c8564) * fVar8 + fRam005c8544) * fVar8 + fRam005c8524) *
          fVar8 + fRam005c8514;
  fVar25 = (((fRam005c85b8 * fVar26 + fRam005c8568) * fVar26 + fRam005c8548) * fVar26 + fRam005c8528
           ) * fVar26 + fRam005c8518;
  fVar7 = (((fRam005c85bc * fVar11 + fRam005c856c) * fVar11 + fRam005c854c) * fVar11 + fRam005c852c)
          * fVar11 + fRam005c851c;
  *param_2 = (~uVar19 & (uint)fVar16 | uVar19 & (uint)fVar1) ^
             (uVar18 - uVar3 & _DAT_005c8580) << 0x1e ^ *param_1 & _DAT_005c8630;
  param_2[1] = (~uVar6 & (uint)fVar2 | uVar6 & (uint)fVar4) ^ uVar20 << 0x1e ^ uVar13;
  param_2[2] = (~uVar9 & (uint)fVar17 | uVar9 & (uint)fVar25) ^ uVar22 << 0x1e ^ uVar14;
  param_2[3] = (~uVar12 & (uint)fVar5 | uVar12 & (uint)fVar7) ^ uVar24 << 0x1e ^ uVar15;
  *param_3 = (uVar19 & (uint)fVar16 | ~uVar19 & (uint)fVar1) ^ uVar27 << 0x1e;
  param_3[1] = (uVar6 & (uint)fVar2 | ~uVar6 & (uint)fVar4) ^ uVar28 << 0x1e;
  param_3[2] = (uVar9 & (uint)fVar17 | ~uVar9 & (uint)fVar25) ^ uVar21 << 0x1e;
  param_3[3] = (uVar12 & (uint)fVar5 | ~uVar12 & (uint)fVar7) ^ uVar23 << 0x1e;
  return;
}

