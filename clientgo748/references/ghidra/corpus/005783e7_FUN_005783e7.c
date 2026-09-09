// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005783e7 | Name: FUN_005783e7


void FUN_005783e7(int param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,0,0x18);
  *(undefined4 **)(param_1 + 0x18c) = puVar1;
  *(undefined1 *)(puVar1 + 4) = 0;
  *(undefined1 *)((int)puVar1 + 0x11) = 0;
  *puVar1 = &LAB_0057830c;
  puVar1[1] = FUN_0057839f;
  puVar1[2] = FUN_005782d2;
  puVar1[3] = &LAB_005783d6;
  *(undefined1 *)(puVar1 + 5) = 1;
  return;
}

