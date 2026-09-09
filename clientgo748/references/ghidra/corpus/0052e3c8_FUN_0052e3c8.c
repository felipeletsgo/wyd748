// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052e3c8 | Name: FUN_0052e3c8


undefined4 __thiscall FUN_0052e3c8(int param_1,int param_2)

{
  int *piVar1;
  undefined4 *puVar2;
  void *_Dst;
  void *pvVar3;
  int iVar4;
  int local_54;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2460;
  local_10 = ExceptionList;
  if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
    ExceptionList = &local_10;
    piVar1 = (int *)(**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x151);
    for (local_1c = 0; local_1c < 0x3f; local_1c = local_1c + 1) {
      puVar2 = (undefined4 *)(**(code **)(*piVar1 + 0xa8))(local_1c % 9,local_1c / 9);
      if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
         (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar2)) {
        *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
      }
      if ((puVar2 != (undefined4 *)0x0) && (puVar2 != (undefined4 *)0x0)) {
        (**(code **)*puVar2)(1);
      }
    }
    (**(code **)(*piVar1 + 0xbc))();
    FUN_0058f220(DAT_013b71e8 + 0x7c8,param_2 + 0xc,0x200);
    for (local_1c = 0; local_1c < 0x3f; local_1c = local_1c + 1) {
      if ((0x28 < *(short *)(param_2 + 0xc + local_1c * 8)) &&
         (_Dst = operator_new(8), _Dst != (void *)0x0)) {
        _memset(_Dst,0,8);
        FUN_0058f220(_Dst,param_2 + 0xc + local_1c * 8,8);
        pvVar3 = operator_new(0x678);
        local_8 = 0;
        if (pvVar3 == (void *)0x0) {
          local_54 = 0;
        }
        else {
          local_54 = FUN_0040d13e(0,_Dst,0,0);
        }
        local_8 = 0xffffffff;
        if (local_54 != 0) {
          (**(code **)(*piVar1 + 0x8c))(local_54,local_1c % 9,local_1c / 9);
        }
      }
    }
    *(undefined4 *)(DAT_013b71e8 + 0x704) = *(undefined4 *)(param_2 + 0x20c);
    *(undefined2 *)(DAT_013b71e8 + 0xcfa) = 0;
    *(undefined1 *)(DAT_013b71e8 + 0xcf8) = 0;
    iVar4 = FUN_00494dcf();
    if (iVar4 == 30000) {
      FUN_0044b890(0);
      FUN_00447691();
    }
    FUN_004431e4(0);
  }
  ExceptionList = local_10;
  return 1;
}

