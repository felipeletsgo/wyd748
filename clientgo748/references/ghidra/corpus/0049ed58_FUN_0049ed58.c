// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ed58 | Name: FUN_0049ed58


void __thiscall
FUN_0049ed58(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  undefined4 uVar1;
  undefined1 local_70 [4];
  undefined2 local_6c;
  undefined2 local_6a;
  undefined1 local_64 [96];
  
  _memset(local_70,0,0x6c);
  local_6a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
  local_6c = 700;
  if (param_2 == 10) {
    FUN_0058f078(local_64,s_00000000___Load_Tile_Map_Fail_005b9278);
  }
  else {
    uVar1 = __ftol(param_5,param_6,param_2);
    uVar1 = __ftol(uVar1);
    FUN_0058f078(local_64,s__08d___Critical_Data_Err_Cl__d___005b9298,param_3,param_4,uVar1);
  }
  FUN_0042550e(local_70,0x6c);
  return;
}

