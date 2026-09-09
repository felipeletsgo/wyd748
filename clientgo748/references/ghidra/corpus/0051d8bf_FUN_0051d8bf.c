// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051d8bf | Name: FUN_0051d8bf


void __thiscall FUN_0051d8bf(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_c;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    local_c = 0x1b;
    if (*(int *)(param_1 + 0x5c) == 3) {
      uVar1 = (**(code **)(*DAT_0092e654 + 8))();
      if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x1b), iVar2 != 0)) {
        FUN_00429a6d(0x1b);
        iVar2 = FUN_0042afd0();
        if ((iVar2 == 0) ||
           ((*(short *)(param_1 + 0x7b6) != param_3 || (*(int *)(param_1 + 0x7c0) + 400U < uVar1))))
        {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x1b);
          FUN_0042ad2b(uVar3,uVar4);
          *(uint *)(param_1 + 0x7c0) = uVar1;
        }
      }
      *(undefined2 *)(param_1 + 0x7b6) = (undefined2)param_3;
    }
    else if (*(int *)(param_1 + 0x24c) == 0x28) {
      uVar1 = (**(code **)(*DAT_0092e654 + 8))();
      if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x1b), iVar2 != 0)) {
        FUN_00429a6d(0x1b);
        iVar2 = FUN_0042afd0();
        if ((iVar2 == 0) ||
           ((*(short *)(param_1 + 0x7b6) != param_3 || (*(int *)(param_1 + 0x7c0) + 400U < uVar1))))
        {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x1b);
          FUN_0042ad2b(uVar3,uVar4);
          *(uint *)(param_1 + 0x7c0) = uVar1;
        }
      }
      *(undefined2 *)(param_1 + 0x7b6) = (undefined2)param_3;
    }
    else {
      if (param_2 == 1) {
        local_c = 0x15;
      }
      if (param_2 == 2) {
        local_c = 0x16;
      }
      if (param_2 == 3) {
        local_c = 0x17;
      }
      if (((param_2 == 0xb) || (param_2 == 0x1f)) || (param_2 == 0x3d)) {
        local_c = 0x19;
      }
      if (((param_2 == 0xc) || (param_2 == 0x20)) || (param_2 == 0x3e)) {
        local_c = 0x1a;
      }
      if (((param_2 == 0xd) || (param_2 == 0x21)) || (param_2 == 0x3f)) {
        local_c = 0x1b;
      }
      if ((((param_2 == 0x15) || (param_2 == 0x16)) || (param_2 == 0x17)) || (param_2 == 0x29)) {
        local_c = 0x1c;
      }
      if ((100 < param_2) && (param_2 < 0x69)) {
        local_c = 0x18;
      }
      uVar1 = (**(code **)(*DAT_0092e654 + 8))();
      if (((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(local_c), iVar2 != 0)) && (local_c != 0x18))
      {
        FUN_00429a6d(local_c);
        iVar2 = FUN_0042afd0();
        if (((iVar2 == 0) || (*(short *)(param_1 + 0x7b6) != param_3)) ||
           (*(int *)(param_1 + 0x7c0) + 400U < uVar1)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(local_c);
          FUN_0042ad2b(uVar3,uVar4);
          *(uint *)(param_1 + 0x7c0) = uVar1;
        }
      }
      *(undefined2 *)(param_1 + 0x7b6) = (undefined2)param_3;
    }
  }
  return;
}

