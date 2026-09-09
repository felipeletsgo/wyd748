// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00504748 | Name: FUN_00504748


void __thiscall FUN_00504748(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_108;
  undefined1 local_104 [256];
  
  if (*(int *)(param_1 + 0x214) == 0) {
    local_108 = 0x36;
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x608) = uVar1;
    uVar1 = (**(code **)(**(int **)(param_1 + 0x418) + 0x88))(param_2);
    FUN_0058f078(local_104,s__s___s_005bb750,uVar1);
    (**(code **)(**(int **)(param_1 + 0x42c) + 0x80))(local_104,0);
    iVar2 = FUN_005046ce(local_104,&local_108);
    (**(code **)(**(int **)(param_1 + 0x42c) + 0x74))((float)iVar2,(float)local_108);
  }
  return;
}

