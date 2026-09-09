// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00529b07 | Name: FUN_00529b07


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00529b07(int param_1,int param_2)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int local_34;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a22de;
  local_10 = ExceptionList;
  if (param_2 != 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x1eac);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_34 = 0;
    }
    else {
      uVar3 = 6;
      puVar2 = (undefined4 *)
               FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                            *(float *)(param_1 + 0x30) + _DAT_005a4104,
                            *(undefined4 *)(param_1 + 0x2c));
      local_34 = FUN_004d01b0(*puVar2,puVar2[1],puVar2[2],uVar3);
    }
    local_8 = 0xffffffff;
    if (local_34 != 0) {
      FUN_004d0651(param_2 + 0x14);
      FUN_0054ac09(local_34);
    }
  }
  ExceptionList = local_10;
  return 1;
}

