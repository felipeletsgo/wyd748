// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f7482 | Name: FUN_004f7482


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f7482(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined1 local_30 [12];
  undefined4 local_24;
  float local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  float local_10;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x1ac) != 0) {
    iVar1 = FUN_0040c0f0();
    if ((iVar1 == 1) && (*(int *)(*(int *)(param_1 + 0x23c) + 0x1c4) != -1)) {
      (**(code **)(**(int **)(param_1 + 0x23c) + 0x60))(1);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x23c) + 0x60))(0);
    }
    (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(*(undefined4 *)(param_1 + 0x2dc));
    if ((*(int *)(param_1 + 0x1b4) == 1) || (*(int *)(DAT_005d0504 + 0x438) == 1)) {
      if ((*(int *)(param_1 + 0x1b4) == 1) && (*(int *)(DAT_005d0504 + 0x438) == 1)) {
        (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffff);
      }
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      local_24 = *(undefined4 *)(param_1 + 0x28);
      local_1c = *(undefined4 *)(param_1 + 0x2c);
      local_20 = *(float *)(param_1 + 0x30) + _DAT_005a40ec;
      thunk_FUN_00561462(local_30,&local_24,DAT_005ccf9c + 0x2a62c);
      thunk_FUN_00561462(local_18,local_30,DAT_005ccf9c + 0x2a5ec);
      if ((local_10 < _DAT_005a3534) || (_DAT_005a3660 <= local_10)) {
        (**(code **)(**(int **)(param_1 + 0x1ac) + 0x60))(0);
      }
      else {
        iVar1 = *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4);
        uVar4 = 0;
        local_8 = __ftol();
        iVar2 = *(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8);
        uVar3 = 0;
        local_c = __ftol();
        if ((((local_8 < 1) ||
             (*(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4) <= local_8)) ||
            (local_c < 1)) ||
           (*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8) <= local_c)) {
          (**(code **)(**(int **)(param_1 + 0x1ac) + 0x60))(0);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x1ac) + 0x60))(1);
          FUN_004012c6((float)local_8 - _DAT_005a5574,(float)local_c);
          if (*(int *)(param_1 + 0x23c) != 0) {
            (**(code **)(**(int **)(param_1 + 0x23c) + 0x70))
                      (*(float *)(*(int *)(param_1 + 0x1ac) + 0x4c) + _DAT_005a3670,
                       *(float *)(*(int *)(param_1 + 0x1ac) + 0x50) + _DAT_005a40f0,iVar2,uVar3,
                       iVar1,uVar4);
            (**(code **)(**(int **)(param_1 + 0x23c) + 0x60))(1);
          }
        }
      }
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x1ac) + 0x60))(0);
    }
  }
  if (*(int *)(param_1 + 0x248) == -1) {
    (**(code **)(**(int **)(param_1 + 0x23c) + 0x60))(0);
  }
  return;
}

