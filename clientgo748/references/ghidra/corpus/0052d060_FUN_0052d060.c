// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052d060 | Name: FUN_0052d060


undefined4 __thiscall FUN_0052d060(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  void *pvVar4;
  bool bVar5;
  int local_bc;
  uint local_ac;
  undefined1 local_98 [128];
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a23f1;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined1 *)(param_2 + 0x6b) = 0;
  *(undefined1 *)(param_2 + 0x6a) = 0;
  iVar1 = DAT_0067cf38;
  if ((DAT_005b892c == 2) && (*(int *)(DAT_0067cf38 + 0x27af4) == 0)) {
    uVar2 = 0;
  }
  else {
    iVar3 = FUN_00494dcf();
    if ((iVar3 == 30000) &&
       ((DAT_005b892c == 2 && (*(int *)(*(int *)(iVar1 + 0x27af4) + 0x1e8) == 0)))) {
      uVar2 = 1;
    }
    else {
      iVar3 = *(int *)(iVar1 + 0x27ae8);
      local_14 = 0xffffffff;
      local_18 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 6));
      bVar5 = *(char *)(param_2 + 0xc) != '*';
      if (bVar5) {
        *(undefined4 *)(param_1 + 0x61c) = 3000;
      }
      else {
        *(undefined4 *)(param_1 + 0x61c) = 10000;
      }
      local_ac = (uint)!bVar5;
      FUN_0058f078(local_98,s___s_>__s_005bb850,param_1 + 0x168,param_2 + 0xc + local_ac);
      pvVar4 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar4 == (void *)0x0) {
        local_bc = 0;
      }
      else {
        local_bc = FUN_00407203(local_98,local_14,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      if ((local_bc != 0) && (iVar3 != 0)) {
        FUN_00408d33(local_bc);
      }
      uVar2 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(iVar1 + 0x27464) = uVar2;
      if ((*(char *)(param_2 + 0xc) != '=') && (*(char *)(param_2 + 0xc) != '-')) {
        FUN_00504748(param_2 + 0xc + local_ac);
      }
      uVar2 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}

