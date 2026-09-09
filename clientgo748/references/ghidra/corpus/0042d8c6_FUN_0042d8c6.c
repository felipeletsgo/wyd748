// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042d8c6 | Name: FUN_0042d8c6


/* WARNING: Removing unreachable block (ram,0x0042db01) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0042d8c6(int param_1,int param_2)

{
  DWORD DVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_190 [128];
  uint local_110;
  int local_10c;
  undefined1 local_108 [256];
  float local_8;
  
  if ((param_2 == 1) && (*(int *)(param_1 + 0x2a708) != 0)) {
    DVar1 = timeGetTime();
    local_8 = (float)DVar1 * _DAT_005a41ec;
    DAT_005ccfa4 = DAT_005ccfa4 + 1;
    if (_DAT_005a3660 < local_8 - _DAT_005ccfa8) {
      *(float *)(param_1 + 0x2b040) = (float)DAT_005ccfa4 / (local_8 - _DAT_005ccfa8);
      DAT_005ccfa4 = 0;
      local_10c = DAT_013b7214 / 1000 - _DAT_013b7218;
      _DAT_005ccfa8 = local_8;
      local_110 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x10))();
      local_110 = local_110 >> 0x14;
      if (*(int *)(param_1 + 0x2b03c) == 1) {
        FUN_0058f078(local_108,s_FPS__4_3f_Eff__3d_MCnt__3d_UDC___005b49cc,
                     (double)*(float *)(param_1 + 0x2b040),DAT_0067cf40,DAT_0092eaf0,DAT_0092eaf4,
                     DAT_0092e7c8,local_110);
      }
      else {
        FUN_0058f078(local_108,s_FPS____4_3f_005b4a08,(double)*(float *)(param_1 + 0x2b040));
      }
      FUN_00421b8f(local_108,0xffffffaa,0);
    }
    if ((*(int *)(param_1 + 0x2b050) == 1) || (*(int *)(param_1 + 0x2b03c) == 1)) {
      uVar4 = 1;
      uVar2 = __ftol();
      FUN_0042261f(5,uVar2,uVar4);
    }
    if (*(int *)(param_1 + 0x2b04c) == 0) {
      FUN_0058f078(local_190,s_W_i_t_h_Y_o_u_r_D_e_s_t_i_n_y_____005b4a14,7,0x30);
      FUN_00421b8f(local_190,0xffffffaa,0);
      uVar4 = 1;
      uVar2 = __ftol();
      FUN_0042261f(5,uVar2,uVar4);
    }
  }
  iVar3 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x8c))();
  if (iVar3 < 0) {
    uVar2 = 0;
  }
  else {
    if (param_2 == 1) {
      (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x3c))();
    }
    uVar2 = 1;
  }
  return uVar2;
}

