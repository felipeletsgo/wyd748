// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00427aa8 | Name: FUN_00427aa8


void __fastcall FUN_00427aa8(int param_1)

{
  int iVar1;
  int local_110;
  char local_10c [128];
  int local_8c;
  int *local_88;
  undefined1 local_84 [128];
  
  local_110 = 0;
  FUN_0058f078(local_84,s_ScreenShot_Capture_04d_jpg_005b28f8,0);
  FUN_0058f078(local_10c,s_ScreenShot_Capture_04d_bmp_005b2914,0);
  local_8c = -1;
  do {
    local_8c = FUN_0058fb07(local_84,0x8000,0);
    if (local_8c < 0) break;
    FUN_0058f75f(local_8c);
    local_110 = local_110 + 1;
    FUN_0058f078(local_84,s_ScreenShot_Capture_04d_jpg_005b2930,local_110);
    FUN_0058f078(local_10c,s_ScreenShot_Capture_04d_bmp_005b294c,local_110);
  } while (local_110 < 10000);
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x40))
            (*(undefined4 *)(param_1 + 0x2a3e4),0,0,&local_88);
  iVar1 = FUN_0055fe5b(local_10c,0,local_88,0,0);
  if (iVar1 < 0) {
    FUN_00431159(s_Can_t_Capture_Screen_005b2968);
  }
  (**(code **)(*local_88 + 8))(local_88);
  BMP2JPG(local_10c);
  return;
}

