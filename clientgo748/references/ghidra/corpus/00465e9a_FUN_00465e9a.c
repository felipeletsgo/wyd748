// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00465e9a | Name: FUN_00465e9a


void __fastcall FUN_00465e9a(int param_1)

{
  DWORD DVar1;
  int iVar2;
  undefined1 local_88 [128];
  int local_8;
  
  if (*(int *)(param_1 + 0x28e50) != 0) {
    DVar1 = timeGetTime();
    local_8 = 900 - (DVar1 - *(int *)(param_1 + 0x28e50)) / 1000;
    FUN_0058f078(local_88,s__Quest_Timer____02d__02d__005b7dc0,local_8 / 0x3c,local_8 % 0x3c);
    (**(code **)(**(int **)(param_1 + 0x28e4c) + 0x80))(local_88,0);
    iVar2 = FUN_0040c0f0();
    if (iVar2 == 0) {
      (**(code **)(**(int **)(param_1 + 0x28e4c) + 0x60))(1);
    }
  }
  return;
}

