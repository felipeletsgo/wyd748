// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489618 | Name: FUN_00489618


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00489618(int param_1)

{
  float fVar1;
  int iVar2;
  void *pvVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int local_50;
  undefined4 local_2c;
  undefined4 local_28;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fbb0;
  local_10 = ExceptionList;
  if ((*(short *)(param_1 + 0xc) <= *(short *)(param_1 + 0x10)) &&
     (*(short *)(param_1 + 0xe) <= *(short *)(param_1 + 0x12))) {
    ExceptionList = &local_10;
    for (local_18 = (int)*(short *)(param_1 + 0xe); local_18 < *(short *)(param_1 + 0x12);
        local_18 = local_18 + 3) {
      for (local_1c = *(short *)(param_1 + 0xc) + 1; local_1c < *(short *)(param_1 + 0x10);
          local_1c = local_1c + 4) {
        FUN_00430f20((float)local_1c,(float)local_18 + _DAT_005a3660);
        iVar2 = FUN_0049c70f(local_2c,local_28);
        fVar1 = (float)iVar2 * _DAT_005a4214;
        if (*(short *)(param_1 + 0x14) == 0x20) {
          pvVar3 = operator_new(0x9c);
          local_8 = 0;
          if (pvVar3 == (void *)0x0) {
            local_50 = 0;
          }
          else {
            uVar8 = 0x44444444;
            uVar7 = 0xd2;
            uVar6 = 0x3fc00000;
            uVar5 = 0;
            puVar4 = (undefined4 *)FUN_00430f50(local_2c,fVar1,local_28);
            local_50 = FUN_004e2360(*puVar4,puVar4[1],puVar4[2],uVar5,uVar6,uVar7,uVar8);
          }
          local_8 = 0xffffffff;
          if (local_50 != 0) {
            FUN_0054ac09(local_50);
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

