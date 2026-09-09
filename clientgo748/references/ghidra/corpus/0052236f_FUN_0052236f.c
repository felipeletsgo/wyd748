// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052236f | Name: FUN_0052236f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0052236f(int param_1,undefined4 param_2,int param_3)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int local_40;
  int local_3c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a214e;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x214) == 0) {
    ExceptionList = &local_10;
    if (((*(float *)(param_1 + 100) == _DAT_005a3534) &&
        (ExceptionList = &local_10, *(float *)(param_1 + 0x68) == _DAT_005a3534)) &&
       (ExceptionList = &local_10, *(float *)(param_1 + 0x6c) == _DAT_005a3534)) {
      ExceptionList = &local_10;
      *(undefined4 *)(param_1 + 100) = *(undefined4 *)(param_1 + 0x28);
      *(float *)(param_1 + 0x68) = *(float *)(param_1 + 0x30) + _DAT_005a430c;
      *(undefined4 *)(param_1 + 0x6c) = *(undefined4 *)(param_1 + 0x2c);
    }
    if (param_3 == 0) {
      if (((*(int *)(param_1 + 0x5c) == 0x14) &&
          ((*(short *)(param_1 + 0x1d6) == 0 || (*(short *)(param_1 + 0x1d6) == 2)))) ||
         (*(int *)(param_1 + 0x24c) == 0x23)) {
        pvVar1 = operator_new(0x5c);
        local_8 = 0;
        if (pvVar1 == (void *)0x0) {
          local_3c = 0;
        }
        else {
          local_3c = FUN_004dc960(*(undefined4 *)(param_1 + 100),*(undefined4 *)(param_1 + 0x68),
                                  *(undefined4 *)(param_1 + 0x6c),param_2,0x2a);
        }
        local_8 = 0xffffffff;
        if (local_3c != 0) {
          *(undefined4 *)(param_1 + 0x564) = *(undefined4 *)(local_3c + 0x48);
          *(undefined4 *)(param_1 + 0x568) = *(undefined4 *)(local_3c + 0x4c);
          FUN_0054ac09(local_3c);
        }
      }
    }
    else if (param_3 == 1) {
      pvVar1 = operator_new(0xdc);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_40 = 0;
      }
      else {
        uVar7 = 0;
        uVar6 = 5;
        uVar5 = 0x3f800000;
        uVar4 = 0xff551100;
        uVar3 = 0xffff3300;
        puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
        local_40 = FUN_004d8020(*(undefined4 *)(param_1 + 100),*(undefined4 *)(param_1 + 0x68),
                                *(undefined4 *)(param_1 + 0x6c),param_2,*puVar2,puVar2[1],puVar2[2],
                                uVar3,uVar4,uVar5,uVar6,uVar7);
      }
      local_8 = 0xffffffff;
      if ((local_40 != 0) && (*(undefined4 *)(local_40 + 0x84) = 0x3f000000, local_40 != 0)) {
        FUN_0054ac09(local_40);
      }
    }
  }
  ExceptionList = local_10;
  return;
}

