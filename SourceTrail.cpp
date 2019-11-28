#include "sdk.h" // Code::Blocks SDK
#include <configurationpanel.h>
#include "SourceTrail.h"
#include "stclient.h"
#include <wx/log.h>

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<SourceTrail> reg(_T("SourceTrail"));
    int idSetActive = wxNewId();

}


// events handling
BEGIN_EVENT_TABLE(SourceTrail, cbPlugin)
	EVT_MENU(idSetActive, SourceTrail::OnSetActive)
END_EVENT_TABLE()

// constructor
SourceTrail::SourceTrail()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("SourceTrail.zip")))
    {
        NotifyMissingFile(_T("SourceTrail.zip"));
    }
    m_FileNewMenu = 0;
    wxLogMessage(wxT("SourceTrail::SourceTrail"));
}

// destructor
SourceTrail::~SourceTrail()
{
}

void SourceTrail::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...
    wxLogMessage(wxT("SourceTrail::OnAttach"));
    Manager::Get()->GetLogManager()->Log(wxT("Create Client"));
    m_pClient = new stClient(6666,6667);
    m_pClient->StartServer();
    cbPlugin::OnAttach();
}

void SourceTrail::OnRelease(bool appShutDown)
{
    wxLogMessage(wxT("SourceTrail::OnRelease"));
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    delete m_pClient;
    if (m_FileNewMenu)
	{
		m_FileNewMenu->Delete(idSetActive);
		m_FileNewMenu = 0;
	}
    cbPlugin::OnRelease(appShutDown);
}

void SourceTrail::BuildMenu(wxMenuBar* menuBar)
{
    wxLogMessage(wxT("SourceTrail::BuildMenu"));
    if (m_FileNewMenu)
	{
		m_FileNewMenu->Delete(idSetActive);
		m_FileNewMenu = 0;
	}

    const int pos = menuBar->FindMenu(_("&Edit"));
    if (pos != wxNOT_FOUND)
    {
        wxMenu* pEditMenu = menuBar->GetMenu(pos);
        pEditMenu->AppendSeparator();
        pEditMenu->Append( idSetActive, _( "SourceTrail: Show Token" ), _( "Send the token under the caret to SourceTrail" ) );
    }
    else
        wxLogMessage(_T("Could not find Edit Menu!"));
}

void SourceTrail::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    if ( !menu || !IsAttached() )
        return;

    switch ( type )
    {
        case mtEditorManager:
            menu->AppendSeparator();
            menu->Append( idSetActive, _( "SourceTrail: Show Token" ), _( "Send the token under the caret to SourceTrail" ) );
    }

}
//
//bool SourceTrail::BuildToolBar(wxToolBar* toolBar)
//{
//    //The application is offering its toolbar for your plugin,
//    //to add any toolbar items you want...
//    //Append any items you need on the toolbar...
//    NotImplemented(_T("SourceTrail::BuildToolBar()"));
//
//    // return true if you add toolbar items
//    return false;
//}

void SourceTrail::OnSetActive(wxCommandEvent& event)
{
    if(m_pClient)
    {
        m_pClient->SendLocation();
    }
}
