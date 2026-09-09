// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053976b | Name: FUN_0053976b


undefined4 __thiscall
FUN_0053976b(int param_1,undefined4 param_2,undefined4 param_3,float param_4,undefined4 *param_5)

{
  POINT pt;
  LONG LVar1;
  LONG LVar2;
  BOOL BVar3;
  int local_8;
  
  LVar1 = __ftol();
  LVar2 = __ftol();
  local_8 = 0;
  while( true ) {
    if (9 < local_8) {
      return 0;
    }
    if ((*(int *)(param_1 + 0x20a34 + local_8 * 4) != 0) &&
       (pt.y = LVar2, pt.x = LVar1,
       BVar3 = PtInRect((RECT *)(*(int *)(param_1 + 0x20a34 + local_8 * 4) + 0x168),pt), BVar3 == 1)
       ) break;
    local_8 = local_8 + 1;
  }
  if (param_4 < *(float *)(*(int *)(param_1 + 0x20a34 + local_8 * 4) + 0x30)) {
    *param_5 = *(undefined4 *)(*(int *)(param_1 + 0x20a34 + local_8 * 4) + 0x30);
    return 1;
  }
  return 0;
}

