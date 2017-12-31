#define _WIN32_DCOM


#include <iostream>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

#include <vector>

#include "gsl.hpp"

#include "scoped_utils.h"

#define TRY(exp)      do { auto hr__ = exp; if (hr__ < 0) throw hr__; } while(0)

void Win32_PerfRawData_PerfProc_Process()
{
   TRY(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
   auto uniniter = gsl::finally(&CoUninitialize);

   TRY(CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, 0));

   auto pWbemLocator = [&]
   {
      IWbemLocator* pWbemLocator = nullptr;
      TRY(CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&pWbemLocator));
      return make_scoped_ptr(pWbemLocator, [](IWbemLocator* ptr) { ptr->Release(); });
   }();

   // Connect to the desired namespace.
   auto bstrNameSpace = make_scoped_ptr(SysAllocString(L"\\\\.\\root\\cimv2"), ::SysFreeString);
   if (!bstrNameSpace)
      throw E_OUTOFMEMORY;

   auto pNameSpace = [&]
   {
      IWbemServices* pNameSpace = nullptr;
      TRY(pWbemLocator->ConnectServer(bstrNameSpace, nullptr, nullptr, nullptr, 0L, nullptr, nullptr, &pNameSpace));
      return make_scoped_ptr(pNameSpace, [](IWbemServices* ptr) { ptr->Release(); });
   }();

   pWbemLocator.release();
   bstrNameSpace.release();

   auto pRefresher = [&]
   {
      IWbemRefresher*pRefresher = nullptr;
      TRY(CoCreateInstance(CLSID_WbemRefresher, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (void**)&pRefresher));
      return make_scoped_ptr(pRefresher, [](IWbemRefresher* ptr) { ptr->Release(); });
   }();

   auto pConfig = [&]
   {
      IWbemConfigureRefresher *pConfig = nullptr;
      TRY(pRefresher->QueryInterface(IID_IWbemConfigureRefresher, (void **)&pConfig));
      return make_scoped_ptr(pConfig, [](IWbemConfigureRefresher* ptr) { ptr->Release(); });
   }();

   // Add an enumerator to the refresher.
   auto pEnum = [&]
   {
      IWbemHiPerfEnum *pEnum = nullptr;
      long lID = 0;
      TRY(pConfig->AddEnum(pNameSpace, L"Win32_PerfRawData_PerfProc_Process", 0, nullptr, &pEnum, &lID));
      return make_scoped_ptr(pEnum, [](IWbemHiPerfEnum* ptr) { ptr->Release(); });
   }();
   pConfig.release();

   // Get a property handle for the VirtualBytes property.

   // Refresh the object ten times and retrieve the value.
   std::vector<IWbemObjectAccess*> apEnumAccess;
   auto enumReleaser = gsl::finally([&]
   {   
      for (auto enumAccess : apEnumAccess)
         if (enumAccess)
            enumAccess->Release();
   });

   long lVirtualBytesHandle = 0;
   long lIDProcessHandle = 0;

   for (auto x = 0; x < 3; ++x)
   {
      TRY(pRefresher->Refresh(0L));

      DWORD dwNumReturned = 0;
      auto hr = pEnum->GetObjects(0L, 0, nullptr, &dwNumReturned);      
      if (hr == WBEM_E_BUFFER_TOO_SMALL && dwNumReturned > apEnumAccess.size())
      {  
         apEnumAccess.resize(dwNumReturned, nullptr);                                                    // If the buffer was not big enough, 
         TRY(pEnum->GetObjects(0L, (DWORD)apEnumAccess.size(), apEnumAccess.data(), &dwNumReturned));  // allocate a bigger buffer and retry.
      }
      else if (hr == WBEM_S_NO_ERROR)
            throw WBEM_E_NOT_FOUND;

      // First time through, get the handles.
      if (0 == x)
      {
         CIMTYPE VirtualBytesType;
         CIMTYPE ProcessHandleType;
         TRY(apEnumAccess[0]->GetPropertyHandle(L"VirtualBytes", &VirtualBytesType, &lVirtualBytesHandle));
         TRY(apEnumAccess[0]->GetPropertyHandle(L"IDProcess", &ProcessHandleType, &lIDProcessHandle));
      }

      for (DWORD i = 0; i < dwNumReturned; i++)
      {
         DWORD dwVirtualBytes = 0;
         TRY(apEnumAccess[i]->ReadDWORD(lVirtualBytesHandle, &dwVirtualBytes));

         DWORD dwIDProcess = 0;
         TRY(apEnumAccess[i]->ReadDWORD(lIDProcessHandle, &dwIDProcess));

         wprintf(L"Process ID %lu is using %lu bytes\n",dwIDProcess, dwVirtualBytes);
      }
      
      Sleep(1000); // Sleep for a second.
   }
}

int main() try
{
   Win32_PerfRawData_PerfProc_Process();
   return EXIT_SUCCESS;
}
catch (HRESULT hr)
{
   wprintf(L"Error status=%08x\n", hr);
   return EXIT_FAILURE;
}
