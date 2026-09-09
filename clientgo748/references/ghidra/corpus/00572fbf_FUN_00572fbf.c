// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572fbf | Name: FUN_00572fbf


uint FUN_00572fbf(int *param_1)

{
  uint uVar1;
  
  if (param_1[0x5e] == 0) {
    uVar1 = FUN_00572c64(param_1);
    if ((char)uVar1 == '\0') goto LAB_00573039;
  }
  if (param_1[0x5e] == *(int *)(param_1[100] + 0x54) + 0xd0) {
    *(undefined4 *)(*param_1 + 0x14) = 0x61;
    *(undefined4 *)(*param_1 + 0x18) = *(undefined4 *)(param_1[100] + 0x54);
    (**(code **)(*param_1 + 4))(param_1,3);
    param_1[0x5e] = 0;
  }
  else {
    uVar1 = (**(code **)(param_1[5] + 0x14))(param_1,*(int *)(param_1[100] + 0x54));
    if ((char)uVar1 == '\0') {
LAB_00573039:
      return uVar1 & 0xffffff00;
    }
  }
  *(uint *)(param_1[100] + 0x54) = *(int *)(param_1[100] + 0x54) + 1U & 7;
  return 1;
}

