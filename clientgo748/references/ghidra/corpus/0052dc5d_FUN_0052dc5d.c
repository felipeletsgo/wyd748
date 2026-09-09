// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052dc5d | Name: FUN_0052dc5d


undefined4 __thiscall FUN_0052dc5d(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  void *pvVar3;
  char *_Str2;
  int local_2ac;
  undefined1 local_28c [128];
  undefined1 local_20c [128];
  int *local_18c;
  int *local_188;
  int local_184;
  int local_180;
  int local_17c;
  int local_178;
  void *local_174;
  undefined4 *local_170;
  int local_16c;
  int local_168;
  int local_164;
  undefined4 auStack_160 [15];
  int *local_124;
  char local_120 [128];
  undefined1 local_a0 [128];
  int local_20;
  int local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a244b;
  local_10 = ExceptionList;
  local_14 = param_2;
  if (param_1 != *(int *)(DAT_0067cf38 + 0x4c)) {
    return 1;
  }
  ExceptionList = &local_10;
  iVar1 = FUN_00494dcf();
  if (iVar1 != 30000) {
    ExceptionList = local_10;
    return 1;
  }
  local_1c = DAT_0067cf38;
  local_18 = (int *)(**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x259);
  (**(code **)(*local_18 + 0x8c))(*(undefined1 *)(local_14 + 0x98));
  if ((*(short *)((int)DAT_013b71e8 + 0xcfa) == 0) && (*(short *)(local_14 + 0x9a) != 0)) {
    local_20 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(local_14 + 0x9a));
    if (local_20 == 0) {
      ExceptionList = local_10;
      return 1;
    }
    FUN_0058f078(local_a0,&DAT_00a36178,local_20 + 0x168);
    (**(code **)(**(int **)(local_1c + 0x8c) + 0x8c))(local_a0,0x259,&DAT_00a34f78);
    *(uint *)(*(int *)(local_1c + 0x8c) + 0x1e8) = (uint)*(ushort *)(local_14 + 0x9a);
    (**(code **)(**(int **)(local_1c + 0x8c) + 0x60))(1);
    (**(code **)(*DAT_005ccec0 + 0x98))();
    FUN_0058f220(DAT_013b71e8 + 0x318,local_14,0x9c);
    *(undefined2 *)((int)DAT_013b71e8 + 0xcfa) = 0;
    ExceptionList = local_10;
    return 1;
  }
  local_164 = 0;
  local_16c = DAT_0067cf38;
  for (local_168 = 0; local_168 < 0xf; local_168 = local_168 + 1) {
    uVar2 = (**(code **)(**(int **)(local_16c + 0x28) + 0x48))(local_168 + 0x2000);
    auStack_160[local_168] = uVar2;
    local_170 = (undefined4 *)(**(code **)(*(int *)auStack_160[local_168] + 0xa4))(0,0);
    if ((local_170 != (undefined4 *)0x0) &&
       (iVar1 = _memcmp((void *)local_170[0x19c],(void *)(local_14 + 0xc + local_168 * 8),8),
       iVar1 != 0)) {
      local_164 = 1;
    }
    if ((local_170 == (undefined4 *)0x0) && (0 < *(short *)(local_14 + 0xc + local_168 * 8))) {
      local_164 = 1;
    }
    if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_170)) {
      DAT_005ccec0[0x7a] = 0;
    }
    if (local_170 != (undefined4 *)0x0) {
      if (local_170 != (undefined4 *)0x0) {
        (**(code **)*local_170)(1);
      }
      local_170 = (undefined4 *)0x0;
    }
    if ((0 < *(short *)(local_14 + 0xc + local_168 * 8)) &&
       (local_174 = operator_new(8), local_174 != (void *)0x0)) {
      FUN_0058f220(local_174,local_14 + 0xc + local_168 * 8,8);
      pvVar3 = operator_new(0x678);
      local_8 = 0;
      if (pvVar3 == (void *)0x0) {
        local_2ac = 0;
      }
      else {
        local_2ac = FUN_0040d13e(0,local_174,0,0);
      }
      local_8 = 0xffffffff;
      local_178 = local_2ac;
      if (local_2ac != 0) {
        (**(code **)(*(int *)auStack_160[local_168] + 0x8c))(local_2ac,0,0);
      }
    }
  }
  local_124 = (int *)(**(code **)(**(int **)(local_16c + 0x28) + 0x48))(0x25b);
  FUN_0058f078(local_120,&DAT_005bb8ac,*(undefined4 *)(local_14 + 0x94));
  if (local_164 != 1) {
    _Str2 = (char *)(**(code **)(*local_124 + 0x88))();
    iVar1 = _strcmp(local_120,_Str2);
    if (iVar1 == 0) goto LAB_0052e1b1;
  }
  local_17c = (**(code **)(**(int **)(local_16c + 0x28) + 0x48))(0x269);
  local_180 = (**(code **)(**(int **)(local_16c + 0x28) + 0x48))(0x259);
  *(undefined4 *)(local_17c + 0x1e8) = 0;
  *(undefined4 *)(local_180 + 0x1e8) = 0;
  uVar2 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
  *(undefined4 *)(local_16c + 0x274d0) = uVar2;
  *(undefined1 *)(DAT_013b71e8 + 0x33e) = *(undefined1 *)(local_17c + 0x1e8);
LAB_0052e1b1:
  (**(code **)(*local_124 + 0x80))(local_120,0);
  (**(code **)(**(int **)(local_1c + 0x28) + 0x48))(0x240);
  iVar1 = FUN_0040c0f0();
  if (iVar1 == 0) {
    local_18c = (int *)(**(code **)(**(int **)(local_1c + 0x28) + 0x48))(0x26a);
    local_188 = (int *)(**(code **)(**(int **)(local_1c + 0x28) + 0x48))(0x25a);
    local_184 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(local_14 + 0x9a));
    if (local_184 != 0) {
      FUN_0058f078(local_28c,&DAT_005bb8b4,param_1 + 0x168);
      FUN_0058f078(local_20c,&DAT_005bb8bc,local_184 + 0x168);
      (**(code **)(*local_18c + 0x80))(local_28c,1);
      (**(code **)(*local_188 + 0x80))(local_20c,1);
      FUN_0044b890(1);
    }
  }
  ExceptionList = local_10;
  return 1;
}

