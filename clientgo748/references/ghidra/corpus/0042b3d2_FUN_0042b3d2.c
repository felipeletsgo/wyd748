// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b3d2 | Name: FUN_0042b3d2


int __fastcall FUN_0042b3d2(int param_1)

{
  int iVar1;
  int local_8;
  
  if ((**(int **)(param_1 + 4) == 0) || (*(int *)(param_1 + 0xc) == 0)) {
    iVar1 = -0x7ffbfe10;
  }
  else {
    *(undefined4 *)(param_1 + 0x14) = 0;
    *(undefined4 *)(param_1 + 0x18) = 0;
    *(undefined4 *)(param_1 + 0x20) = 0;
    *(undefined4 *)(param_1 + 0x24) = 0;
    iVar1 = FUN_0042ab3f(**(undefined4 **)(param_1 + 4),&local_8);
    if ((-1 < iVar1) &&
       ((local_8 == 0 || (iVar1 = FUN_0042a97c(**(undefined4 **)(param_1 + 4),0), -1 < iVar1)))) {
      FUN_0042b9e9();
      iVar1 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 4) + 0x34))
                        (**(undefined4 **)(param_1 + 4),0);
    }
  }
  return iVar1;
}

