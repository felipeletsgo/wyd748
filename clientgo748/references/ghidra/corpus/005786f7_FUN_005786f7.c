// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005786f7 | Name: FUN_005786f7


void FUN_005786f7(int *param_1,int param_2,undefined1 param_3,undefined4 param_4,undefined4 param_5,
                 undefined4 param_6)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = param_1[1];
  if (param_2 != 1) {
    *(undefined4 *)(*param_1 + 0x14) = 0xc;
    *(int *)(*param_1 + 0x18) = param_2;
    (**(code **)*param_1)(param_1);
  }
  puVar2 = (undefined4 *)FUN_0057843d(param_1,param_2,0x78);
  *puVar2 = 0;
  puVar2[1] = param_5;
  puVar2[2] = param_4;
  puVar2[3] = param_6;
  *(undefined1 *)((int)puVar2 + 0x22) = 0;
  *(undefined1 *)(puVar2 + 8) = param_3;
  puVar2[9] = *(undefined4 *)(iVar1 + 0x40);
  *(undefined4 **)(iVar1 + 0x40) = puVar2;
  return;
}

