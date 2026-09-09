// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00425d36 | Name: FUN_00425d36


int __thiscall FUN_00425d36(int param_1,int param_2,int param_3)

{
  CHAR local_204 [512];
  
  if (param_2 < -0x7dfffff8) {
    if (param_2 == -0x7dfffff9) {
      FUN_0058ee20(local_204,s_Your_Direct3D_hardware_accelerat_005b23e8);
      goto LAB_00425f7b;
    }
    if (param_2 < -0x7dfffffb) {
      if (param_2 == -0x7dfffffc) {
        FUN_0058ee20(local_204,s_This_Program_cannot_run_in_a_des_005b22a0);
        goto LAB_00425f7b;
      }
      if (param_2 == -0x7ff8fff2) {
        FUN_0058ee20(local_204,s_Not_enough_memory__005b26b4);
        goto LAB_00425f7b;
      }
      if (param_2 == -0x7dffffff) {
        FUN_0058ee20(local_204,s_Could_not_initialize_Direct3D__Y_005b2188);
        goto LAB_00425f7b;
      }
      if (param_2 == -0x7dfffffd) {
        FUN_0058ee20(local_204,s_Could_not_find_any_compatible_Di_005b2270);
        goto LAB_00425f7b;
      }
    }
    else {
      if (param_2 == -0x7dfffffb) {
        FUN_0058ee20(local_204,s_No_hardware_accelerated_Direct3D_005b234c);
        goto LAB_00425f7b;
      }
      if (param_2 == -0x7dfffffa) {
        FUN_0058ee20(local_204,s_This_Program_requires_functional_005b2384);
        goto LAB_00425f7b;
      }
    }
  }
  else if (param_2 < -0x7789fe83) {
    if (param_2 == -0x7789fe84) {
      FUN_0058ee20(local_204,s_Not_enough_video_memory__005b26c8);
      goto LAB_00425f7b;
    }
    switch(param_2) {
    case -0x7dfffff8:
      FUN_0058ee20(local_204,s_Your_Direct3D_hardware_accelerat_005b2478);
      break;
    case -0x7dfffff7:
      FUN_0058ee20(local_204,s_This_Program_requires_functional_005b2534);
      break;
    case -0x7dfffff6:
      FUN_0058ee20(local_204,s_A_D3D_object_has_a_non_zero_refe_005b2658);
      break;
    case -0x7dfffff5:
      FUN_0058ee20(local_204,s_Could_not_load_required_media__005b2610);
      break;
    case -0x7dfffff4:
      FUN_0058ee20(local_204,s_Could_not_reset_the_Direct3D_dev_005b2630);
      break;
    default:
      goto switchD_00425e25_default;
    }
    goto LAB_00425f7b;
  }
switchD_00425e25_default:
  FUN_0058ee20(local_204,s_Generic_application_error__Enabl_005b26e4);
LAB_00425f7b:
  if (param_3 == 1) {
    FUN_0058ee30(local_204,s_This_program_will_now_exit__005b2730);
    MessageBoxA((HWND)0x0,local_204,s_Error___005b2750,0x10);
    if (*(int *)(param_1 + 0x2a3d8) != 0) {
      SendMessageA(*(HWND *)(param_1 + 0x2a3d8),0x10,0,0);
    }
  }
  else {
    if (param_3 == 2) {
      FUN_0058ee30(local_204,s_Switching_to_the_reference_raste_005b2758);
    }
    MessageBoxA((HWND)0x0,local_204,s_Error___005b27dc,0x30);
  }
  return param_2;
}

