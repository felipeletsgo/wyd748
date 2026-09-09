// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005406af | Name: FUN_005406af


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_005406af(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 *puVar5;
  undefined1 local_1f8 [48];
  undefined4 local_1c8;
  undefined4 local_1c4;
  undefined1 local_1c0 [64];
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  int local_174;
  undefined4 local_170;
  undefined4 local_16c;
  undefined4 local_168;
  float local_164 [27];
  int local_f8;
  undefined4 local_f4 [2];
  undefined4 local_ec;
  undefined1 local_e8 [12];
  int local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined1 local_cc [48];
  float local_9c;
  float local_98;
  float local_94;
  undefined1 local_8c [64];
  undefined4 local_4c;
  int local_48;
  undefined1 local_44 [64];
  
  local_48 = 0;
  local_174 = FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),0,180000);
  if ((local_174 != 0) &&
     (iVar1 = FUN_0049c70f(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c)),
     0x7d < iVar1)) {
    local_f8 = *(int *)(DAT_013b71e8 + 0x1b084);
    FUN_00430f10();
    FUN_00430f10();
    FUN_00430f10();
    puVar2 = (undefined4 *)
             FUN_00430f50(*(undefined4 *)(local_f8 + 0x48),*(undefined4 *)(local_f8 + 0x50),
                          *(undefined4 *)(local_f8 + 0x4c));
    local_180 = *puVar2;
    local_17c = puVar2[1];
    local_178 = puVar2[2];
    puVar2 = (undefined4 *)
             FUN_00430f50(*(undefined4 *)(*(int *)(local_f8 + 0x54) + 0x28),
                          *(undefined4 *)(*(int *)(local_f8 + 0x54) + 0x2c),
                          *(undefined4 *)(*(int *)(local_f8 + 0x54) + 0x30));
    local_170 = *puVar2;
    local_16c = puVar2[1];
    local_168 = puVar2[2];
    puVar2 = &local_170;
    puVar5 = local_1f8;
    FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c),
                 *(undefined4 *)(param_1 + 0x30));
    puVar2 = (undefined4 *)FUN_004d3bc0(puVar5,puVar2);
    local_d8 = *puVar2;
    local_d4 = puVar2[1];
    local_d0 = puVar2[2];
    uVar3 = FUN_00430f50(*(undefined4 *)(local_f8 + 0x58),*(undefined4 *)(local_f8 + 0x60),
                         *(undefined4 *)(local_f8 + 0x5c));
    fVar4 = (float10)FUN_004d3cf0(&local_d8,uVar3);
    if (fVar4 <= (float10)_DAT_005a3534) {
      FUN_00430f50(*(undefined4 *)(local_f8 + 0x48),*(undefined4 *)(local_f8 + 0x4c),
                   *(undefined4 *)(local_f8 + 0x50));
      FUN_00430f50(*(undefined4 *)(local_f8 + 0x58),*(undefined4 *)(local_f8 + 0x5c),
                   *(undefined4 *)(local_f8 + 0x60));
      FUN_00421910(local_164,0xc,9,FUN_00430f10);
      for (local_dc = 0; local_dc < 9; local_dc = local_dc + 1) {
        local_164[local_dc * 3] = *(float *)(param_1 + 0x28);
        local_164[local_dc * 3 + 2] = *(float *)(param_1 + 0x2c);
        local_164[local_dc * 3 + 1] = *(float *)(param_1 + 0x30);
      }
      local_164[3] = local_164[0x15] + *(float *)(local_174 + 0x348);
      local_164[5] = -*(float *)(local_174 + 0x350) + local_164[0x14];
      local_164[4] = local_164[0xd] + *(float *)(local_174 + 0x358);
      local_164[6] = local_164[0x18] + *(float *)(local_174 + 0x344);
      local_164[0xb] = -*(float *)(local_174 + 0x34c) + local_164[0x1a];
      local_164[0x10] = local_164[0x19] + *(float *)(local_174 + 0x354);
      local_164[7] = local_164[4];
      local_164[8] = local_164[5];
      local_164[9] = local_164[3];
      local_164[10] = local_164[4];
      local_164[0xc] = local_164[6];
      local_164[0xd] = local_164[4];
      local_164[0xe] = local_164[0xb];
      local_164[0xf] = local_164[3];
      local_164[0x11] = local_164[5];
      local_164[0x12] = local_164[6];
      local_164[0x13] = local_164[0x10];
      local_164[0x14] = local_164[5];
      local_164[0x15] = local_164[3];
      local_164[0x16] = local_164[0x10];
      local_164[0x17] = local_164[0xb];
      local_164[0x18] = local_164[6];
      local_164[0x19] = local_164[0x10];
      local_164[0x1a] = local_164[0xb];
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      thunk_FUN_0056230d(local_1c0,*(undefined4 *)(local_174 + 0x32c),
                         *(undefined4 *)(local_174 + 0x330),*(undefined4 *)(local_174 + 0x32c));
      thunk_FUN_00562821(local_44,*(undefined4 *)(param_1 + 0x34),0xbfc90fdb,0);
      thunk_FUN_00561a5b(local_44,DAT_005ccf9c + 0x2a66c,local_44);
      thunk_FUN_00561a5b(local_44,local_44,local_1c0);
      for (local_dc = 0; local_dc < 9; local_dc = local_dc + 1) {
        thunk_FUN_0056239d(local_8c,local_164[local_dc * 3],local_164[local_dc * 3 + 1],
                           local_164[local_dc * 3 + 2]);
        thunk_FUN_00561a5b(local_cc,local_44,local_8c);
        local_164[local_dc * 3] = local_9c;
        local_164[local_dc * 3 + 1] = local_98;
        local_164[local_dc * 3 + 2] = local_94;
      }
      iVar1 = FUN_005649c7(local_164 + 3,local_164 + 6,local_164 + 0xf,local_f4,local_e8,0,0,0);
      if (iVar1 == 1) {
        local_48 = 1;
      }
      else {
        iVar1 = FUN_005649c7(local_164 + 0xf,local_164 + 0x12,local_164 + 3,local_f4,local_e8,0,0,0)
        ;
        if (iVar1 == 1) {
          local_48 = 1;
        }
        else {
          iVar1 = FUN_005649c7(local_164 + 3,local_164 + 0xc,local_164 + 0x12,local_f4,local_e8,0,0,
                               0);
          if (iVar1 == 1) {
            local_48 = 1;
          }
          else {
            iVar1 = FUN_005649c7(local_164 + 0x12,local_164 + 0x18,local_164 + 0xc,local_f4,local_e8
                                 ,0,0,0);
            if (iVar1 == 1) {
              local_48 = 1;
            }
            else {
              iVar1 = FUN_005649c7(local_164 + 9,local_164 + 0xc,local_164 + 0x15,local_f4,local_e8,
                                   0,0,0);
              if (iVar1 == 1) {
                local_48 = 1;
              }
              else {
                iVar1 = FUN_005649c7(local_164 + 0x15,local_164 + 0x18,local_164 + 0xc,local_f4,
                                     local_e8,0,0,0);
                if (iVar1 == 1) {
                  local_48 = 1;
                }
                else {
                  iVar1 = FUN_005649c7(local_164 + 3,local_164 + 9,local_164 + 0xf,local_f4,local_e8
                                       ,0,0,0);
                  if (iVar1 == 1) {
                    local_48 = 1;
                  }
                  else {
                    iVar1 = FUN_005649c7(local_164 + 0xf,local_164 + 0x15,local_164 + 9,local_f4,
                                         local_e8,0,0,0);
                    if (iVar1 == 1) {
                      local_48 = 1;
                    }
                    else {
                      local_48 = 0;
                    }
                  }
                }
              }
            }
          }
        }
      }
      local_4c = *(undefined4 *)(local_174 + 0x340);
      if (local_48 == 1) {
        FUN_00430f20(local_f4[0],local_ec);
        iVar1 = FUN_0049c70f(local_1c8,local_1c4);
        if (0x7d < iVar1) {
          local_48 = 2;
        }
      }
    }
  }
  return local_48;
}

