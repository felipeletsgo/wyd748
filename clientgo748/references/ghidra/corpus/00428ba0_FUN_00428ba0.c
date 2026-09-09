// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428ba0 | Name: FUN_00428ba0


void __thiscall FUN_00428ba0(undefined4 *param_1,int param_2)

{
  int iVar1;
  undefined1 local_28 [36];
  
  DAT_005b2980 = param_2;
  if ((-1 < param_2) && (param_2 < 0xf)) {
    FUN_00429480();
    if (param_2 == 0xd) {
      FUN_00428a12(&DAT_005b3688);
    }
    if (param_2 == 0xe) {
      FUN_00428a12(&DAT_005b3788);
    }
    iVar1 = FUN_005906a7(s_music_login_mp3_005b3888 + param_2 * 0x100,local_28);
    if (iVar1 == 0) {
      FUN_00429460();
    }
    else {
      FUN_00429460();
    }
    iVar1 = FUN_004294d0(0);
    if (iVar1 != -10000) {
      FUN_004294a0(0,*param_1);
    }
  }
  return;
}

