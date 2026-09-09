// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055d345 | Name: FUN_0055d345


WPARAM __fastcall FUN_0055d345(int *param_1)

{
  int iVar1;
  ushort *puVar2;
  int local_30;
  uint local_2c;
  tagMSG local_28;
  HACCEL local_c;
  int local_8;
  
  local_c = LoadAcceleratorsA((HINSTANCE)0x0,(LPCSTR)0xb0);
  PeekMessageA(&local_28,(HWND)0x0,0,0,0);
  while (local_28.message != 0x12) {
    if (param_1[2] == 0) {
      local_8 = GetMessageA(&local_28,(HWND)0x0,0,0);
    }
    else {
      local_8 = PeekMessageA(&local_28,(HWND)0x0,0,0,1);
    }
    if (local_8 == 0) {
      if (param_1[2] != 0) {
        local_2c = (**(code **)(*(int *)param_1[0x3d] + 8))();
        FUN_004aec3d();
        (**(code **)(*(int *)param_1[0x3e] + 0x28))(local_2c);
        if ((DAT_013b71f8 != 0) && (DAT_013b71fc == 0)) {
          if (DAT_013b7204 < DAT_013b7208) {
            local_30 = 0;
            FUN_0058f220(&local_30,DAT_013b71f4 + DAT_013b7204,4);
            if ((uint)(local_30 + DAT_013b71f8) < local_2c) {
              DAT_013b7204 = DAT_013b7204 + 4;
              puVar2 = (ushort *)(DAT_013b71f4 + DAT_013b7204);
              (**(code **)(*(int *)param_1[0x3e] + 8))(puVar2[2],puVar2);
              DAT_013b7204 = DAT_013b7204 + (uint)*puVar2;
            }
          }
          else {
            DAT_013b71f8 = 0;
            FUN_00403df2(s_End_Replay_005c4f20,4000);
            (**(code **)(**(int **)(DAT_0067cf38 + 0x84) + 0x88))(1,1);
            *(undefined1 *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x795) = 0;
          }
        }
        if (DAT_013b736c == 1) {
          if (DAT_013b7374 == 0) {
            DAT_013b7374 = 1;
          }
          ShowWindow(DAT_013b7364,5);
          UpdateWindow(DAT_013b7364);
        }
        else if ((param_1[0x3a] == 0) ||
                ((param_1[0x3a] != 0 && (*(int *)(param_1[0x3a] + 0x10c) != 2)))) {
          (**(code **)(*param_1 + 0x14))();
        }
        if (((param_1[0x3a] == 0) ||
            ((param_1[0x3a] != 0 && (*(int *)(param_1[0x3a] + 0x10c) != 2)))) &&
           (*(int *)(param_1[0x38] + 0x2a710) == 0)) {
          iVar1 = FUN_0042c51b();
          if (iVar1 == 0) {
            MessageBoxA((HWND)param_1[1],s_Game_data_crashed__Install_game_a_005c4f34,
                        s_Error_005c4f2c,0x1000);
            PostMessageA((HWND)param_1[1],0x10,0,0);
          }
          if (param_1[0x3a] != 0) {
            if ((undefined4 *)param_1[0x3a] != (undefined4 *)0x0) {
              (*(code *)**(undefined4 **)param_1[0x3a])(1);
            }
            param_1[0x3a] = 0;
          }
          (**(code **)(*(int *)param_1[0x3e] + 0x54))(7);
        }
        if (*(int *)(param_1[0x3e] + 0x1b08c) != 0) {
          FUN_004b16c0();
        }
      }
    }
    else if (((local_c == (HACCEL)0x0) || (param_1[1] == 0)) ||
            (iVar1 = TranslateAcceleratorA((HWND)param_1[1],local_c,&local_28), iVar1 == 0)) {
      TranslateMessage(&local_28);
      DispatchMessageA(&local_28);
    }
  }
  if (local_c != (HACCEL)0x0) {
    DestroyAcceleratorTable(local_c);
  }
  return local_28.wParam;
}

