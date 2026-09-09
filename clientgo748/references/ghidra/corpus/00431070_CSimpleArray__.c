// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431070 | Name: CSimpleArray<>


/* Library Function - Multiple Matches With Same Base Name
    public: __thiscall ATL::CSimpleArray<unsigned long,class ATL::CSimpleArrayEqualHelper<unsigned
   long> >::CSimpleArray<unsigned long,class ATL::CSimpleArrayEqualHelper<unsigned long> >(void)
    public: __thiscall ATL::CSimpleArray<struct CTraceSnapshot::CModuleInfo,class
   ATL::CSimpleArrayEqualHelper<struct CTraceSnapshot::CModuleInfo> >::CSimpleArray<struct
   CTraceSnapshot::CModuleInfo,class ATL::CSimpleArrayEqualHelper<struct
   CTraceSnapshot::CModuleInfo> >(void)
   
   Library: Visual Studio */

undefined4 * __fastcall CSimpleArray<>(undefined4 *param_1)

{
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  return param_1;
}

