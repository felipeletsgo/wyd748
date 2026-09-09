// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d5d42 | Name: FUN_004d5d42


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d5d42(int *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (((((DAT_0092e660 != 1) || (param_1[0x7c] == 3)) || (param_1[0x7c] == 4)) ||
      ((param_1[0x7c] == 5 || (param_1[0x7c] == 6)))) &&
     ((param_1[0x18] != 0 && (iVar1 = (**(code **)(*param_1 + 0x4c))(), iVar1 == 1)))) {
    iVar1 = param_1[0x18];
    *(int *)(iVar1 + 0x288) = param_1[0x71];
    *(int *)(iVar1 + 0x28c) = param_1[0x72];
    *(int *)(iVar1 + 0x290) = param_1[0x73];
    *(int *)(iVar1 + 0x294) = param_1[0x74];
    *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
    *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    if (param_1[0x66] == 1) {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3e99999a;
      *(undefined4 *)(param_1[0x18] + 700) = 0x3e99999a;
      *(undefined4 *)(param_1[0x18] + 0x2c0) = 0x3e99999a;
    }
    else if (param_1[0x66] == 2) {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      *(float *)(param_1[0x18] + 0x2b8) = _DAT_005a4160 * *(float *)(param_1[0x18] + 0x288);
      *(float *)(param_1[0x18] + 700) = _DAT_005a4160 * *(float *)(param_1[0x18] + 0x28c);
      *(float *)(param_1[0x18] + 0x2c0) = _DAT_005a4160 * *(float *)(param_1[0x18] + 0x290);
    }
    else if (param_1[0x66] == 0) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      *(undefined4 *)(param_1[0x18] + 0x2b8) = 0;
      *(undefined4 *)(param_1[0x18] + 700) = 0;
      *(undefined4 *)(param_1[0x18] + 0x2c0) = 0;
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,9);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      *(undefined4 *)(param_1[0x18] + 0x2b8) = 0;
      *(undefined4 *)(param_1[0x18] + 700) = 0;
      *(undefined4 *)(param_1[0x18] + 0x2c0) = 0;
    }
    if (param_1[0x87] != 0) {
      FUN_0058f220(param_1[0x87] + 0x288,param_1[0x18] + 0x288,0x44);
    }
    FUN_004be1ef(0,0x3f800000,0);
    if (param_1[0x87] != 0) {
      FUN_00430f50(_DAT_005a4adc * (float)param_1[0x89],_DAT_005a3660 / (float)param_1[0x89],
                   _DAT_005a4158 * (float)param_1[0x89]);
      puVar2 = (undefined4 *)(param_1[0x18] + 0x74);
      puVar3 = (undefined4 *)(param_1[0x87] + 0x34);
      for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      FUN_004be1ef(local_10,local_c,local_8);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
  }
  return 1;
}

