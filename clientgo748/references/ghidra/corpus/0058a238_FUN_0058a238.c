// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058a238 | Name: FUN_0058a238


uint FUN_0058a238(uint param_1,byte *param_2,uint param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  
  uVar3 = param_1 & 0xffff;
  param_1 = param_1 >> 0x10;
  if (param_2 == (byte *)0x0) {
    uVar3 = 1;
  }
  else {
    while (param_3 != 0) {
      uVar2 = 0x15b0;
      if (param_3 < 0x15b0) {
        uVar2 = param_3;
      }
      param_3 = param_3 - uVar2;
      if (0xf < (int)uVar2) {
        uVar1 = uVar2 >> 4;
        uVar2 = uVar2 + uVar1 * -0x10;
        do {
          iVar4 = uVar3 + *param_2;
          iVar5 = iVar4 + (uint)param_2[1];
          iVar6 = iVar5 + (uint)param_2[2];
          iVar7 = iVar6 + (uint)param_2[3];
          iVar8 = iVar7 + (uint)param_2[4];
          iVar9 = iVar8 + (uint)param_2[5];
          iVar10 = iVar9 + (uint)param_2[6];
          iVar11 = iVar10 + (uint)param_2[7];
          iVar12 = iVar11 + (uint)param_2[8];
          iVar13 = iVar12 + (uint)param_2[9];
          iVar14 = iVar13 + (uint)param_2[10];
          iVar15 = iVar14 + (uint)param_2[0xb];
          iVar16 = iVar15 + (uint)param_2[0xc];
          iVar17 = iVar16 + (uint)param_2[0xd];
          iVar18 = iVar17 + (uint)param_2[0xe];
          uVar3 = iVar18 + (uint)param_2[0xf];
          param_1 = param_1 + iVar4 + iVar5 + iVar6 + iVar7 + iVar8 + iVar9 + iVar10 + iVar11 +
                    iVar12 + iVar13 + iVar14 + iVar15 + iVar16 + iVar17 + iVar18 + uVar3;
          param_2 = param_2 + 0x10;
          uVar1 = uVar1 - 1;
        } while (uVar1 != 0);
      }
      for (; uVar2 != 0; uVar2 = uVar2 - 1) {
        uVar3 = uVar3 + *param_2;
        param_2 = param_2 + 1;
        param_1 = param_1 + uVar3;
      }
      uVar3 = uVar3 % 0xfff1;
      param_1 = param_1 % 0xfff1;
    }
    uVar3 = param_1 << 0x10 | uVar3;
  }
  return uVar3;
}

