// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00487b92 | Name: FUN_00487b92


undefined4 __thiscall FUN_00487b92(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  void *_Dst;
  int iVar3;
  void *pvVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_4c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fb3d;
  local_10 = ExceptionList;
  piVar1 = *(int **)(param_1 + 0x28730);
  piVar2 = *(int **)(param_1 + 0x2873c);
  if (((short)piVar1[0x101] == *(short *)(param_2 + 0xc)) &&
     (ExceptionList = &local_10, _Dst = operator_new(8), _Dst != (void *)0x0)) {
    _memset(_Dst,0,8);
    iVar3 = (**(code **)(*piVar1 + 0xb4))
                      ((int)*(short *)(param_2 + 0xe) % 9,(int)*(short *)(param_2 + 0xe) / 9);
    FUN_0058f220(_Dst,*(undefined4 *)(iVar3 + 0x670),8);
    iVar3 = DAT_013b71e8;
    *(undefined4 *)(DAT_013b71e8 + 0x704) = *(undefined4 *)(param_2 + 0x14);
    pvVar4 = operator_new(0x678);
    local_8 = 0;
    if (pvVar4 == (void *)0x0) {
      local_4c = 0;
    }
    else {
      local_4c = FUN_0040d13e(0,_Dst,0,0);
    }
    local_8 = 0xffffffff;
    if (local_4c != 0) {
      (**(code **)(*piVar2 + 0x8c))
                (local_4c,(int)*(short *)(param_2 + 0x10) % 9,(int)*(short *)(param_2 + 0x10) / 9);
      FUN_0058f220(iVar3 + 0x7c8 + *(short *)(param_2 + 0x10) * 8,_Dst,8);
      iVar3 = FUN_0054cd07(_Dst,0x3d);
      if (0 < iVar3) {
        FUN_0054e9ab(*(undefined4 *)(local_4c + 0x670),iVar3);
        FUN_0058f078(local_4c + 0x39a,&PTR_DAT_005b88a8,iVar3);
        FUN_00421b8f(local_4c + 0x39a,*(undefined4 *)(local_4c + 900),0);
      }
      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x1f), iVar3 != 0)) {
        uVar6 = 0;
        uVar5 = 0;
        FUN_00429a6d(0x1f);
        FUN_0042ad2b(uVar5,uVar6);
      }
    }
    FUN_004431e4(0);
  }
  ExceptionList = local_10;
  return 1;
}

