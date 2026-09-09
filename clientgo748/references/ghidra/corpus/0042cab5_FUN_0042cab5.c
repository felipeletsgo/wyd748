// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042cab5 | Name: FUN_0042cab5


void __fastcall FUN_0042cab5(int param_1)

{
  HDC hdc;
  int local_60c;
  undefined2 local_608 [256];
  undefined2 auStack_408 [256];
  undefined2 auStack_208 [256];
  int local_8;
  
  DAT_005b491c = 0x32;
  _memset(local_608,0,0x600);
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x4c))(*(undefined4 *)(param_1 + 0x2a3e4),local_608);
  for (local_8 = 0; local_8 < 0x100; local_8 = local_8 + 1) {
    local_60c = __ftol();
    if (0xffff < local_60c) {
      local_60c = 0xffff;
    }
    local_608[local_8] = (undefined2)local_60c;
    auStack_408[local_8] = (undefined2)local_60c;
    auStack_208[local_8] = (undefined2)local_60c;
  }
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x48))
            (*(undefined4 *)(param_1 + 0x2a3e4),0,local_608);
  if (*(int *)(param_1 + 0x2a3c0) != 0) {
    hdc = GetDC(*(HWND *)(DAT_013b71e0 + 4));
    SetDeviceGammaRamp(hdc,local_608);
    ReleaseDC(*(HWND *)(DAT_013b71e0 + 4),hdc);
  }
  if (*(int *)(param_1 + 0x2a70c) != 0) {
    if (*(undefined4 **)(param_1 + 0x2a70c) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x2a70c))(1);
    }
    *(undefined4 *)(param_1 + 0x2a70c) = 0;
  }
  if (*(int *)(param_1 + 0x2a710) != 0) {
    if (*(undefined4 **)(param_1 + 0x2a710) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x2a710))(1);
    }
    *(undefined4 *)(param_1 + 0x2a710) = 0;
  }
  if (*(int *)(param_1 + 0x2a704) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2a704) + 8))(*(undefined4 *)(param_1 + 0x2a704));
    *(undefined4 *)(param_1 + 0x2a704) = 0;
  }
  FUN_004280d6();
  if (*(int *)(param_1 + 0x2a708) != 0) {
    if (*(undefined4 **)(param_1 + 0x2a708) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x2a708))(1);
    }
    *(undefined4 *)(param_1 + 0x2a708) = 0;
  }
  FUN_0058ea3a(DAT_005ccf78);
  DAT_005ccf78 = 0;
  return;
}

