// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042c51b | Name: FUN_0042c51b


undefined4 __fastcall FUN_0042c51b(int param_1)

{
  void *pvVar1;
  int iVar2;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f091;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x2a710) == 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x69a68);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_20 = 0;
    }
    else {
      local_20 = FUN_004afac0();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x2a710) = local_20;
    if (*(int *)(param_1 + 0x2a710) != 0) {
      iVar2 = FUN_004afbc6();
      if (iVar2 == 0) {
        FUN_00431159(s_Initialize_Mesh_Manager_Failed_005b492c);
        ExceptionList = local_10;
        return 0;
      }
      (**(code **)(**(int **)(param_1 + 0x2a710) + 8))();
      *(undefined4 *)(param_1 + 0x2b04c) = 1;
      ExceptionList = local_10;
      return 1;
    }
  }
  ExceptionList = local_10;
  return 1;
}

