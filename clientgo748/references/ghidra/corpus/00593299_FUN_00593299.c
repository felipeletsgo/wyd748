// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593299 | Name: FUN_00593299


int FUN_00593299(undefined8 *param_1,int param_2,int param_3)

{
  undefined1 local_2c [24];
  int local_14;
  int local_10;
  
  FUN_0059ab97(*param_1,&local_14,local_2c);
  FUN_0059ab20((uint)(local_14 == 0x2d) + param_2,local_10 + param_3,&local_14);
  FUN_005932ee(param_2,param_3,&local_14,0);
  return param_2;
}

