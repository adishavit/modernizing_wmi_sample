#define _WIN32_DCOM


#include <iostream>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

#include <vector>

#include "scoped_utils.h"


int Win32_PerfRawData_PerfProc_Process()
{
   // To add error checking,
   // check returned HRESULT below where collected.
   HRESULT hr = S_OK;

   if (FAILED(hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
      return hr;
   auto uniniter = make_scoped_invoker(CoUninitialize);

   if (FAILED(hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, 0)))
      return hr;

   auto pWbemLocator = [&]
   {
      IWbemLocator* pWbemLocator = nullptr;
      if (FAILED(hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&pWbemLocator)))
         return scoped_ptr<IWbemLocator>();
      return make_scoped_ptr(pWbemLocator, [](IWbemLocator* ptr) { ptr->Release(); });
   }();
   if (!pWbemLocator)
      return hr;

   // Connect to the desired namespace.
   auto bstrNameSpace = make_scoped_ptr(SysAllocString(L"\\\\.\\root\\cimv2"), ::SysFreeString);
   if (!bstrNameSpace)
   {
      hr = E_OUTOFMEMORY;
      return hr;
   }

   auto pNameSpace = [&]
   {
      IWbemServices* pNameSpace = nullptr;
      if (FAILED(hr = pWbemLocator->ConnectServer(bstrNameSpace, nullptr, nullptr, nullptr, 0L, nullptr, nullptr, &pNameSpace)))
         return scoped_ptr<IWbemServices>();
      return make_scoped_ptr(pNameSpace, [](IWbemServices* ptr) { ptr->Release(); });
   }();
   if (!pNameSpace)
      return hr;

   pWbemLocator.release();
   bstrNameSpace.release();

   auto pRefresher = [&]
   {
      IWbemRefresher*pRefresher = nullptr;
      if (FAILED(hr = CoCreateInstance(CLSID_WbemRefresher, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (void**)&pRefresher)))
         return scoped_ptr<IWbemRefresher>();
      return make_scoped_ptr(pRefresher, [](IWbemRefresher* ptr) { ptr->Release(); });
   }();
   if (!pRefresher)
      return hr;

   auto pConfig = [&]
   {
      IWbemConfigureRefresher *pConfig = nullptr;
      if (FAILED(hr = pRefresher->QueryInterface(IID_IWbemConfigureRefresher, (void **)&pConfig)))
         return scoped_ptr<IWbemConfigureRefresher>();
      return make_scoped_ptr(pConfig, [](IWbemConfigureRefresher* ptr) { ptr->Release(); });
   }();
   if (!pRefresher)
      return hr;

   // Add an enumerator to the refresher.
   auto pEnum = [&]
   {
      IWbemHiPerfEnum *pEnum = nullptr;
      long lID = 0;
      if (FAILED(hr = pConfig->AddEnum(pNameSpace, L"Win32_PerfRawData_PerfProc_Process", 0, nullptr, &pEnum, &lID)))
         return scoped_ptr<IWbemHiPerfEnum>();
      return make_scoped_ptr(pEnum, [](IWbemHiPerfEnum* ptr) { ptr->Release(); });
   }();
   pConfig.release();

   // Get a property handle for the VirtualBytes property.

   // Refresh the object ten times and retrieve the value.
   std::vector<IWbemObjectAccess*> apEnumAccess;
   auto enumReleaser = make_scoped_invoker([&]
   {   
      for (auto enumAccess : apEnumAccess)
         if (enumAccess)
            enumAccess->Release();
   });

   long lVirtualBytesHandle = 0;
   long lIDProcessHandle = 0;

   for (auto x = 0; x < 3; ++x)
   {
      if (FAILED(hr = pRefresher->Refresh(0L)))
         return hr;

      DWORD dwNumReturned = 0;
      hr = pEnum->GetObjects(0L, 0, nullptr, &dwNumReturned);
      // If the buffer was not big enough,
      // allocate a bigger buffer and retry.
      if (hr == WBEM_E_BUFFER_TOO_SMALL && dwNumReturned > apEnumAccess.size())
      {
         apEnumAccess.resize(dwNumReturned, nullptr);
         if (FAILED(hr = pEnum->GetObjects(0L, (DWORD)apEnumAccess.size(), apEnumAccess.data(), &dwNumReturned)))
            return hr;
      }
      else
      {
         if (hr == WBEM_S_NO_ERROR)
         {
            hr = WBEM_E_NOT_FOUND;
            return hr;
         }
      }

      // First time through, get the handles.
      if (0 == x)
      {
         CIMTYPE VirtualBytesType;
         CIMTYPE ProcessHandleType;
         if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(L"VirtualBytes", &VirtualBytesType, &lVirtualBytesHandle)))
            return hr;

         if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(L"IDProcess", &ProcessHandleType, &lIDProcessHandle)))
            return hr;
      }

      for (DWORD i = 0; i < dwNumReturned; i++)
      {
         DWORD dwVirtualBytes = 0;
         if (FAILED(hr = apEnumAccess[i]->ReadDWORD(lVirtualBytesHandle, &dwVirtualBytes)))
            return hr;

         DWORD dwIDProcess = 0;
         if (FAILED(hr = apEnumAccess[i]->ReadDWORD(lIDProcessHandle, &dwIDProcess)))
            return hr;

         wprintf(L"Process ID %lu is using %lu bytes\n",dwIDProcess, dwVirtualBytes);
      }
      
      Sleep(1000); // Sleep for a second.
   }

   return 1;
}

int main()
{
   auto hr = Win32_PerfRawData_PerfProc_Process();
   if (FAILED(hr))
   {
      wprintf(L"Error status=%08x\n", hr);
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}