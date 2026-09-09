// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428747 | Name: FUN_00428747


void __fastcall FUN_00428747(int param_1)

{
  int iVar1;
  undefined1 local_10 [4];
  undefined1 local_c [4];
  int local_8;
  
  iVar1 = (**(code **)(**(int **)(param_1 + 0x10) + 0x20))
                    (*(undefined4 *)(param_1 + 0x10),&local_8,local_10,local_c,0);
  if ((iVar1 == 0) && (local_8 == 1)) {
    FUN_004293c0();
    FUN_00429420(0,0);
    FUN_00429390();
  }
  return;
}

