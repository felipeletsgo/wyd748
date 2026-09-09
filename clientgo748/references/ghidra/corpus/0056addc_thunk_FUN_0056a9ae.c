// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056addc | Name: thunk_FUN_0056a9ae


undefined4 __fastcall thunk_FUN_0056a9ae(int *param_1)

{
  int *piVar1;
  
  piVar1 = (int *)*param_1;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x28))(piVar1);
  }
  *param_1 = 0;
  return 0;
}

