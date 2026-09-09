// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005926db | Name: FUN_005926db


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005926db(UINT param_1,int param_2,int param_3)

{
  HANDLE hProcess;
  undefined4 *puVar1;
  UINT uExitCode;
  
  FUN_00592780();
  if (DAT_013b8024 == 1) {
    uExitCode = param_1;
    hProcess = GetCurrentProcess();
    TerminateProcess(hProcess,uExitCode);
  }
  _DAT_013b8020 = 1;
  DAT_013b801c = (undefined1)param_3;
  if (param_2 == 0) {
    if ((DAT_013b873c != (undefined4 *)0x0) &&
       (puVar1 = (undefined4 *)(DAT_013b8738 - 4), DAT_013b873c <= puVar1)) {
      do {
        if ((code *)*puVar1 != (code *)0x0) {
          (*(code *)*puVar1)();
        }
        puVar1 = puVar1 + -1;
      } while (DAT_013b873c <= puVar1);
    }
    FUN_00592792(&DAT_005b1030,&DAT_005b1038);
  }
  FUN_00592792(&DAT_005b103c,&DAT_005b1044);
  if (param_3 == 0) {
    DAT_013b8024 = 1;
                    /* WARNING: Subroutine does not return */
    ExitProcess(param_1);
  }
  FUN_00592789();
  return;
}

