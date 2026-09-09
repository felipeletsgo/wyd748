// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004292b0 | Name: FUN_004292b0


int __thiscall FUN_004292b0(int param_1,uint param_2)

{
  if ((param_2 & 2) == 0) {
    FUN_00428344();
    if ((param_2 & 1) != 0) {
      FUN_0058ea3a(param_1);
    }
  }
  else {
    FUN_005903c2(param_1,0x18,*(undefined4 *)(param_1 + -4),FUN_00428344);
    if ((param_2 & 1) != 0) {
      FUN_0058ea3a(param_1 + -4);
    }
    param_1 = param_1 + -4;
  }
  return param_1;
}

