// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586ef2 | Name: FUN_00586ef2


void FUN_00586ef2(int param_1,undefined4 param_2,int param_3,undefined4 *param_4)

{
  undefined1 uVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  undefined1 *puVar5;
  
  puVar4 = param_4;
  param_4 = (undefined4 *)0x0;
  puVar4 = (undefined4 *)*puVar4;
  if (0 < *(int *)(param_1 + 0x110)) {
    param_3 = param_3 - (int)puVar4;
    do {
      puVar2 = (undefined1 *)*puVar4;
      puVar5 = *(undefined1 **)(param_3 + (int)puVar4);
      puVar3 = puVar2 + *(int *)(param_1 + 0x5c);
      for (; puVar2 < puVar3; puVar2 = puVar2 + 2) {
        uVar1 = *puVar5;
        puVar5 = puVar5 + 1;
        *puVar2 = uVar1;
        puVar2[1] = uVar1;
      }
      param_4 = (undefined4 *)((int)param_4 + 1);
      puVar4 = puVar4 + 1;
    } while ((int)param_4 < *(int *)(param_1 + 0x110));
  }
  return;
}

