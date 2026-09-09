// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042ac98 | Name: FUN_0042ac98


undefined4 __thiscall FUN_0042ac98(int param_1,uint param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else if (param_2 < *(uint *)(param_1 + 0x10)) {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 4) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

