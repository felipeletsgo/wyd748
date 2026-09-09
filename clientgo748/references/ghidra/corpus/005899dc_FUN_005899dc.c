// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005899dc | Name: FUN_005899dc


undefined4 * FUN_005899dc(int param_1,undefined4 param_2,int param_3)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = (undefined4 *)(**(code **)(param_1 + 0x20))(*(undefined4 *)(param_1 + 0x28),1,0x40);
  if (puVar1 != (undefined4 *)0x0) {
    iVar2 = (**(code **)(param_1 + 0x20))(*(undefined4 *)(param_1 + 0x28),8,0x5a0);
    puVar1[9] = iVar2;
    if (iVar2 != 0) {
      iVar2 = (**(code **)(param_1 + 0x20))(*(undefined4 *)(param_1 + 0x28),1,param_3);
      puVar1[10] = iVar2;
      if (iVar2 == 0) {
        (**(code **)(param_1 + 0x24))(*(undefined4 *)(param_1 + 0x28),puVar1[9]);
        (**(code **)(param_1 + 0x24))(*(undefined4 *)(param_1 + 0x28),puVar1);
        return (undefined4 *)0x0;
      }
      *puVar1 = 0;
      puVar1[0xb] = iVar2 + param_3;
      puVar1[0xe] = param_2;
      FUN_00589971(puVar1,param_1,0);
      return puVar1;
    }
    (**(code **)(param_1 + 0x24))(*(undefined4 *)(param_1 + 0x28),puVar1);
  }
  return (undefined4 *)0x0;
}

