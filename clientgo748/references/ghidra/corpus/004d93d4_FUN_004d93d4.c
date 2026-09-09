// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d93d4 | Name: FUN_004d93d4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d93d4(int *param_1)

{
  byte bVar1;
  int iVar2;
  float10 fVar3;
  float local_6c;
  float local_48;
  int local_2c;
  undefined1 local_28 [16];
  uint local_18;
  int local_10;
  float local_c;
  int local_8;
  
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  if (param_1[0x21] == 0) {
    param_1[0x23] = (int)((float)((uint)(iVar2 - param_1[0x20]) % 2000) / _DAT_005a441c);
  }
  else {
    param_1[0x23] = (int)((float)(uint)(iVar2 - param_1[0x20]) / (float)param_1[0x21]);
  }
  if (_DAT_005a3660 < (float)param_1[0x23]) {
    if (param_1[0x21] != 0) {
      (**(code **)(**(int **)(DAT_013b71e0 + 0xf8) + 100))(param_1);
    }
  }
  else {
    local_8 = FUN_004b14fd(0x2bf,1,180000);
    if (local_8 != 0) {
      (**(code **)(**(int **)(local_8 + 0x318) + 0x34))(*(undefined4 *)(local_8 + 0x318),local_28);
      (**(code **)(**(int **)(local_8 + 0x318) + 0x2c))
                (*(undefined4 *)(local_8 + 0x318),0,0,&local_10,0);
      fVar3 = (float10)FUN_00423b00((float)param_1[0x23] * _DAT_005a4148);
      if (fVar3 <= (float10)_DAT_005a3534) {
        fVar3 = (float10)FUN_00423b00((float)param_1[0x23] * _DAT_005a4148);
        fVar3 = -fVar3;
      }
      else {
        fVar3 = (float10)FUN_00423b00((float)param_1[0x23] * _DAT_005a4148);
      }
      local_6c = (float)fVar3;
      local_c = local_6c;
      for (local_2c = 0; local_2c < (int)(local_18 / 0x18); local_2c = local_2c + 1) {
        bVar1 = __ftol();
        *(uint *)(local_10 + 0xc + local_2c * 0x18) =
             (uint)bVar1 << 0x10 | (uint)bVar1 << 8 | (uint)bVar1;
      }
      (**(code **)(**(int **)(local_8 + 0x318) + 0x30))(*(undefined4 *)(local_8 + 0x318));
      if (param_1[0x24] == 1) {
        *(float *)(local_8 + 0x330) = (float)param_1[0x23] * _DAT_005a4160 + _DAT_005a4158;
        *(undefined4 *)(local_8 + 0x32c) = *(undefined4 *)(local_8 + 0x330);
      }
      else if (param_1[0x24] == 4) {
        local_48 = 0.3;
        if (((param_1[7] != 0) && (*(int *)(param_1[7] + 0x24) == 3)) &&
           (*(char *)(param_1[7] + 0x79c) == '\x01')) {
          local_48 = 0.7;
        }
        *(float *)(local_8 + 0x330) = local_c * local_48 + _DAT_005a4160;
        *(undefined4 *)(local_8 + 0x32c) = *(undefined4 *)(local_8 + 0x330);
      }
      else if (param_1[0x24] == 5) {
        *(float *)(local_8 + 0x32c) = local_c * _DAT_005a34a0 + _DAT_005a40ec;
        *(float *)(local_8 + 0x330) = local_c * _DAT_005a40f4 + _DAT_005a34a0;
      }
      else {
        *(float *)(local_8 + 0x330) = local_c * _DAT_005a34a0 + _DAT_005a34a0;
        *(undefined4 *)(local_8 + 0x32c) = *(undefined4 *)(local_8 + 0x330);
      }
    }
    param_1[0x22] = (int)((float)param_1[0x23] * _DAT_005a4148);
    if (param_1[7] != 0) {
      param_1[0x1d] = *(int *)(param_1[7] + 0x28);
      param_1[0x1e] = *(int *)(param_1[7] + 0x30);
      param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
    }
  }
  return 1;
}

