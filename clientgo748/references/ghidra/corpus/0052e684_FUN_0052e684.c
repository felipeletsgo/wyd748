// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052e684 | Name: FUN_0052e684


undefined4 __fastcall FUN_0052e684(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x269,param_1);
  *(undefined4 *)(iVar1 + 0x1e8) = 1;
  return 1;
}

