// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056985c | Name: FUN_0056985c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint * FUN_0056985c(int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  uint *puVar4;
  uint uVar5;
  uint uVar6;
  uint unaff_EBX;
  uint *puVar7;
  uint unaff_EDI;
  undefined2 in_FPUControlWord;
  float10 fVar8;
  uint local_8;
  
  puVar4 = operator_new(unaff_EBX << 4);
  if (puVar4 != (uint *)0x0) {
    fVar1 = (float)(int)unaff_EDI;
    if ((int)unaff_EDI < 0) {
      fVar1 = fVar1 + _DAT_005a67dc;
    }
    fVar2 = (float)(int)unaff_EBX;
    if ((int)unaff_EBX < 0) {
      fVar2 = fVar2 + _DAT_005a67dc;
    }
    local_8 = CONCAT22(local_8._2_2_,in_FPUControlWord);
    DAT_013b738c = local_8;
    local_8 = 0;
    puVar7 = puVar4;
    if (unaff_EBX != 0) {
      do {
        fVar3 = (float)(int)local_8;
        if ((int)local_8 < 0) {
          fVar3 = fVar3 + _DAT_005a67dc;
        }
        fVar3 = fVar3 * (fVar1 / fVar2) - _DAT_005a67d4;
        fVar8 = (float10)FUN_00592f0d((double)fVar3);
        uVar5 = (uint)ROUND((float)fVar8);
        uVar6 = uVar5 + 1;
        if ((int)uVar5 < 0) {
          uVar5 = -(uint)(param_1 != 0) & unaff_EDI - 1;
        }
        if (unaff_EDI <= uVar6) {
          uVar6 = ~-(uint)(param_1 != 0) & unaff_EDI - 1;
        }
        *puVar7 = uVar5;
        puVar7[2] = uVar6;
        local_8 = local_8 + 1;
        fVar3 = _DAT_005a67d8 - (fVar3 - (float)fVar8);
        puVar7[1] = (uint)fVar3;
        puVar7[3] = (uint)(1.0 - fVar3);
        puVar7 = puVar7 + 4;
      } while (local_8 < unaff_EBX);
    }
  }
  return puVar4;
}

