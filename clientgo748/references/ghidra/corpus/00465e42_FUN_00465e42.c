// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00465e42 | Name: FUN_00465e42


void __thiscall FUN_00465e42(int param_1,char param_2)

{
  DWORD DVar1;
  
  if (param_2 == '\x01') {
    DVar1 = timeGetTime();
    *(DWORD *)(param_1 + 0x28e50) = DVar1;
  }
  else {
    *(undefined4 *)(param_1 + 0x28e50) = 0;
  }
  (**(code **)(**(int **)(param_1 + 0x28e4c) + 0x60))(param_2);
  return;
}

