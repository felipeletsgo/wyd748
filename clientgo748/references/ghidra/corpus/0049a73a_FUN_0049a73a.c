// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049a73a | Name: FUN_0049a73a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0049a73a(int param_1,undefined4 param_2)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  int local_c;
  
  if (DAT_0067cf38 == param_1) {
    if (*(int *)(param_1 + 0x28) != 0) {
      (**(code **)(**(int **)(param_1 + 0x28) + 0x20))(param_2);
    }
    if (-1 < *(short *)(param_1 + 0x20098)) {
      FUN_0049ddbd();
    }
    if (*(int *)(param_1 + 0x54) != 1) {
      *(undefined4 *)(param_1 + 0x7c) = 0;
      *(undefined4 *)(param_1 + 0x80) = 0;
      iVar4 = *(int *)(param_1 + 0x4c);
      if ((iVar4 != 0) && (*(int *)(param_1 + 0x40) != 0)) {
        fVar1 = *(float *)(iVar4 + 0x28) - *(float *)(*(int *)(param_1 + 0x40) + 0x20a28);
        fVar2 = *(float *)(iVar4 + 0x2c) - *(float *)(*(int *)(param_1 + 0x40) + 0x20a2c);
        local_c = 0x10;
        iVar4 = __ftol();
        if ((iVar4 >> 7 < 0x1b) ||
           (((iVar4 = __ftol(), 0x1e < iVar4 >> 7 || (iVar4 = __ftol(), iVar4 >> 7 < 0x15)) ||
            (iVar4 = __ftol(), 0x18 < iVar4 >> 7)))) {
          bVar3 = false;
        }
        else {
          bVar3 = true;
        }
        if (bVar3) {
          local_c = 0x14;
        }
        if ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 0x11) &&
           (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 10)) {
          local_c = 0x14;
        }
        if ((fVar1 < _DAT_005a3534) || ((float)local_c <= fVar1)) {
          if ((_DAT_005a3534 <= fVar1) || (fVar1 <= (float)-local_c)) {
            if ((fVar1 <= (float)(0x80 - local_c)) || (_DAT_005a44a8 <= fVar1)) {
              if ((_DAT_005a44a8 <= fVar1) &&
                 ((fVar1 < (float)(local_c + 0x80) &&
                  (*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0)))) {
                *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0xb0);
                uVar5 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
                if ((int)uVar5 < 0) {
                  uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
                }
                *(uint *)(param_1 + 0x78) = uVar5;
                FUN_00534968();
              }
            }
            else if (*(int *)(*(int *)(param_1 + 0x40) + 0xb0) == 0) {
              FUN_0049add2(1);
            }
          }
          else if (*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) {
            *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0xac);
            uVar5 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
            }
            *(uint *)(param_1 + 0x78) = uVar5;
            FUN_00534968();
          }
        }
        else if (*(int *)(*(int *)(param_1 + 0x40) + 0xac) == 0) {
          FUN_0049add2(0);
        }
        if ((fVar2 < _DAT_005a3534) || ((float)local_c <= fVar2)) {
          if ((_DAT_005a3534 <= fVar2) || (fVar2 <= (float)-local_c)) {
            if ((fVar2 <= (float)(0x80 - local_c)) || (128.0 <= fVar2)) {
              if ((128.0 <= fVar2) &&
                 ((fVar2 < (float)(local_c + 0x80) &&
                  (*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0)))) {
                *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0xb8);
                uVar5 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
                if ((int)uVar5 < 0) {
                  uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
                }
                *(uint *)(param_1 + 0x78) = uVar5;
                FUN_00534968();
              }
            }
            else if (*(int *)(*(int *)(param_1 + 0x40) + 0xb8) == 0) {
              FUN_0049add2(3);
            }
          }
          else if (*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) {
            *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0xb4);
            uVar5 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
            }
            *(uint *)(param_1 + 0x78) = uVar5;
            FUN_00534968();
          }
        }
        else if (*(int *)(*(int *)(param_1 + 0x40) + 0xb4) == 0) {
          FUN_0049add2(2);
        }
      }
    }
  }
  return 1;
}

