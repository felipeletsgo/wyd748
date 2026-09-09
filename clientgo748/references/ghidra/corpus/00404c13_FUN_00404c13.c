// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00404c13 | Name: FUN_00404c13


void __thiscall FUN_00404c13(int param_1,char *param_2,undefined4 param_3,int param_4)

{
  size_t sVar1;
  LPSTR pCVar2;
  char local_84 [128];
  
  *(undefined4 *)(param_1 + 0x1ec) = param_3;
  sVar1 = _strlen(param_2);
  if ((sVar1 < 0x23) || (param_4 != 0)) {
    (**(code **)(**(int **)(param_1 + 0x1fc) + 0x80))(param_2,0);
    (**(code **)(**(int **)(param_1 + 0x200) + 0x80))(param_4,0);
  }
  else {
    pCVar2 = CharPrevA(param_2,param_2 + 0x22);
    _memset(local_84,0,0x80);
    _strncpy(local_84,param_2,(int)pCVar2 - (int)param_2);
    (**(code **)(**(int **)(param_1 + 0x1fc) + 0x80))(local_84,0);
    (**(code **)(**(int **)(param_1 + 0x200) + 0x80))(param_2 + ((int)pCVar2 - (int)param_2),0);
  }
  return;
}

