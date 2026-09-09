// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042accd | Name: FUN_0042accd


undefined4 __thiscall FUN_0042accd(int param_1,uint param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0x800401f0;
  }
  else if (param_2 < *(uint *)(param_1 + 0x10)) {
    *param_3 = 0;
    uVar1 = (**(code **)**(undefined4 **)(*(int *)(param_1 + 4) + param_2 * 4))
                      (*(undefined4 *)(*(int *)(param_1 + 4) + param_2 * 4),&DAT_005a9e9c,param_3);
  }
  else {
    uVar1 = 0x80070057;
  }
  return uVar1;
}

