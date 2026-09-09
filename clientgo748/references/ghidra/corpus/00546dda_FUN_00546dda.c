// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00546dda | Name: FUN_00546dda


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall FUN_00546dda(int param_1,float param_2)

{
  POINT pt;
  float fVar1;
  LONG LVar2;
  LONG LVar3;
  BOOL BVar4;
  float10 fVar5;
  undefined4 local_8;
  
  LVar2 = __ftol();
  LVar3 = __ftol();
  local_8 = -100.0;
  pt.y = LVar3;
  pt.x = LVar2;
  BVar4 = PtInRect((RECT *)(param_1 + 0x168),pt);
  if (BVar4 != 0) {
    fVar1 = ((float)(*(uint *)(param_1 + 0x178) % 12000) / _DAT_005a5b78) * _DAT_005a4148;
    fVar5 = (float10)FUN_00423b00(fVar1 + fVar1 +
                                  ((param_2 - (float)*(int *)(param_1 + 0x168)) * _DAT_005a4148) /
                                  _DAT_005a365c);
    local_8 = (float)(fVar5 * (float10)_DAT_005a4214 - (float10)_DAT_005a4214);
  }
  return (float10)local_8;
}

