// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573156 | Name: FUN_00573156


void FUN_00573156(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,0,0x5c);
  *(undefined4 **)(param_1 + 400) = puVar1;
  *puVar1 = FUN_0057312c;
  *(undefined1 **)(*(int *)(param_1 + 400) + 4) = &LAB_00572dd2;
  *(code **)(*(int *)(param_1 + 400) + 8) = FUN_00572fbf;
  *(code **)(*(int *)(param_1 + 400) + 0xc) = FUN_00572bc8;
  iVar2 = 0x10;
  do {
    *(code **)(iVar2 + *(int *)(param_1 + 400)) = FUN_00572bc8;
    iVar2 = iVar2 + 4;
  } while (iVar2 < 0x50);
  *(code **)(*(int *)(param_1 + 400) + 0x10) = FUN_00572127;
  *(code **)(*(int *)(param_1 + 400) + 0x48) = FUN_00572363;
  FUN_0057312c(param_1);
  return;
}

