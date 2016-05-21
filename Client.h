// @bNoDelay Will setsockopt TCP_NODELAY, disabling Nagle's algorithm (increased speed, more traffic)
// @timeout  If set a value it's the number of milliseconds to add onto 500 minimum for Windows to SO_RCVTIMEO. 
static SOCKET CreateSocketAndConnect(const std::string ipAddress, const uint32 port, const bool bNoDelay = false, const uint32 timeout = 0)
{
	// Create socket
	SOCKET sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sSocket == INVALID_SOCKET)
	{
		printf("Util::createSocketAndConnect socket() failed: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	// Prepare address in form of sockaddr_in
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	// Try to resolve host name if need to
	if (serverAddr.sin_addr.s_addr == INADDR_NONE)
	{
		struct hostent* host = gethostbyname(ipAddress.c_str());

		if (!host) 
		{
			printf("Util::createSocketAndConnect failed to resolve hostname\n");
			return INVALID_SOCKET;
		}

		CopyMemory(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);
	}

	if (bNoDelay)
	{
		// Will disable Nagle's algorithm (increased speed, more traffic)
		int32 i = 1;
		setsockopt(sSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(i));
	}

	if (timeout)
	{			
		int32 iTimeout = timeout;
		setsockopt(sSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iTimeout, sizeof(iTimeout));
	}

	// Connect	
	if (connect(sSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf("Util::createSocketAndConnect connect() failed: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	return sSocket;
}