// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e94b6 | Name: FUN_004e94b6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e94b6(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  float10 fVar8;
  float local_ac;
  undefined4 local_38;
  undefined4 local_34;
  int local_2c;
  undefined1 local_28 [16];
  uint local_18;
  int local_10;
  float local_c;
  int local_8;
  
  if (param_1[0x27] != 0) {
    iVar1 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x29] = (int)((float)(uint)(iVar1 - param_1[0x25]) / (float)param_1[0x26]);
    if ((float)param_1[0x29] <= _DAT_005a3660) {
      if (param_1[7] != 0) {
        if (*(int *)(param_1[7] + 0x24) == 3) {
          local_8 = param_1[7];
          if (*(char *)(local_8 + 0x79c) == '\x01') {
            FUN_004310a0(local_8 + 0x50);
          }
          else {
            uVar2 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),
                                 *(undefined4 *)(param_1[7] + 0x30),
                                 *(undefined4 *)(param_1[7] + 0x2c));
            FUN_004310a0(uVar2);
          }
        }
        else {
          uVar2 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30)
                               ,*(undefined4 *)(param_1[7] + 0x2c));
          FUN_004310a0(uVar2);
        }
      }
      (**(code **)(*param_1 + 0x38))();
      if ((float)param_1[0x29] <= _DAT_005a3660) {
        fVar8 = (float10)FUN_00423b00((float)param_1[0x29] * _DAT_005a4148);
        if (fVar8 <= (float10)_DAT_005a3534) {
          fVar8 = (float10)FUN_00423b00((float)param_1[0x29] * _DAT_005a4148);
          fVar8 = -fVar8;
        }
        else {
          fVar8 = (float10)FUN_00423b00((float)param_1[0x29] * _DAT_005a4148);
        }
        local_ac = (float)fVar8;
        local_c = local_ac;
        if (param_1[0x2a] == 2) {
          param_1[0x1e] = (int)((float)param_1[0x1e] - _DAT_005a40ec);
        }
        if (param_1[0x2a] == 3) {
          param_1[0x1e] = (int)(local_ac + (float)param_1[0x1e]);
        }
        local_38 = 0x2c0;
        local_34 = 0x2c1;
        if (param_1[0x2a] == 4) {
          local_38 = 0x1f5;
          local_34 = 0x1f6;
          local_c = local_ac * _DAT_005a34a0;
        }
        iVar1 = FUN_004b14fd(local_38,1,180000);
        if (iVar1 == 0) {
          return 0;
        }
        if (*(int *)(iVar1 + 0x318) == 0) {
          return 0;
        }
        (**(code **)(**(int **)(iVar1 + 0x318) + 0x34))(*(undefined4 *)(iVar1 + 0x318),local_28);
        (**(code **)(**(int **)(iVar1 + 0x318) + 0x2c))
                  (*(undefined4 *)(iVar1 + 0x318),0,0,&local_10,0);
        uVar3 = local_18 / 0x18;
        for (local_2c = 0; local_2c < (int)uVar3; local_2c = local_2c + 1) {
          if (param_1[0x2a] == 4) {
            iVar4 = __ftol();
            iVar5 = __ftol();
            iVar6 = __ftol();
            uVar7 = __ftol();
            *(uint *)(local_10 + 0xc + local_2c * 0x18) =
                 iVar4 << 0x18 | iVar5 << 0x10 | iVar6 << 8 | uVar7;
          }
          else {
            __ftol();
            iVar4 = __ftol();
            iVar5 = __ftol();
            uVar7 = __ftol();
            *(uint *)(local_10 + 0xc + local_2c * 0x18) = iVar4 << 0x10 | iVar5 << 8 | uVar7;
          }
        }
        (**(code **)(**(int **)(iVar1 + 0x318) + 0x30))(*(undefined4 *)(iVar1 + 0x318));
        iVar1 = FUN_004b14fd(local_34,1,180000);
        if (iVar1 == 0) {
          return 0;
        }
        if (*(int *)(iVar1 + 0x318) == 0) {
          return 0;
        }
        (**(code **)(**(int **)(iVar1 + 0x318) + 0x34))(*(undefined4 *)(iVar1 + 0x318),local_28);
        (**(code **)(**(int **)(iVar1 + 0x318) + 0x2c))
                  (*(undefined4 *)(iVar1 + 0x318),0,0,&local_10,0);
        for (local_2c = 0; local_2c < (int)(local_18 / 0x18); local_2c = local_2c + 1) {
          if (param_1[0x2a] == 4) {
            iVar4 = __ftol();
            iVar5 = __ftol();
            iVar6 = __ftol();
            uVar3 = __ftol();
            *(uint *)(local_10 + 0xc + local_2c * 0x18) =
                 iVar4 << 0x18 | iVar5 << 0x10 | iVar6 << 8 | uVar3;
          }
          else {
            __ftol();
            iVar4 = __ftol();
            iVar5 = __ftol();
            uVar3 = __ftol();
            *(uint *)(local_10 + 0xc + local_2c * 0x18) = iVar4 << 0x10 | iVar5 << 8 | uVar3;
          }
        }
        (**(code **)(**(int **)(iVar1 + 0x318) + 0x30))(*(undefined4 *)(iVar1 + 0x318));
        param_1[0x28] =
             (int)((float)param_1[0x29] * _DAT_005a4148 + (float)param_1[0x29] * _DAT_005a4148);
      }
    }
    else {
      param_1[0x27] = 0;
      param_1[10] = 0;
    }
  }
  return 1;
}

