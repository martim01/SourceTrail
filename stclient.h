#pragma once
#include <wx/socket.h>
#include <wx/string.h>
#include <wx/event.h>

class stClient : public wxEvtHandler
{
    public:
        stClient(unsigned int nServerPort, unsigned int nWritePort);
        ~stClient();
        bool StartServer();
        bool SendMessageToSourceTrail(const wxString& sMessage);
        void SendLocation();
    private:
        void OnSocketEvent(wxSocketEvent& event);
        void OnServerSocketEvent(wxSocketEvent& event);
        void HandleMessages(const wxString& sBuffer);
        void DecodeMessage(wxString sMessage);

        void MoveCursor(const wxString& sFile, unsigned long nLine, unsigned long nCol);

        void SendPing();


        wxSocketServer* m_pServer;

        bool m_bConnected;
        wxString m_sMessageBuffer;
        unsigned int m_nServerPort;
        unsigned int m_nWritePort;

        static const wxString MSG_END;
        static const wxString MSG_PING;
        static const wxString MSG_CURSOR;
        static const wxString MSG_CREATE_CDB;
};

