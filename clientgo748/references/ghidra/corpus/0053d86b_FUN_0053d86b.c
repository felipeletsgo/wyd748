// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053d86b | Name: FUN_0053d86b


void __fastcall FUN_0053d86b(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a269c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a59c0;
  local_8 = 0;
  (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0xb8))
            (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),param_1[0xc],0);
  local_8 = 0xffffffff;
  FUN_0054aa45();
  ExceptionList = local_10;
  return;
}

