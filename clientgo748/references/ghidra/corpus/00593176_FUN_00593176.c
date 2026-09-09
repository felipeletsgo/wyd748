// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593176 | Name: FUN_00593176


int FUN_00593176(undefined8 *param_1,int param_2,int param_3,undefined4 param_4)

{
  undefined1 local_2c [24];
  int local_14 [4];
  
  FUN_0059ab97(*param_1,local_14,local_2c);
  FUN_0059ab20((uint)(0 < param_3) + param_2 + (uint)(local_14[0] == 0x2d),param_3 + 1,local_14);
  FUN_005931d7(param_2,param_3,param_4,local_14,0);
  return param_2;
}

