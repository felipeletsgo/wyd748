// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044a9e9 | Name: FUN_0044a9e9


void __thiscall FUN_0044a9e9(int param_1,int param_2,int param_3)

{
  undefined1 local_128 [128];
  undefined1 local_a8 [32];
  int *local_88;
  undefined1 local_84 [64];
  undefined1 local_44 [32];
  undefined1 local_24 [32];
  
  if (*(int *)(param_1 + 0x26e3c) == 1) {
    FUN_0047fcd7(0);
  }
  local_88 = *(int **)(param_1 + 0x28c10);
  if (local_88 != (int *)0x0) {
    FUN_0058ee20(local_24,&DAT_00a40078);
    FUN_0058ee20(local_44,&DAT_00a400f8);
    FUN_0058ee20(local_a8,&DAT_00a40178);
    if (*(int *)(param_1 + 0x28c1c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x28c1c) + 0x80))(local_24,0);
    }
    if (*(int *)(param_1 + 0x28c20) != 0) {
      (**(code **)(**(int **)(param_1 + 0x28c20) + 0x80))(local_44,0);
    }
    if (*(int *)(param_1 + 0x28c24) != 0) {
      (**(code **)(**(int **)(param_1 + 0x28c24) + 0x80))(local_a8,0);
    }
    (**(code **)(*local_88 + 0x60))(param_2);
    FUN_0058f078(local_84,s_UI_PotalPos_txt_005b7d18);
    if (((param_2 == 1) &&
        (FUN_0049e50f(*(undefined4 *)(param_1 + 0x28c14),local_84,param_3 * 10,
                      (param_3 + 1) * 10 + -1), *(int *)(param_1 + 0x27c84) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x27c84) + 0x28) == 1)) {
      FUN_0044c53f();
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x27c7c) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x27c7c) + 0x28) == 1)) {
      FUN_00447c73();
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x285d8) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x285d8) + 0x28) == 1)) {
      FUN_004484f3(0);
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x27b38) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x27b38) + 0x28) == 1)) {
      FUN_0044ae38(0,0);
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x27b34) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x27b34) + 0x28) == 1)) {
      FUN_00447691();
    }
    if (((param_2 == 1) && (*(int *)(param_1 + 0x285d4) != 0)) &&
       (*(int *)(*(int *)(param_1 + 0x285d4) + 0x28) == 1)) {
      FUN_004481c5(0);
    }
    if (param_2 == 0) {
      _memset((void *)(param_1 + 0x27178),0,0x24);
    }
    else {
      FUN_0058f078(local_128,&DAT_005b7d28,&DAT_00a3dc78);
      FUN_0049e50f(*(undefined4 *)(param_1 + 0x28c14),local_84,param_3 * 10,(param_3 + 1) * 10 + -1)
      ;
      switch(param_3) {
      case 0:
        FUN_0058f078(local_128,&DAT_005b7d2c,&DAT_00a3dc78);
        break;
      case 1:
        FUN_0058f078(local_128,&DAT_005b7d30,&DAT_00a3dcf8);
        break;
      case 2:
        FUN_0058f078(local_128,&DAT_005b7d34,&DAT_00a3dd78);
        break;
      case 3:
        FUN_0058f078(local_128,&DAT_005b7d38,&DAT_00a3ddf8);
        break;
      case 4:
        FUN_0058f078(local_128,&DAT_005b7d3c,&DAT_00a3de78);
        break;
      case 5:
        FUN_0058f078(local_128,&DAT_005b7d40,&DAT_00a3def8);
      }
      (**(code **)(**(int **)(param_1 + 0x28c18) + 0x80))(local_128,0);
      (**(code **)(**(int **)(param_1 + 0x28c18) + 0x84))(0xffffffff);
    }
  }
  return;
}

