// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406ae4 | Name: FUN_00406ae4


undefined4 __thiscall FUN_00406ae4(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x2c) == 0) {
    return 0;
  }
  if (*(int *)(param_1 + 0x30) == 1) {
    if (param_2 == 0x21) {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x5c))(*(undefined4 *)(param_1 + 0x44),2);
      return uVar1;
    }
    if (param_2 == 0x26) {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x5c))(*(undefined4 *)(param_1 + 0x44),3);
      return uVar1;
    }
    if (param_2 == 0x22) {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x5c))(*(undefined4 *)(param_1 + 0x44),4);
      return uVar1;
    }
    if (param_2 == 0x28) {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x5c))(*(undefined4 *)(param_1 + 0x44),5);
      return uVar1;
    }
    if (param_2 == 0x2e) {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x5c))(*(undefined4 *)(param_1 + 0x44),6);
      return uVar1;
    }
  }
  return 0;
}

