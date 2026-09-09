// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00427c0c | Name: FUN_00427c0c


int __fastcall FUN_00427c0c(int *param_1)

{
  int *local_c;
  int local_8;
  
  local_8 = (**(code **)(*param_1 + 0x14))();
  if ((-1 < local_8) &&
     (local_8 = (**(code **)(*(int *)param_1[0xa8f9] + 0x38))(param_1[0xa8f9],param_1 + 0xa8e9),
     -1 < local_8)) {
    (**(code **)(*(int *)param_1[0xa8f9] + 0x40))(param_1[0xa8f9],0,0,&local_c);
    (**(code **)(*local_c + 0x20))(local_c,param_1 + 0xa92f);
    (**(code **)(*local_c + 8))(local_c);
    if (param_1[0xa8e1] == 0) {
      GetClassLongA((HWND)param_1[0xa8f6],-0xc);
      (**(code **)(*(int *)param_1[0xa8f9] + 0x30))(param_1[0xa8f9],1);
    }
    local_8 = (**(code **)(*param_1 + 0x10))();
    if (-1 < local_8) {
      local_8 = 0;
    }
  }
  return local_8;
}

