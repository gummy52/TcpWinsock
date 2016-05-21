/*
## Server
##	- TcpListener class
## Client
##  - CreateSocketAndConnect function
*/

class TcpListener
{
	public:
		TcpListener(const uint32 port) :
			m_sListenSocket(INVALID_SOCKET)
		{
			struct sockaddr_in local;

			// Create our listening socket
			m_sListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

			if (m_sListenSocket == INVALID_SOCKET)
			{
				printf("TcpListener socket() failed: %d\n", WSAGetLastError());
				return;
			}

			local.sin_addr.s_addr = htonl(INADDR_ANY);
			local.sin_family = AF_INET;
			local.sin_port = htons(port);

			// Bind
			if (::bind(m_sListenSocket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
			{
				printf("TcpListener bind() failed: %d\n", WSAGetLastError());
				m_sListenSocket = INVALID_SOCKET;
				return;
			}

			if (::listen(m_sListenSocket, SOMAXCONN) == SOCKET_ERROR)
			{
				printf("TcpListener listen() failed: %d\n", WSAGetLastError());
				m_sListenSocket = INVALID_SOCKET;
				return;
			}
		}

		~TcpListener()
		{
			if (m_sListenSocket && m_sListenSocket != INVALID_SOCKET)
				closesocket(m_sListenSocket);
		}
			
		bool okay() const 
		{ 
			return m_sListenSocket > 0;
		}

		SOCKET waitAndAccept(std::string& addr, const bool bNoDelay = false)
		{
			if (!okay())
			{
				printf("TcpListener::waitAndAccept you can not do this on an invalid socket.\n");
				return INVALID_SOCKET;
			}

			sockaddr_in client;
			int32 addrsize = sizeof(client);

			// WinSOck ::accept
			SOCKET csock = ::accept(m_sListenSocket, (struct sockaddr*)&client, &addrsize);
			
			if (csock == SOCKET_ERROR || csock == INVALID_SOCKET)
			{			
				printf("TcpListener accept() failed: %d\n", WSAGetLastError());
				cleanup();
				return SOCKET_ERROR;
			}

			// TCP_NODELAY, optional
			if (bNoDelay)
			{
				int32 i = 1;
				setsockopt(csock, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(i));
			}

			addr = inet_ntoa(client.sin_addr);
			return csock;
		}

		SOCKET mySocket() const { return m_sListenSocket; }

	private:
		void cleanup()
		{				
			closesocket(m_sListenSocket);
			m_sListenSocket = INVALID_SOCKET;
		}

		SOCKET m_sListenSocket;
};