// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005898ea | Name: FUN_005898ea


void FUN_005898ea(int *param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)(**(code **)param_1[1])(param_1,1,0x58);
  param_1[0x69] = (int)puVar1;
  puVar1[0x11] = 0;
  puVar1[0xd] = 0;
  *puVar1 = FUN_0058980c;
  puVar1[2] = &DAT_0058758f;
  puVar1[3] = &LAB_005898d6;
  if (4 < param_1[0x19]) {
    *(undefined4 *)(*param_1 + 0x14) = 0x36;
    *(undefined4 *)(*param_1 + 0x18) = 4;
    (**(code **)*param_1)(param_1);
  }
  if (0x100 < param_1[0x15]) {
    *(undefined4 *)(*param_1 + 0x14) = 0x38;
    *(undefined4 *)(*param_1 + 0x18) = 0x100;
    (**(code **)*param_1)(param_1);
  }
  FUN_00589036();
  FUN_00589163(param_1);
  if (param_1[0x13] == 2) {
    FUN_005897d6();
  }
  return;
}

