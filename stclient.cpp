#include "stclient.h"
#include <wx/tokenzr.h>
#include "sdk.h" // Code::Blocks SDK
#include "cbstyledtextctrl.h"

const wxString stClient::MSG_END = wxT("<EOM>");
const wxString stClient::MSG_PING = wxT("ping");
const wxString stClient::MSG_CURSOR = wxT("moveCursor");
const wxString stClient::MSG_CREATE_CDB = wxT("createCDB");

stClient::stClient(unsigned int nServerPort, unsigned int nWritePort) : m_pServer(0), m_bConnected(false),
m_nServerPort(nServerPort),
m_nWritePort(nWritePort)
{

}

stClient::~stClient()
{
    if(m_pServer)
    {
        m_pServer->Destroy();

    }
}

bool stClient::SendMessageToSourceTrail(const wxString& sMessage)
{

    wxIPV4address addr;
    addr.Hostname(wxT("localhost"));
    addr.Service(m_nWritePort);
// Create the socket
    wxSocketClient* pSocket = new wxSocketClient();

    // Set up the event handler and subscribe to most events
    int nId = wxNewId();

    // Wait for the connection event
    pSocket->Connect(addr, false);
    pSocket->WaitOnConnect(2);
    if(pSocket->IsConnected())
    {
        pSocket->SetFlags(wxSOCKET_BLOCK | wxSOCKET_WAITALL);
        Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("SENT: '%s'"), sMessage.c_str()));
        char* pBuffer  = new char[sMessage.length()];
        strcpy(pBuffer, sMessage.ToAscii());
        pSocket->Write(pBuffer, sMessage.length());
        pSocket->Close();
        pSocket->Destroy();
        delete[] pBuffer;
    }
}


void stClient::HandleMessages(const wxString& sBuffer)
{
    Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("HandleMessages: "), sBuffer.c_str()));
    m_sMessageBuffer << sBuffer;

    while(m_sMessageBuffer.empty() == false)
    {
        int nFind = m_sMessageBuffer.Find(wxT("<EOM>"));
        if(nFind == wxNOT_FOUND)
        {
            break;
        }
        else
        {
            wxString sMessage = m_sMessageBuffer.Left(nFind);
            m_sMessageBuffer = m_sMessageBuffer.Mid(nFind+5);
            DecodeMessage(sMessage);
        }
    }
}


void stClient::DecodeMessage(wxString sMessage)
{
    Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("DecodeMessage: %s"), sMessage.c_str()));
    wxArrayString asDecode = wxStringTokenize(sMessage, wxT(">>"));
    for(size_t i = 0; i < asDecode.size(); i++)
    {
        asDecode[i].Trim();
        Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("%d = %s"), i, asDecode[i].c_str()));
    }

    if(asDecode[0] == MSG_PING)
    {
        SendPing();
    }
    else if(asDecode[0] == MSG_CURSOR && asDecode.size() > 6)
    {
        unsigned long nLine;
        unsigned long nCol;
        if(asDecode[4].ToULong(&nLine) && asDecode[6].ToULong(&nCol))
        {
            MoveCursor(asDecode[2], nLine, nCol);
        }
    }
    else
    {
        Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("'%s' != '%s'"), asDecode[0].c_str(), MSG_CURSOR.c_str()));
    }
}

void stClient::MoveCursor(const wxString& sFile, unsigned long nLine, unsigned long nCol)
{
    Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("MoveCursor: %s %d,%d"), sFile.c_str(), nLine, nCol));

    cbEditor* pEditor = (cbEditor*)Manager::Get()->GetEditorManager()->IsBuiltinOpen(sFile);
    if (!pEditor)
    {
        pEditor = Manager::Get()->GetEditorManager()->Open(sFile); //this will send a editor activated event
    }
    if (pEditor)
    {
        Manager::Get()->GetEditorManager()->SetActiveEditor(pEditor);
        pEditor->Activate(); //this does not run FillList, because m_Ignore is true here
        pEditor->SetFocus();           // ...and set focus to this editor
        cbEditor* pInbuilt = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
        pInbuilt->GotoLine(nLine, true);
        cbStyledTextCtrl* pControl = pInbuilt->GetControl();
        nCol += pControl->PositionFromLine(nLine-1);
        pControl->SetCurrentPos(nCol);
        pControl->SetSelection(nCol, nCol);
        pControl->EnsureCaretVisible();

    }
    else
    {
        Manager::Get()->GetLogManager()->Log(wxT("Could not create an editor for file"));
    }
}


bool stClient::StartServer()
{
    Manager::Get()->GetLogManager()->Log(wxT("stClient::StartServer"));
    // Create the address - defaults to localhost:0 initially
    wxIPV4address addr;
    addr.Service(m_nServerPort);
    // Create the socket. We maintain a class pointer so we can
    // sut it down
    m_pServer = new wxSocketServer(addr);
    // We use Ok() here to see if the server is really listening
    if (! m_pServer->Ok())
    {
        Manager::Get()->GetLogManager()->Log(wxT("stClient Failed to start server"));
        return false;
    }
    Manager::Get()->GetLogManager()->Log(wxT("stClient Server started"));
    // Set up the event handler and subscribe to connection events
    int nId = wxNewId();
    m_pServer->SetEventHandler(*this, nId);
    m_pServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
    m_pServer->Notify(true);

    Connect(nId, wxEVT_SOCKET, (wxObjectEventFunction)&stClient::OnServerSocketEvent);
    return true;
}


void stClient::OnServerSocketEvent(wxSocketEvent& event)
{
    Manager::Get()->GetLogManager()->Log(wxT("OnServerSocketEvent: Connection"));
    // Accept the new connection and get the socket pointer
    wxSocketBase* pSock = m_pServer->Accept(false);
    // Tell the new socket how and where to process its events
    int nId = wxNewId();
    pSock->SetEventHandler(*this, nId);
    pSock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    pSock->Notify(true);
    Connect(nId, wxEVT_SOCKET, (wxObjectEventFunction)&stClient::OnSocketEvent);
}
void stClient::OnSocketEvent(wxSocketEvent& event)
{
    Manager::Get()->GetLogManager()->Log(wxT("OnSocketEvent"));
    wxSocketBase * pSock = event.GetSocket();
    char buf[512];
    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT:
            Manager::Get()->GetLogManager()->Log(wxT("OnSocketEvent: Input"));
            pSock->Read(buf, sizeof(buf));
            Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("OnSocketEvent: Read %d: %s"), pSock->LastCount(), wxString::FromAscii(buf).c_str()));
            HandleMessages(wxString::FromAscii(buf));
            break;
        // The server hangs up after sending the data
        case wxSOCKET_LOST:
            Manager::Get()->GetLogManager()->Log(wxT("OnSocketEvent: Closed"));
            pSock->Destroy();
            break;

    }
}


void stClient::SendPing()
{
    SendMessageToSourceTrail(wxT("ping>>CodeBlocks<EOM>"));
}

void stClient::SendLocation()
{

    //get current active editor
    cbEditor* pEditor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(pEditor)
    {
        cbStyledTextCtrl* pControl = pEditor->GetControl();
        wxString sMessage;
        int nPosition = pControl->GetCurrentPos();
        int nLine = pControl->GetCurrentLine();
        wxString sFilename = pEditor->GetFilename();

        sFilename.Replace(wxT("\\"), wxT("/"));

        sMessage.Printf(wxT("setActiveToken>>%s>>%d>>%d<EOM>"), sFilename.c_str(), nLine, pControl->GetColumn(nPosition));
        SendMessageToSourceTrail(sMessage);
    }
}
