// Interface
class SocketMgr
{
	public:
		SocketMgr();
		virtual ~SocketMgr();

		void Uninitialise();

	protected:
		virtual void ListenThread() = 0;
		virtual void WorkerThread() = 0;
	
		// Must be called after child class construction.
		void startThreads()
		{
			m_threadWorker = std::thread(&SocketMgr::WorkerThread, this);
			m_threadListener = std::thread(&SocketMgr::ListenThread, this);
			m_bInit = true;
		}

		void setListenSocket(SOCKET sSocket)
		{
			ASSERT(m_sListenSocket == 0);
			m_sListenSocket = sSocket;
		}
		
		bool doCancelWorker() const { return m_bCancelWorker; }
		bool doCancelListener() const { return m_bCancelListener; }

	private:	
		bool m_bCancelWorker;
		bool m_bCancelListener;
		bool m_bInit;

		SOCKET m_sListenSocket;

		std::thread m_threadWorker;
		std::thread m_threadListener;

		WSAData m_wsaData;
};

// Example usage
class ExampleSocketServer : public SocketMgr
{
	public:
		ExampleSocketServer()
		{
			SocketMgr::startThreads();
		}		

	public:
		class ClientSocket
		{
			ClientSocket(const SOCKET socket, const std::string addr)
			{
				// Client connected!
				// This object will be destroyed when the socket is no longer connected
			}
			
			bool IsClosed() const { return false; }
		};
		
	private:
		void ListenThread() final
		{
			TcpListener listener(sWorld->getConfig(CONFIG_PORT_WORLD));
			SocketMgr::setListenSocket(listener.mySocket());

			while (!doCancelListener())
			{
				std::string addr;
				SOCKET sSocket = listener.waitAndAccept(addr, sConfig->GetBoolDefault("Network.TcpNodelay", true), WORLDSOCKET_DEFAULT_TIMEOUT_NO_RECV);

				if (sSocket == SOCKET_ERROR)
				{
					// Don't race.
					std::this_thread::sleep_for(std::chrono::microseconds(1));
					continue;
				}

				std::lock_guard<std::mutex> lock(m_mutexSocketList);
				PurgeEndingSockets();
				m_vActiveSockets.push_back(std::make_shared<ClientSocket>(sSocket, addr));
			}
		}

		void WorkerThread() final
		{
			while (!doCancelWorker())
			{
				{
					std::lock_guard<std::mutex> lock(m_mutexSocketList);
					PurgeEndingSockets();
				}

				// Don't race.
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}
		}
		
		void PurgeEndingSockets
		{
			auto itr = m_vActiveSockets.begin();

			while (itr != m_vActiveSockets.end())
			{
				WorldSocket& worldSocket = *(*itr);

				if (worldSocket.IsClosed())
				{
					itr = m_vActiveSockets.erase(itr);
				}
				else
				{
					++itr;
				}
			}
		}

		std::mutex m_mutexSocketList;
		std::vector<std::shared_ptr<WorldSocket>> m_vActiveSockets;
};