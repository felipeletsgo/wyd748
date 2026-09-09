// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00425b1b | Name: FUN_00425b1b


bool __thiscall FUN_00425b1b(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  bool bVar3;
  
  uVar1 = Direct3DCreate8(0xdc);
  *(undefined4 *)(param_1 + 0x2a3e0) = uVar1;
  if (*(int *)(param_1 + 0x2a3e0) == 0) {
    FUN_00431129(0x10000000);
    iVar2 = FUN_00425d36(0x82000001,1);
    bVar3 = -1 < iVar2;
  }
  else {
    *(undefined4 *)(param_1 + 0x2a3d8) = param_2;
    if (*(int *)(param_1 + 0x2a3dc) == 0) {
      *(undefined4 *)(param_1 + 0x2a3dc) = *(undefined4 *)(param_1 + 0x2a3d8);
    }
    GetWindowRect(*(HWND *)(param_1 + 0x2a3d8),(LPRECT)(param_1 + 0x2a4e4));
    GetClientRect(*(HWND *)(param_1 + 0x2a3d8),(LPRECT)(param_1 + 0x2a4f4));
    iVar2 = FUN_00426022();
    if (iVar2 < 0) {
      FUN_00431129(0x10000001);
      if (*(int *)(param_1 + 0x2a3e0) != 0) {
        (**(code **)(**(int **)(param_1 + 0x2a3e0) + 8))(*(undefined4 *)(param_1 + 0x2a3e0));
        *(undefined4 *)(param_1 + 0x2a3e0) = 0;
      }
      iVar2 = FUN_00425d36(iVar2,1);
      bVar3 = -1 < iVar2;
    }
    else {
      iVar2 = FUN_00427119();
      if (iVar2 < 0) {
        FUN_00431129(0x10000002);
        if (*(int *)(param_1 + 0x2a3e0) != 0) {
          (**(code **)(**(int **)(param_1 + 0x2a3e0) + 8))(*(undefined4 *)(param_1 + 0x2a3e0));
          *(undefined4 *)(param_1 + 0x2a3e0) = 0;
        }
        iVar2 = FUN_00425d36(iVar2,1);
        bVar3 = -1 < iVar2;
      }
      else {
        iVar2 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                          (*(undefined4 *)(param_1 + 0x2a3e0),0,1,*(undefined4 *)(param_1 + 0x2a514)
                           ,0,3,0x31545844);
        if (-1 < iVar2) {
          *(undefined4 *)(param_1 + 0x2a5c0) = 1;
        }
        iVar2 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                          (*(undefined4 *)(param_1 + 0x2a3e0),0,1,*(undefined4 *)(param_1 + 0x2a514)
                           ,0,3,0x33545844);
        if (-1 < iVar2) {
          *(undefined4 *)(param_1 + 0x2a5c4) = 1;
        }
        bVar3 = true;
      }
    }
  }
  return bVar3;
}

