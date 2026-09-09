// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058a204 | Name: FUN_0058a204


undefined4 FUN_0058a204(int param_1,int param_2)

{
  FUN_00589971(param_1,param_2,0);
  (**(code **)(param_2 + 0x24))(*(undefined4 *)(param_2 + 0x28),*(undefined4 *)(param_1 + 0x28));
  (**(code **)(param_2 + 0x24))(*(undefined4 *)(param_2 + 0x28),*(undefined4 *)(param_1 + 0x24));
  (**(code **)(param_2 + 0x24))(*(undefined4 *)(param_2 + 0x28),param_1);
  return 0;
}

