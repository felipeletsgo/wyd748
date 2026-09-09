// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044e937 | Name: FUN_0044e937


void __fastcall FUN_0044e937(int param_1)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  int *piVar4;
  
  iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
  if (*(int *)(iVar1 + 0x68) == 0) {
    *(int *)(param_1 + 0x28e54) = *(int *)(param_1 + 0x28e54) + 1;
    if (2 < *(int *)(param_1 + 0x28e54)) {
      *(undefined4 *)(param_1 + 0x28e54) = 0;
    }
    if ((*(int *)(param_1 + 0x28e74) == 1) && (*(int *)(param_1 + 0x28e54) == 1)) {
      *(undefined4 *)(param_1 + 0x28e54) = 2;
    }
    if (*(int *)(param_1 + 0x28e54) == 0) {
      (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
      iVar3 = __ftol();
      if (((iVar3 >> 7 < 0x1b) || (iVar3 = __ftol(), 0x1e < iVar3 >> 7)) ||
         ((iVar3 = __ftol(), iVar3 >> 7 < 0x15 || (iVar3 = __ftol(), 0x18 < iVar3 >> 7)))) {
        bVar2 = false;
      }
      else {
        bVar2 = true;
      }
      if (bVar2) {
        *(undefined4 *)(iVar1 + 0x2c) = 0xbf2421d7;
        *(undefined4 *)(iVar1 + 0x28) = 0x3f81b7a6;
      }
      else {
        *(undefined4 *)(iVar1 + 0x2c) = 0xbf451e9b;
        *(undefined4 *)(iVar1 + 0x28) = 0x3f4d2a4d;
      }
      *(undefined4 *)(iVar1 + 0x24) = 0;
      *(float *)(iVar1 + 0x34) = *(float *)(iVar1 + 0xc0) - 0.0;
      *(float *)(iVar1 + 0x38) = *(float *)(iVar1 + 0xc0) - 0.0;
      *(undefined4 *)(iVar1 + 0x3c) = 0x3ea8f5c3;
      *(undefined4 *)(DAT_005ccf9c + 0x2b038) = 1;
      *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x48) = 0;
      *(undefined4 *)(param_1 + 0x274e8) = 0;
    }
    else if (*(int *)(param_1 + 0x28e54) == 1) {
      (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
      iVar3 = __ftol();
      if ((((iVar3 >> 7 < 0x1b) || (iVar3 = __ftol(), 0x1e < iVar3 >> 7)) ||
          (iVar3 = __ftol(), iVar3 >> 7 < 0x15)) || (iVar3 = __ftol(), 0x18 < iVar3 >> 7)) {
        bVar2 = false;
      }
      else {
        bVar2 = true;
      }
      if (bVar2) {
        *(undefined4 *)(iVar1 + 0x2c) = 0xbf2421d7;
        *(undefined4 *)(iVar1 + 0x28) = 0x3f81b7a6;
      }
      else {
        *(undefined4 *)(iVar1 + 0x2c) = 0xbf451e9b;
        *(undefined4 *)(iVar1 + 0x28) = 0x3f4d2a4d;
      }
      *(undefined4 *)(iVar1 + 0x24) = 0;
      *(undefined4 *)(iVar1 + 0x34) = 0x41300000;
      *(undefined4 *)(iVar1 + 0x38) = 0x41300000;
      *(undefined4 *)(iVar1 + 0x3c) = 0x3ea8f5c3;
      *(undefined4 *)(DAT_005ccf9c + 0x2b038) = 1;
      *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x48) = 0;
      *(undefined4 *)(param_1 + 0x274e8) = 0;
    }
    else if (*(int *)(param_1 + 0x28e54) == 2) {
      *(undefined4 *)(iVar1 + 0x24) = 0;
      *(undefined4 *)(iVar1 + 0x2c) = 0x3dcccccd;
      *(undefined4 *)(iVar1 + 0x34) = 0x40600000;
      *(undefined4 *)(iVar1 + 0x38) = 0x40600000;
      *(undefined4 *)(iVar1 + 0x3c) = 0x3ea8f5c3;
      *(undefined4 *)(iVar1 + 0x28) = 0x40490fdb;
      *(undefined4 *)(DAT_005ccf9c + 0x2b038) = 1;
      *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x48) = 0;
      *(undefined4 *)(param_1 + 0x274e8) = 0;
    }
    piVar4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x134);
    if (piVar4 != (int *)0x0) {
      (**(code **)(*piVar4 + 0x8c))(*(undefined4 *)(param_1 + 0x274e8));
    }
  }
  return;
}

