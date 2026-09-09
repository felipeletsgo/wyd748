// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042d173 | Name: FUN_0042d173


int __fastcall FUN_0042d173(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_22c [128];
  undefined4 local_1ac;
  undefined4 *local_1a8 [4];
  int *local_198;
  uint local_194;
  int local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_154;
  undefined4 local_150;
  undefined4 local_14c;
  undefined4 local_148;
  undefined4 local_144;
  undefined4 local_140;
  undefined4 local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined1 local_120 [196];
  uint local_5c;
  int local_4c;
  int local_48;
  undefined1 local_44 [64];
  
  (**(code **)(*param_1 + 0x24))(0,0,param_1[0xa941],param_1[0xa942]);
  for (local_4c = 0; local_4c < 10; local_4c = local_4c + 1) {
    (**(code **)(*(int *)param_1[0xa8f9] + 0xf4))(param_1[0xa8f9],local_4c,0);
  }
  if ((param_1[0xa9c1] != 0) ||
     (local_48 = FUN_00560f07(param_1[0xa8f9],param_1 + 0xa9c1), -1 < local_48)) {
    if (DAT_013b71e8 != (int *)0x0) {
      (**(code **)(*DAT_013b71e8 + 0x3c))();
    }
    FUN_0042ced1();
    if (DAT_005b48ec == -1) {
      local_190 = 0;
      local_138 = 0x20000000;
      local_134 = 0x40020000;
      local_130 = 0x40040002;
      local_12c = 0x40020003;
      local_128 = 0x40010004;
      local_124 = 0xffffffff;
      local_154 = 0x20000000;
      local_150 = 0x40020000;
      local_14c = 0x40000001;
      local_148 = 0x40040002;
      local_144 = 0x40020003;
      local_140 = 0x40010004;
      local_13c = 0xffffffff;
      local_170 = 0x20000000;
      local_16c = 0x40020000;
      local_168 = 0x40010001;
      local_164 = 0x40040002;
      local_160 = 0x40020003;
      local_15c = 0x40010004;
      local_158 = 0xffffffff;
      local_18c = 0x20000000;
      local_188 = 0x40020000;
      local_184 = 0x40020001;
      local_180 = 0x40040002;
      local_17c = 0x40020003;
      local_178 = 0x40010004;
      local_174 = 0xffffffff;
      local_1a8[0] = &local_138;
      local_1a8[1] = &local_154;
      local_1a8[2] = &local_170;
      local_1a8[3] = &local_18c;
      param_1[0xaa24] = 0x10;
      (**(code **)(*(int *)param_1[0xa8f9] + 0x1c))(param_1[0xa8f9],local_120);
      if (0xfffe0100 < local_5c) {
        param_1[0xaa24] = 0;
      }
      for (local_194 = 0; local_194 < 8; local_194 = local_194 + 1) {
        FUN_0058f078(local_22c,s_shader_d_bin_005b499c,local_194 + 1);
        iVar1 = FUN_0058fb07(local_22c,0x8000,0);
        if (iVar1 == -1) {
          FUN_00431159(s_Read_VertexShader__d_Error___005b49ac,local_194 + 1);
          return 0;
        }
        local_1ac = FUN_0058f83f(iVar1);
        FUN_0056478d(local_1ac,&local_198);
        uVar2 = (**(code **)(*local_198 + 0xc))(local_198,local_1ac);
        FUN_0058f8c9(iVar1,uVar2);
        uVar2 = (**(code **)(*local_198 + 0xc))(local_198,&DAT_005b48ec + local_194,param_1[0xaa24])
        ;
        local_190 = (**(code **)(*(int *)param_1[0xa8f9] + 300))
                              (param_1[0xa8f9],local_1a8[local_194 % 4],uVar2);
        if (local_190 < 0) {
          return local_190;
        }
        FUN_0058f75f(iVar1);
        (**(code **)(*local_198 + 8))(local_198);
      }
    }
    FUN_00430f10();
    thunk_FUN_00561ca4(local_44,param_1 + 0xa97b);
    (**(code **)(*(int *)param_1[0xa8f9] + 0x13c))(param_1[0xa8f9],2,local_44,4);
    (**(code **)(*param_1 + 0x3c))();
    if (param_1[0xa9c4] != 0) {
      (**(code **)(*(int *)param_1[0xa9c4] + 8))();
    }
  }
  return 0;
}

