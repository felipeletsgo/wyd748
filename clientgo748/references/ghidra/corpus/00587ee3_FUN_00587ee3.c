// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587ee3 | Name: FUN_00587ee3


void FUN_00587ee3(int param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x30);
  *(undefined4 **)(param_1 + 0x19c) = puVar1;
  *(undefined1 *)(puVar1 + 2) = 0;
  *puVar1 = &LAB_00587abb;
  puVar1[10] = *(int *)(param_1 + 100) * *(int *)(param_1 + 0x5c);
  if (*(int *)(param_1 + 0x110) == 2) {
    puVar1[1] = FUN_00587ad0;
    puVar1[3] = FUN_00587cfd;
    uVar2 = (**(code **)(*(int *)(param_1 + 4) + 4))(param_1,1,puVar1[10]);
    puVar1[8] = uVar2;
  }
  else {
    puVar1[8] = 0;
    puVar1[1] = FUN_00587b76;
    puVar1[3] = FUN_00587ba7;
  }
  FUN_00587a0b();
  return;
}

