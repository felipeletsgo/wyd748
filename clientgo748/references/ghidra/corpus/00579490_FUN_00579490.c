// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00579490 | Name: FUN_00579490


void FUN_00579490(int param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x1c);
  *(undefined4 **)(param_1 + 0x17c) = puVar1;
  *(undefined1 *)(puVar1 + 2) = 0;
  *puVar1 = &LAB_00579342;
  puVar1[1] = &LAB_0057946f;
  FUN_00579195();
  return;
}

