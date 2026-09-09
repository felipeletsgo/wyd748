// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047ef1d | Name: FUN_0047ef1d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0047ef1d(int param_1,uint param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  float10 fVar4;
  float10 fVar5;
  int local_48;
  float local_20;
  float local_18;
  float local_8;
  
  if ((*(int *)(param_1 + 0x26e24) != 0) && (*(int *)(param_1 + 0x4c) != 0)) {
    if (*(int *)(param_1 + 0x26e1c) != 0) {
      *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x795) = 0;
    }
    if (*(int *)(param_1 + 0x26e24) == 1) {
      FUN_0047f7da();
    }
    else if (*(int *)(param_1 + 0x26e24) == 2) {
      if (*(int *)(param_1 + 0x26e28) + 0xaf0U < param_2) {
        *(undefined4 *)(param_1 + 0x26e24) = 3;
        *(uint *)(param_1 + 0x26e28) = param_2;
      }
      else {
        fVar4 = (float10)FUN_00423b00();
        local_8 = (float)fVar4;
        if (_DAT_005a40ec <= local_8) {
          local_8 = 0.3;
        }
        if (local_8 < _DAT_005a3534) {
          local_8 = 0.0;
        }
        if (*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) != 0) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) + 0x2e4) = 0xf;
        }
        *(float *)(*(int *)(param_1 + 0x4c) + 0x30) =
             local_8 + *(float *)(*(int *)(param_1 + 0x4c) + 0x30);
      }
    }
    else if (*(int *)(param_1 + 0x26e24) == 3) {
      iVar3 = _rand();
      if ((uint)(*(int *)(param_1 + 0x26e28) + 1000 + iVar3 % 300) <= param_2) {
        *(uint *)(param_1 + 0x26e28) = param_2;
        *(uint *)(param_1 + 0x26e2c) = (uint)(*(int *)(param_1 + 0x26e2c) == 0);
      }
      fVar4 = (float10)FUN_00423b00();
      fVar1 = (float)((float10)(iVar3 % 300) / (float10)_DAT_005a43fc + fVar4);
      if ((*(int *)(param_1 + 0x26e2c) != 0) ||
         (_DAT_005a3658 <= *(float *)(*(int *)(param_1 + 0x4c) + 0x30))) {
        if (_DAT_005a366c < *(float *)(*(int *)(param_1 + 0x4c) + 0x30)) {
          *(float *)(*(int *)(param_1 + 0x4c) + 0x30) =
               *(float *)(*(int *)(param_1 + 0x4c) + 0x30) - fVar1;
        }
      }
      else {
        *(float *)(*(int *)(param_1 + 0x4c) + 0x30) =
             fVar1 + *(float *)(*(int *)(param_1 + 0x4c) + 0x30);
      }
      local_20 = 0.0;
      local_18 = 0.0;
      fVar1 = *(float *)(*(int *)(param_1 + 0x4c) + 0x28);
      fVar2 = *(float *)(*(int *)(param_1 + 0x4c) + 0x2c);
      if (fVar1 < (float)*(int *)(&DAT_005b4fc0 +
                                 *(int *)(param_1 + 0x26e44) * 8 +
                                 *(int *)(param_1 + 0x26e40) * 0x50)) {
        if (fVar1 <= (float)*(int *)(&DAT_005b4fc0 +
                                    *(int *)(param_1 + 0x26e44) * 8 +
                                    *(int *)(param_1 + 0x26e40) * 0x50)) {
          local_18 = *(float *)(param_1 + 0x26e38) + 0.0;
        }
      }
      else {
        local_18 = 0.0 - *(float *)(param_1 + 0x26e38);
      }
      if (fVar2 < (float)*(int *)(&DAT_005b4fc4 +
                                 *(int *)(param_1 + 0x26e44) * 8 +
                                 *(int *)(param_1 + 0x26e40) * 0x50)) {
        if (fVar2 <= (float)*(int *)(&DAT_005b4fc4 +
                                    *(int *)(param_1 + 0x26e44) * 8 +
                                    *(int *)(param_1 + 0x26e40) * 0x50)) {
          local_20 = *(float *)(param_1 + 0x26e38) + 0.0;
        }
      }
      else {
        local_20 = 0.0 - *(float *)(param_1 + 0x26e38);
      }
      fVar1 = (float)*(int *)(&DAT_005b4fc0 +
                             *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50) -
              (fVar1 + local_18);
      fVar2 = (float)*(int *)(&DAT_005b4fc4 +
                             *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50) -
              (fVar2 + local_20);
      if ((fVar1 <= _DAT_005a34a0) && (_DAT_005a43f8 <= fVar1)) {
        local_18 = 0.0;
      }
      if ((fVar2 <= _DAT_005a34a0) && (_DAT_005a43f8 <= fVar2)) {
        local_20 = 0.0;
      }
      fVar4 = (float10)FUN_00591303((double)local_18);
      if (((float10)*(float *)(param_1 + 0x26e38) == fVar4) &&
         (fVar4 = (float10)FUN_00591303((double)local_20),
         (float10)*(float *)(param_1 + 0x26e38) == fVar4)) {
        local_18 = local_18 / _DAT_005a365c;
        local_20 = local_20 / _DAT_005a365c;
      }
      if ((local_18 == _DAT_005a3534) && (local_20 == _DAT_005a3534)) {
        *(int *)(param_1 + 0x26e44) = *(int *)(param_1 + 0x26e44) + 1;
        if ((*(int *)(&DAT_005b4fc0 +
                     *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50) == 0) &&
           (*(int *)(&DAT_005b4fc4 +
                    *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50) == 0)) {
          *(undefined4 *)(param_1 + 0x26e24) = 4;
          *(uint *)(param_1 + 0x26e28) = param_2;
        }
        else {
          *(float *)(param_1 + 0x26e30) =
               (float)*(int *)(&DAT_005b4fc0 +
                              *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50);
          *(float *)(param_1 + 0x26e34) =
               (float)*(int *)(&DAT_005b4fc4 +
                              *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50);
          *(uint *)(param_1 + 0x26e28) = param_2;
        }
      }
      else {
        *(float *)(*(int *)(param_1 + 0x4c) + 0x88c) =
             local_18 + *(float *)(*(int *)(param_1 + 0x4c) + 0x88c);
        *(float *)(*(int *)(param_1 + 0x4c) + 0x890) =
             local_20 + *(float *)(*(int *)(param_1 + 0x4c) + 0x890);
        fVar1 = (float)*(int *)(&DAT_005b4fc0 +
                               *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50)
                - *(float *)(*(int *)(param_1 + 0x4c) + 0x28);
        fVar2 = (float)*(int *)(&DAT_005b4fc4 +
                               *(int *)(param_1 + 0x26e44) * 8 + *(int *)(param_1 + 0x26e40) * 0x50)
                - *(float *)(*(int *)(param_1 + 0x4c) + 0x2c);
        local_48 = *(int *)(param_1 + 0x26e44);
        if (9 < local_48) {
          local_48 = 9;
        }
        fVar4 = (float10)FUN_00591303((double)fVar1);
        fVar5 = (float10)FUN_00591303((double)fVar2);
        if (((float10)_DAT_005a43f0 <= fVar5 + (float10)(double)fVar4) ||
           ((local_48 != 9 &&
            ((*(int *)(&DAT_005b4fc8 + local_48 * 8 + *(int *)(param_1 + 0x26e40) * 0x50) != 0 ||
             (*(int *)(&DAT_005b4fcc + local_48 * 8 + *(int *)(param_1 + 0x26e40) * 0x50) != 0))))))
        {
          if (*(float *)(param_1 + 0x26e38) <= _DAT_005a4160) {
            fVar4 = (float10)FUN_00423b00();
            *(float *)(param_1 + 0x26e38) = (float)(fVar4 + (float10)*(float *)(param_1 + 0x26e38));
          }
        }
        else if (_DAT_005a4158 <= *(float *)(param_1 + 0x26e38)) {
          fVar4 = (float10)FUN_00423b00();
          *(float *)(param_1 + 0x26e38) = (float)((float10)*(float *)(param_1 + 0x26e38) - fVar4);
        }
        if (*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) != 0) {
          iVar3 = __ftol();
          *(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) + 0x2e4) = 0xf - iVar3;
        }
        fVar4 = (float10)FUN_00493c30(fVar1,fVar2);
        *(float *)(*(int *)(param_1 + 0x4c) + 0x22c) = (float)(fVar4 + (float10)_DAT_005a4380);
      }
    }
    else if (*(int *)(param_1 + 0x26e24) == 4) {
      if (*(int *)(param_1 + 0x26e28) + 0xaf0U < param_2) {
        *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x30) =
             *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x228);
        *(undefined4 *)(param_1 + 0x26e24) = 5;
        *(uint *)(param_1 + 0x26e28) = param_2;
      }
      else {
        fVar4 = (float10)FUN_00423b00();
        if (*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) != 0) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) + 0x2e4) = 0xf;
        }
        if (_DAT_005a3534 < *(float *)(*(int *)(param_1 + 0x4c) + 0x30) - (float)fVar4) {
          *(float *)(*(int *)(param_1 + 0x4c) + 0x30) =
               *(float *)(*(int *)(param_1 + 0x4c) + 0x30) - (float)fVar4;
        }
      }
    }
    else if (*(int *)(param_1 + 0x26e24) == 5) {
      FUN_0047fa1d();
    }
  }
  return;
}

