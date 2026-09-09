// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d12e4 | Name: FUN_004d12e4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d12e4(int param_1)

{
  float fVar1;
  undefined4 uVar2;
  int local_d4;
  undefined1 local_d0 [64];
  float local_90;
  uint local_8c;
  float local_88;
  undefined1 local_84 [64];
  undefined1 local_44 [64];
  
  if ((*(int *)(param_1 + 0x24) == 6) || (DAT_0092e660 != 1)) {
    FUN_00430f10();
    FUN_00430f10();
    FUN_00430f10();
    local_90 = -*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c);
    local_88 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x28);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
    uVar2 = FUN_004b925c(7,360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar2);
    for (local_8c = 0; local_8c < 100; local_8c = local_8c + 1) {
      if (*(float *)(param_1 + 0xc0 + local_8c * 0x40) < _DAT_005a3660) {
        fVar1 = *(float *)(param_1 + 200 + local_8c * 0x40);
        thunk_FUN_0056230d(local_d0,fVar1,fVar1,fVar1);
        thunk_FUN_0056239d(local_44,*(undefined4 *)(param_1 + 0x8c + local_8c * 0x40),
                           *(undefined4 *)(param_1 + 0x90 + local_8c * 0x40),
                           *(undefined4 *)(param_1 + 0x94 + local_8c * 0x40));
        thunk_FUN_0056242d(local_84,local_90);
        thunk_FUN_00562821(local_84,_DAT_005a4380 - local_88,local_90,0);
        thunk_FUN_00561a5b(local_84,local_84,local_d0);
        thunk_FUN_00561a5b(local_84,local_84,local_44);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_84);
        for (local_d4 = 0; local_d4 < 4; local_d4 = local_d4 + 1) {
          *(undefined4 *)(param_1 + 0x38 + local_d4 * 0x18) = *(undefined4 *)(param_1 + 0x28);
        }
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                  (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0x2c,0x18);
        fVar1 = fVar1 * _DAT_005a4160;
        thunk_FUN_0056230d(local_d0,fVar1,fVar1,fVar1);
        thunk_FUN_0056239d(local_44,*(undefined4 *)(param_1 + 0x8c + local_8c * 0x40),
                           *(undefined4 *)(param_1 + 0x90 + local_8c * 0x40),
                           *(undefined4 *)(param_1 + 0x94 + local_8c * 0x40));
        thunk_FUN_0056242d(local_84,local_90);
        thunk_FUN_00562821(local_84,_DAT_005a4380 - local_88,local_90,0);
        thunk_FUN_00561a5b(local_84,local_84,local_d0);
        thunk_FUN_00561a5b(local_84,local_84,local_44);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_84);
        for (local_d4 = 0; local_d4 < 4; local_d4 = local_d4 + 1) {
          *(undefined4 *)(param_1 + 0x38 + local_d4 * 0x18) = 0xaaaaaaaa;
        }
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                  (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0x2c,0x18);
      }
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
  }
  return 1;
}

