// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00426022 | Name: FUN_00426022


undefined4 __fastcall FUN_00426022(int *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  uint local_7f4;
  uint local_7f0;
  int local_7ec [20];
  uint local_79c;
  int aiStack_798 [20];
  int *local_748;
  uint local_744;
  int aiStack_740 [20];
  uint local_6f0;
  uint local_6ec;
  uint local_6e8;
  uint local_6e4;
  uint local_6dc;
  int *local_6d8;
  uint local_6d4 [400];
  uint auStack_94 [20];
  uint local_44;
  uint local_40;
  uint local_3c;
  uint local_38;
  uint local_34;
  int local_30;
  int local_2c;
  int local_28 [3];
  undefined *local_1c [3];
  uint local_10;
  uint local_c;
  int local_8;
  
  local_1c[2] = (undefined *)0x2;
  local_1c[0] = &DAT_005b27e4;
  local_1c[1] = &DAT_005b27e8;
  local_28[0] = 1;
  local_28[1] = 2;
  local_8 = 0;
  local_28[2] = 0;
  local_2c = 0;
  local_30 = 0;
  local_10 = 0;
  do {
    uVar1 = (**(code **)(*(int *)param_1[0xa8f8] + 0x10))(param_1[0xa8f8]);
    if (uVar1 <= local_10) {
      if (param_1[0xa8df] == 0) {
        uVar3 = 0x82000003;
      }
      else {
        for (local_c = 0; local_c < (uint)param_1[0xa8df]; local_c = local_c + 1) {
          for (local_7f4 = 0; local_7f4 < (uint)param_1[local_c * 0x10e3 + 0x110];
              local_7f4 = local_7f4 + 1) {
            if (param_1[local_c * 0x10e3 + local_7f4 * 0x32a + 0x439] != 0) {
              param_1[local_c * 0x10e3 + 0x10e3] = local_7f4;
              param_1[0xa8e0] = local_c;
              param_1[0xa8e1] = 1;
              if (param_1[local_c * 0x10e3 + local_7f4 * 0x32a + 0x111] == 2) {
                if (local_8 == 0) {
                  FUN_00425d36(0x82000005,2);
                }
                else if (local_30 == 0) {
                  FUN_00425d36(0x82000006,2);
                }
                else if (local_28[2] == 0) {
                  FUN_00425d36(0x82000007,2);
                }
                else if (local_2c == 0) {
                  FUN_00425d36(0x82000008,2);
                }
                else {
                  FUN_00425d36(0x82000009,2);
                }
              }
              return 0;
            }
          }
        }
        uVar3 = 0x82000004;
      }
      return uVar3;
    }
    local_6d8 = param_1 + param_1[0xa8df] * 0x10e3 + 1;
    (**(code **)(*(int *)param_1[0xa8f8] + 0x14))(param_1[0xa8f8],local_10,0,local_6d8);
    (**(code **)(*(int *)param_1[0xa8f8] + 0x20))(param_1[0xa8f8],local_10,local_6d8 + 0x10b);
    local_6d8[0x10f] = 0;
    local_6d8[0x10e2] = 0;
    local_3c = 0;
    local_38 = 0;
    local_34 = (**(code **)(*(int *)param_1[0xa8f8] + 0x18))(param_1[0xa8f8],local_10);
    auStack_94[local_3c] = local_6d8[0x10e];
    local_3c = local_3c + 1;
    for (local_44 = 0; local_44 < local_34; local_44 = local_44 + 1) {
      (**(code **)(*(int *)param_1[0xa8f8] + 0x1c))(param_1[0xa8f8],local_10,local_44,&local_6e8);
      if ((0x27f < local_6e8) && (399 < local_6e4)) {
        local_6ec = 0;
        while ((local_6ec < local_38 &&
               (((local_6d4[local_6ec * 4] != local_6e8 ||
                 (local_6d4[local_6ec * 4 + 1] != local_6e4)) ||
                (local_6d4[local_6ec * 4 + 3] != local_6dc))))) {
          local_6ec = local_6ec + 1;
        }
        if (local_6ec == local_38) {
          local_6d4[local_38 * 4] = local_6e8;
          local_6d4[local_38 * 4 + 1] = local_6e4;
          local_6d4[local_38 * 4 + 3] = local_6dc;
          local_6d4[local_38 * 4 + 2] = 0;
          local_38 = local_38 + 1;
          for (local_6f0 = 0; (local_6f0 < local_3c && (local_6dc != auStack_94[local_6f0]));
              local_6f0 = local_6f0 + 1) {
          }
          if (local_6f0 == local_3c) {
            auStack_94[local_3c] = local_6dc;
            local_3c = local_3c + 1;
          }
        }
      }
    }
    FUN_005900f0(local_6d4,local_38,0x10,FUN_00425a8f);
    for (local_40 = 0; local_40 < 2; local_40 = local_40 + 1) {
      local_748 = local_6d8 + local_6d8[0x10f] * 0x32a + 0x110;
      *local_748 = local_28[local_40];
      (**(code **)(*(int *)param_1[0xa8f8] + 0x34))
                (param_1[0xa8f8],local_10,local_28[local_40],local_748 + 1);
      local_748[0x36] = (int)local_1c[local_40];
      local_748[0x38] = 0;
      local_748[0x327] = 0;
      local_748[0x37] = 0;
      local_748[0x328] = 0;
      local_748[0x329] = 0;
      for (local_744 = 0; local_744 < local_3c; local_744 = local_744 + 1) {
        local_7ec[local_744] = 0;
        aiStack_740[local_744] = 0;
        iVar2 = (**(code **)(*(int *)param_1[0xa8f8] + 0x24))
                          (param_1[0xa8f8],local_10,*local_748,auStack_94[local_744],
                           auStack_94[local_744],0);
        if (-1 < iVar2) {
          if (((*local_748 == 1) && (local_8 = 1, (local_748[4] & 0x80000U) != 0)) &&
             (local_28[2] = 1, local_744 == 0)) {
            local_2c = 1;
          }
          if ((local_748[8] & 0x10000U) != 0) {
            if ((local_748[8] & 0x100000U) != 0) {
              aiStack_798[local_744] = 0x50;
              iVar2 = (**(code **)(*param_1 + 8))
                                (local_748 + 1,aiStack_798[local_744],auStack_94[local_744]);
              if (-1 < iVar2) {
                local_7ec[local_744] = 1;
              }
            }
            if (local_7ec[local_744] == 0) {
              aiStack_798[local_744] = 0x40;
              iVar2 = (**(code **)(*param_1 + 8))
                                (local_748 + 1,aiStack_798[local_744],auStack_94[local_744]);
              if (-1 < iVar2) {
                local_7ec[local_744] = 1;
              }
            }
            if (local_7ec[local_744] == 0) {
              aiStack_798[local_744] = 0x80;
              iVar2 = (**(code **)(*param_1 + 8))
                                (local_748 + 1,aiStack_798[local_744],auStack_94[local_744]);
              if (-1 < iVar2) {
                local_7ec[local_744] = 1;
              }
            }
          }
          if (local_7ec[local_744] == 0) {
            aiStack_798[local_744] = 0x20;
            iVar2 = (**(code **)(*param_1 + 8))
                              (local_748 + 1,aiStack_798[local_744],auStack_94[local_744]);
            if (-1 < iVar2) {
              local_7ec[local_744] = 1;
            }
          }
          if (((local_7ec[local_744] != 0) && (param_1[0xa969] != 0)) &&
             (iVar2 = FUN_00426ca6(local_10,*local_748,auStack_94[local_744],aiStack_740 + local_744
                                  ), iVar2 == 0)) {
            local_7ec[local_744] = 0;
          }
        }
      }
      for (local_79c = 0; local_79c < local_38; local_79c = local_79c + 1) {
        for (local_7f0 = 0; local_7f0 < local_3c; local_7f0 = local_7f0 + 1) {
          if ((local_6d4[local_79c * 4 + 3] == auStack_94[local_7f0]) && (local_7ec[local_7f0] == 1)
             ) {
            local_748[local_748[0x38] * 5 + 0x39] = local_6d4[local_79c * 4];
            local_748[local_748[0x38] * 5 + 0x3a] = local_6d4[local_79c * 4 + 1];
            local_748[local_748[0x38] * 5 + 0x3b] = local_6d4[local_79c * 4 + 3];
            local_748[local_748[0x38] * 5 + 0x3c] = aiStack_798[local_7f0];
            local_748[local_748[0x38] * 5 + 0x3d] = aiStack_740[local_7f0];
            local_748[0x38] = local_748[0x38] + 1;
            if (*local_748 == 1) {
              local_30 = 1;
            }
          }
        }
      }
      for (local_79c = 0; local_79c < (uint)local_748[0x38]; local_79c = local_79c + 1) {
        if ((local_748[local_79c * 5 + 0x39] == param_1[0xa941]) &&
           (local_748[local_79c * 5 + 0x3a] == param_1[0xa942])) {
          local_748[0x327] = local_79c;
          if (param_1[0xa944] == 0x10) {
            if (((local_748[local_79c * 5 + 0x3b] == 0x17) ||
                (local_748[local_79c * 5 + 0x3b] == 0x18)) ||
               (local_748[local_79c * 5 + 0x3b] == 0x19)) {
              param_1[0xa945] = local_748[local_79c * 5 + 0x3b];
              break;
            }
          }
          else if (((local_748[local_79c * 5 + 0x3b] == 0x16) ||
                   (local_748[local_79c * 5 + 0x3b] == 0x15)) ||
                  (local_748[local_79c * 5 + 0x3b] == 0x14)) {
            param_1[0xa945] = local_748[local_79c * 5 + 0x3b];
            break;
          }
        }
      }
      if ((local_7ec[0] != 0) && ((local_748[4] & 0x80000U) != 0)) {
        local_748[0x37] = 1;
        local_748[0x328] = 1;
      }
      if (local_748[0x38] != 0) {
        local_6d8[0x10f] = local_6d8[0x10f] + 1;
      }
    }
    if (local_6d8[0x10f] != 0) {
      param_1[0xa8df] = param_1[0xa8df] + 1;
    }
    local_10 = local_10 + 1;
  } while( true );
}

