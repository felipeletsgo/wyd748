// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bb847 | Name: FUN_004bb847


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_004bb847(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,float param_6,undefined4 param_7,undefined1 param_8,int param_9)

{
  undefined4 uVar1;
  undefined1 local_c4 [64];
  undefined1 local_84 [64];
  undefined1 local_44 [64];
  
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  if (param_1[1] == 1) {
    if ((uint)param_1[0xc9] < 0x57e41) {
      uVar1 = FUN_004b925c(param_1[0xa6] + param_9,360000);
      (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
    }
    else {
      uVar1 = FUN_004b925c(param_1[0xa6] + param_9,param_1[0xc9]);
      (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
    }
  }
  else if ((uint)param_1[0xc9] < 0x2bf21) {
    uVar1 = FUN_004b9a4b(param_1[0xa6] + param_9,180000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
  }
  else {
    uVar1 = FUN_004b9a4b(param_1[0xa6] + param_9,param_1[0xc9]);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
  }
  thunk_FUN_0056230d(local_c4,param_1[0xcb],param_1[0xcc],param_1[0xcb]);
  thunk_FUN_0056239d(local_84,param_2,param_3,param_4);
  thunk_FUN_00562821(local_44,param_5,_DAT_005a46bc + param_6,param_7);
  thunk_FUN_00561a5b(local_44,DAT_005ccf9c + 0xa99b,local_44);
  thunk_FUN_00561a5b(local_44,local_44,local_c4);
  thunk_FUN_00561a5b(local_44,local_44,local_84);
  (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_44);
  (**(code **)(*param_1 + 0xc))(param_8,param_9);
  return 1;
}

