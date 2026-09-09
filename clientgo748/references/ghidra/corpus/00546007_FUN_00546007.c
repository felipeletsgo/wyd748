// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00546007 | Name: FUN_00546007


undefined4 __fastcall FUN_00546007(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 local_84 [64];
  undefined1 local_44 [64];
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else {
    iVar2 = (**(code **)(*param_1 + 0x4c))();
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      if (param_1[0x5f] != 0) {
        if (param_1[0x73] == 1) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
        }
        iVar2 = __ftol();
        if (((iVar2 >> 7 == 0x1c) || (iVar2 = __ftol(), iVar2 >> 7 == 0x1d)) &&
           ((iVar2 = __ftol(), iVar2 >> 7 == 0x17 || (iVar2 = __ftol(), iVar2 >> 7 == 0x16)))) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
          param_1[100] = 0x3f666666;
          param_1[0x65] = 0x3f666666;
          param_1[0x66] = 0x3f666666;
          param_1[0x60] = 0x3f666666;
          param_1[0x61] = 0x3f666666;
          param_1[0x62] = 0x3f666666;
          param_1[0x68] = param_1[0x60];
          param_1[0x69] = param_1[0x61];
          param_1[0x6a] = param_1[0x62];
          param_1[0x6b] = param_1[99];
          param_1[0x6c] = 0x3f666666;
          param_1[0x6d] = 0x3f666666;
          param_1[0x6e] = 0x3f666666;
          param_1[0x6f] = 0x3f666666;
          param_1[0x67] = 0x3f666666;
          param_1[99] = 0x3f666666;
          (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],param_1 + 0x60);
          *(undefined4 *)(param_1[0x5f] + 4) = 1;
          *(undefined4 *)(param_1[0x5f] + 0x298) = 0x196;
          uVar1 = FUN_004b925c(0x196,360000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
          if (DAT_005ccf9c[0xa8e4] == 1) {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,7);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x13,4);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,3);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
          }
        }
        else if (((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) {
          param_1[100] = 0x3f000000;
          param_1[0x65] = 0x3f000000;
          param_1[0x66] = 0x3f000000;
          param_1[0x60] = 0x3f000000;
          param_1[0x61] = 0x3f000000;
          param_1[0x62] = 0x3f000000;
          param_1[0x68] = param_1[0x60];
          param_1[0x69] = param_1[0x61];
          param_1[0x6a] = param_1[0x62];
          param_1[0x6b] = param_1[99];
          param_1[0x6c] = 0x3dcccccd;
          param_1[0x6d] = 0x3dcccccd;
          param_1[0x6e] = 0x3dcccccd;
          param_1[0x6f] = 0x3dcccccd;
          param_1[0x67] = 0x3f666666;
          param_1[99] = 0x3f666666;
          (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],param_1 + 0x60);
          *(undefined4 *)(param_1[0x5f] + 0x298) = 2;
          uVar1 = FUN_004b9a4b(3,180000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
          if (DAT_005ccf9c[0xa8e4] == 1) {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,7);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,7);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,5);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
          }
        }
        else {
          param_1[100] = 0x3f000000;
          param_1[0x65] = 0x3f000000;
          param_1[0x66] = 0x3f000000;
          param_1[0x60] = 0x3f000000;
          param_1[0x61] = 0x3f000000;
          param_1[0x62] = 0x3f000000;
          param_1[0x68] = param_1[0x60];
          param_1[0x69] = param_1[0x61];
          param_1[0x6a] = param_1[0x62];
          param_1[0x6b] = param_1[99];
          param_1[0x6c] = 0;
          param_1[0x6d] = 0;
          param_1[0x6e] = 0;
          param_1[0x6f] = 0;
          param_1[0x67] = 0;
          param_1[99] = 0x3f000000;
          (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],param_1 + 0x60);
          if (DAT_005ccfa0 == 2) {
            *(undefined4 *)(param_1[0x5f] + 0x298) = 8;
          }
          else {
            *(undefined4 *)(param_1[0x5f] + 0x298) = 8;
          }
          uVar1 = FUN_004b9a4b(9,180000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
          (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
          (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
        }
        FUN_00430f10();
        FUN_00430f10();
        thunk_FUN_00562821(local_44,0,0,0);
        thunk_FUN_0056239d(local_84,param_1[10],param_1[0xc],param_1[0xb]);
        thunk_FUN_00561a5b(local_44,DAT_005ccf9c + 0xa99b,local_44);
        thunk_FUN_00561a5b(local_84,local_44,local_84);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_84);
        (**(code **)(*(int *)param_1[0x5f] + 0xc))(0,0);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,4,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
      uVar1 = 1;
    }
  }
  return uVar1;
}

