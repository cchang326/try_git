//
  // Copyright (c) 2008 Microsoft Corporation. All Rights Reserved.
//
// Abstract:
//
// This class implements Command Plugin Channel Factory. This factory creates the Command
// plugin channel for each connection.
// 
//

#include "StdAfx.h"
#include "RdpWdUmx.h" //for input channel name
#include <CommandPluginFactory.h>
#include <RdpVirtualChannels.h>

#ifdef RUN_WPP
#include <CommandPluginFactory.cpp.tmh>
#endif

HRESULT
CRdpCommandChannelPluginFactory_CreateInstance(
    IUnknown* pUnkPlatformContext,
    REFIID    riid,
    PVOID*    ppvObject
    )
{
    HRESULT hr = S_OK;
    CRdpCommandChannelPluginFactory                    *pCommandPluginFactory=NULL;
        
    TS_ENTERFN();
    
    CHECK_QUIT_BOOL_TO_HR_SWITCH( ppvObject, E_POINTER, "The pointer used to return the session manger object is null");

    CHECK_QUIT_BOOL_TO_HR_SWITCH( pUnkPlatformContext, E_INVALIDARG, "No platform context");

    *ppvObject = NULL;

    pCommandPluginFactory = new CRdpCommandChannelPluginFactory(pUnkPlatformContext);
    CHECK_QUIT_BOOL_TO_HR_SWITCH( pCommandPluginFactory, E_OUTOFMEMORY, "Failed to allocate the CRdpInputChannelPluginFactory object");
    pCommandPluginFactory->AddRef();
    
    hr = pCommandPluginFactory->QueryInterface(riid, ppvObject);
    CHECK_QUIT_HR("Failed to QI");
    
TS_EXIT_POINT:
    
    if (pCommandPluginFactory) {
        pCommandPluginFactory->Release();
    }
    
    TS_LEAVEFN();

    return hr;
}

STDMETHODIMP
CRdpCommandChannelPluginFactory::InitializeInstance()
{
    return S_OK;
}

STDMETHODIMP
CRdpCommandChannelPluginFactory::TerminateInstance()
{
    return S_OK;
}

// Creates a plug-in instance for that connection
STDMETHODIMP
CRdpCommandChannelPluginFactory::CreateConnectionChannels(
    IRDPCoreConnection* pConnection
    )
{
    HRESULT hr;
    VARIANT cvCommandPlugin;
    ComSmartPtr<IRDPCoreChannelManager>     spChannelMgr;
    ComSmartPtr<IRDPCoreVirtualChannel>     spCommandChannel;
    ComSmartPtr<IRDPCOREChannelPlugin>      spCommandPlugin;
    ComSmartPtr<IUnknown>                   spUnkCommandPlugin;
    ComSmartPtr<IRDPCollection>             spCollection;

    VariantInit(&cvCommandPlugin);
    
    hr = pConnection->get_ChannelManager(&spChannelMgr);
    CHECK_QUIT_HR("Failed to get VC Mgr");

    hr = pConnection->get_Properties(&spCollection);
    CHECK_QUIT_HR("Failed to get IRDPCOllection from Connection object");

    hr = spChannelMgr->CreateChannel(RDPWDUMX_CHANNEL_NAME_RESERVED_COMMAND_A, 
                        RdpCoreChannelType_GuaranteedDelivery, &spCommandChannel);
    CHECK_QUIT_HR("Failed to create command Channel");

    hr = RDPAPI_CreateInstance(m_spPlatformContext,
              CLSID_UMTSCommandMgr, 
              IID_IRDPCOREChannelPlugin, 
              (void**)&spCommandPlugin);
    CHECK_QUIT_HR("Failed to create Command Channel Factory");    

    hr = spCommandPlugin->InitializeInstance(spCommandChannel, spCollection);
    CHECK_QUIT_HR("Failed to init Command Plugin");

    hr = spCommandPlugin->QueryInterface(IID_IUnknown, (VOID**)&spUnkCommandPlugin);
    CHECK_QUIT_HR("Failed to QI for IUnknown from Command Plugin");
    
    cvCommandPlugin.vt = VT_UNKNOWN;
    cvCommandPlugin.punkVal = spUnkCommandPlugin;
    spUnkCommandPlugin->AddRef();
    
    spCollection->SetProperty(CONN_PROPERTY_UMTS_COMMANDPLUGIN_ID, &cvCommandPlugin);
    CHECK_QUIT_HR("Failed to set UMTS Command plugin property");

TS_EXIT_POINT:

    VariantClear(&cvCommandPlugin);
    return hr;    
}


STDMETHODIMP
CRdpCommandChannelPluginFactory::DestroyConnectionChannels(
       IRDPCoreConnection* pConnection
       )
    {
    HRESULT hr = S_OK;
    ComSmartPtr<IRDPCollection>             spCollection;
    ComSmartPtr<IRDPCOREChannelPlugin>      spCommandPlugin;

    hr = pConnection->get_Properties(&spCollection);
    CHECK_QUIT_HR("Failed to get IRDPCOllection from Connection object");

    hr = spCollection->GetPropertyInterface(CONN_PROPERTY_UMTS_COMMANDPLUGIN_ID,IID_IRDPCOREChannelPlugin,(void**)&spCommandPlugin);
    CHECK_HR("Failed to get CONN_PROPERTY_UMTS_COMMANDPLUGIN_ID Connection object");
        
    if(spCommandPlugin)
    {
        hr = spCommandPlugin->TerminateInstance();
        CHECK_HR("Failed TerminateInstance");
    }

    spCollection->SetPropertyInterface(CONN_PROPERTY_UMTS_COMMANDPLUGIN_ID,(IUnknown*)NULL);

TS_EXIT_POINT:
    return S_OK;
}

    
