#include "test.h"

#define BUF_SIZE 1000000 // 1MB
#define READ	3
#define	WRITE	5
#define WEBSOCKET_HANDSHAKE_NOT_END		-1
#define WEBSOCKET_HANDSHAKE_END			0
#define WEBSOCKET_HANDSHAKE_SIZE			5
#define WEBSOCKET_HANDSHAKE_STR_SIZE		150
#define WEBSOCKET_HANDSHAKE_KEY_POS			3

#define OUTPUT_BUFFER_SIZE 1000000 // 1MB

typedef struct    // socket info
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct    // buffer info
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char* buffer;
	char* buffer_ptr;
	int rwMode;    // READ or WRITE
	int WebSocket_Info; // HandShake OPCode, Downloading WebSocket Payload Data index
	INT64 payload_len;
	UINT8 WebSocket_opcode;
} PER_IO_DATA, * LPPER_IO_DATA;

typedef struct _node
{
	LPPER_HANDLE_DATA _handleInfo;
	LPPER_IO_DATA _ioInfo;
	struct _node* next;
	struct _node* prev;
} Node;

typedef struct _list
{
	Node* head;
	Node* tail;
	Node* cur;
	UINT numOfData;
}List;

void List_init(List* plist)
{
	plist->head = (Node*)malloc(sizeof(Node));
	plist->tail = (Node*)malloc(sizeof(Node));

	plist->head->next = plist->tail;
	plist->head->prev = plist->tail;
	plist->tail->next = plist->head;
	plist->tail->prev = plist->head;
	plist->cur = plist->head->next->next;

	plist->numOfData = 0;
}

void List_Insert(List* plist, LPPER_HANDLE_DATA _phandleInfo, LPPER_IO_DATA _pioInfo)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->_handleInfo = _phandleInfo;
	newNode->_ioInfo = _pioInfo;

	newNode->next = plist->tail;
	newNode->prev = plist->tail->prev;

	plist->tail->prev->next = newNode;
	plist->tail->prev = newNode;

	plist->numOfData++;
}

void List_Pop(List* plist, LPPER_HANDLE_DATA _phandleInfo, LPPER_IO_DATA _pioInfo)
{
	Node* handleInfo_target = plist->head->next;
	Node* ioInfo_target = plist->head->next;
	//plist->cur = plist->head->next;

	while (plist->head != plist->tail->prev && handleInfo_target->_handleInfo != _phandleInfo)
		handleInfo_target = handleInfo_target->next;
	while (plist->head != plist->tail->prev && ioInfo_target->_ioInfo != _pioInfo)
		ioInfo_target = ioInfo_target->next;
	if (handleInfo_target->_handleInfo == _phandleInfo)
	{
		handleInfo_target->prev->next = handleInfo_target->next;
		handleInfo_target->next->prev = handleInfo_target->prev;
		free(_phandleInfo);
	}
	if (ioInfo_target->_ioInfo == _pioInfo)
	{
		ioInfo_target->prev->next = ioInfo_target->next;
		ioInfo_target->next->prev = ioInfo_target->prev;
		free(_pioInfo->buffer);
		free(_pioInfo);
	}
	plist->numOfData--;
}

void free_List(List* plist)
{
	Node* target = plist->head->next;

	while (target != plist->tail)
	{
		target->prev->next = target->next;
		target->next->prev = target->prev;
		free(target->_handleInfo);
		free(target->_ioInfo->buffer);
		free(target->_ioInfo);
		free(target);
		target = target->next;
	}

	free(plist->head);
	free(plist->tail);
	plist->numOfData = 0;

}
List L;

const char WebSocket_Connection[WEBSOCKET_HANDSHAKE_SIZE][WEBSOCKET_HANDSHAKE_STR_SIZE] = {
"HTTP/1.1 101 Switching Protocols\r\n",
"Upgrade: websocket\r\n",
"Connection: Upgrade\r\n",
"Sec-WebSocket-Accept: ",
"\r\n"
};


void numTobyte(unsigned char* ptr, int value, int dst_index, int byte_size)
{
	int i;
	for (i = 0; i < byte_size; i++)
	{
		ptr[dst_index + byte_size - (i + 1)] = ((value >> (8 * i)) & 0xff);
	}
}

UINT64 byteTonum(unsigned char* ptr, int dst_index, int byte_size)
{
	int i;
	UINT64 num;
	for (i = 0, num = 0; i < byte_size; i++)
		num |= ptr[dst_index + byte_size - (i + 1)] << (8 * i);
	return num;
}

SOCKET m_hServSock;
UINT RebootingCount = 0;
char isRebooting = 0;
void shutdownWAS()
{
	//printf("WAS를 Shutdown합니다\n");
	closesocket(m_hServSock);
	m_hServSock = -1;
	isRebooting = 1;
}

unsigned int __stdcall EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(const char* message);

UINT __count = 0;
void end_process(void)
{
	if (m_hServSock != -1)
		closesocket(m_hServSock);
	WSACleanup();
	free(AdapterDescription_Ptr);
	free(ip);
}

HANDLE hComPort;
void   OnTest1()
{
	WSADATA	wsaData;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAdr;
	int recvBytes, i, flags = 0;

	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
	int addrLen = sizeof(clntAdr); char myIPADDR[100];

	if (__count++ == 0)
		atexit(end_process);

	//select_Adapter(Interface_name, 0/*strnlen_s(Interface_name, sizeof(Interface_name))*/);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	if (RebootingCount++ == 0)
	{
		hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		GetSystemInfo(&sysInfo);

		for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
			_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

		atexit(end_process);
	}

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(3001);

	m_hServSock = hServSock;

	bind(hServSock, (SOCKADDR*)& servAdr, sizeof(servAdr));
	listen(hServSock, 1000);

	List_init(&L);
	isRebooting = 0;

	while (1)
	{
		hClntSock = accept(hServSock, (SOCKADDR*)& clntAdr, &addrLen);
		if (isRebooting == 1)
		{
			//printf("Shutdown상태에 들어갑니다.\n");
			if (hClntSock != INVALID_SOCKET)
			{
				closesocket(hServSock);
			}

			break;
		}


		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// 클라이언트마다 저장공간을 각각 할당해주는 부분
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->buffer = (char*)malloc(BUF_SIZE);
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->buffer_ptr = ioInfo->wsaBuf.buf;
		ioInfo->rwMode = READ;
		ioInfo->WebSocket_Info = WEBSOCKET_HANDSHAKE_NOT_END;
		ioInfo->payload_len = -1; // there is no payload
		List_Insert(&L, handleInfo, ioInfo);

		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf),
			1, (LPDWORD)& recvBytes, (LPDWORD)& flags, &(ioInfo->overlapped), NULL);

	}

	// *************************************************************************************************************************************************
	// 해야될 작업
	// 1. C#에서 Shutdown 시그널을 보냈을 때 해야될 처리들
	// --> malloc으로 메모리할당받은 것 정리, 쓰레드 회수
	// 2. WinPCap특성(절전모드이면 할당받은 포인터가 free되는 것)해결하기
	// --> 선택한 Interface이름을 찾고, 기존에 조회했던 인터페이스와 똑같다는 조건이 성립했다면 포인터를 재할당받기
	// 3. loopback으로 동작상태 모니터링하는 것
	// --> WebSocket OPCode 2(UTF-8), unmasked payload data로 체크하기 + 이 문장을 추가한다면 send후 연결종료(closesocket)
	// --> 메모리 문제 : c#에서 connect를 "127.0.0.1"으로 접속 --> C/C++에서는 클라이언트를 "127.0.0.1"으로 인식했음.
	// --> 반복적으로 연결하고 끊는 이유 : connection이 이루어지지 않은 상황까지 관찰하기 위해서임, 과부하는 timer의 시간을 늘려서 조절하면됨
	// 4. atexit함수 사용하기
	// *************************************************************************************************************************************************
	isRebooting = 1;
	free_List(&L);

	printf("WAS서버가 Shutdown이 되었습니다.\n");
	return;
}

unsigned int __stdcall EchoThreadMain(LPVOID pComPort)
{
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	Websocket_encoding_data value;
	int i, k, len, encoding_len;
	char http_message[16];
	char encoding_value[WEBSOCKET_ENCODING_OUTPUT_DATA_LENGTH];
	UINT64 data;
	char Websocket_masked[4];

	while (1)
	{
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
			(LPDWORD)& handleInfo, (LPOVERLAPPED*)& ioInfo, INFINITE);
		sock = handleInfo->hClntSock;

		if (isRebooting == 1)
		{
			closesocket(sock);
		}

		if (ioInfo->rwMode == READ)
		{
			//puts("message received!");
			if (bytesTrans == 0/*EOF 전송 시*/)
			{
				//printf("closed client\n");
				closesocket(sock);
				List_Pop(&L, handleInfo, ioInfo);
				//free(handleInfo); free(ioInfo);
				continue;
			}
			else if (strcmp("127.0.0.1", inet_ntoa(handleInfo->clntAdr.sin_addr)) == 0 && ioInfo->buffer[0] == 1 && ioInfo->buffer[1] == 0 && ioInfo->buffer[2] == 1 && ioInfo->buffer[3] == 1)
			{
				send(sock, "PingPong", 8, 0);
				List_Pop(&L, handleInfo, ioInfo);
				closesocket(sock);

				continue;
			}
			else
			{
				if ((char)(ioInfo->buffer[0] & 0x0f) == 8) //opcode : Connection Close
				{
					//printf("closed client\n");
					closesocket(sock);
					List_Pop(&L, handleInfo, ioInfo);
					//free(handleInfo); free(ioInfo);
					continue;
				}
			}

			ioInfo->buffer[bytesTrans] = '\0';

			if (ioInfo->WebSocket_Info == WEBSOCKET_HANDSHAKE_NOT_END)
			{
				for (i = 0; i < 14; i++)
					http_message[i] = ioInfo->buffer[i];
				http_message[i] = '\0';
				if (strcmp(http_message, "GET / HTTP/1.1") == 0)
				{
					// Let's HandShake
					strcpy_s(encoding_value, sizeof(encoding_value), getWebsocketKey(ioInfo->buffer));
					encoding_len = WebSocket_Encoding_For_HandShake(encoding_value, encoding_value);

					for (i = 1, strcpy_s(ioInfo->buffer, BUF_SIZE, (char*)WebSocket_Connection), len = strlen(ioInfo->buffer); i < WEBSOCKET_HANDSHAKE_SIZE; i++)
					{
						if (i == WEBSOCKET_HANDSHAKE_KEY_POS)
						{
							strcpy_s(&ioInfo->buffer[len], BUF_SIZE - len, WebSocket_Connection[i]);
							len += strlen(WebSocket_Connection[i]);
							strcat_s(&ioInfo->buffer[len], BUF_SIZE - len, encoding_value);
							len += encoding_len;
							strcat_s(&ioInfo->buffer[len], BUF_SIZE - len, (char*)"\r\n");
							len += 2;
						}
						else
						{
							strcat_s(&ioInfo->buffer[strlen(ioInfo->buffer)], BUF_SIZE - len, WebSocket_Connection[i]);
							len += strlen(WebSocket_Connection[i]);
						}
					}

					memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
					ioInfo->wsaBuf.len = strlen(ioInfo->buffer);
					ioInfo->rwMode = WRITE;

					WSASend(sock, &(ioInfo->wsaBuf),
						1, NULL, 0, &(ioInfo->overlapped), NULL);

				}
				else
				{
					closesocket(sock);
					List_Pop(&L, handleInfo, ioInfo);
					//free(handleInfo); free(ioInfo);

				}
			}
			else //if (ioInfo->WebSocket_Info == WEBSOCKET_HANDSHAKE_END)
			{
				if (ioInfo->WebSocket_Info == 0)
				{
					// Check whether it is extended payload length and calculate the total payload length.
					switch ((UINT8)(ioInfo->buffer[0] & 0x0f))
					{
					case 1: // opcode : text
					case 2: // opcde : binary(UTF-8)
						ioInfo->WebSocket_opcode = (UINT8)(ioInfo->buffer[0] & 0x0f);
						// Calculating WebSocket Payload Length and Ready to get all of last data
						switch ((char)(ioInfo->buffer[1] & 0x7f))
						{
						case 126: // Extended Payload Length (2byte), the payload length is under 18,000,000bytes
							ioInfo->payload_len = (UINT64)((UINT16)((ioInfo->buffer[2] << 8) | (unsigned char)(ioInfo->buffer[3])));
							break;
						case 127: // Extended Payload Length (8byte), the payload length is under 14,075bytes
							for (k = 0, ioInfo->payload_len = 0; k < 8; k++)
							{
								data = (unsigned char)ioInfo->buffer[2 + k];
								ioInfo->payload_len |= data << 8 * (7 - k);
							}
							break;
						default: // the payload length is under 60bytes
							ioInfo->payload_len = (UINT64)(ioInfo->buffer[1] & 0x7f);
							break;
						}
					}
					ioInfo->WebSocket_Info = bytesTrans; // current payload index
				}


				if (ioInfo->payload_len > ioInfo->WebSocket_Info/*current payload index*/)
				{
					// finalize to get all of last data
					memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
					ioInfo->WebSocket_Info += bytesTrans;
					ioInfo->wsaBuf.buf = ioInfo->buffer + ioInfo->WebSocket_Info;
					ioInfo->wsaBuf.len = ioInfo->payload_len - ioInfo->WebSocket_Info;
					ioInfo->rwMode = READ;
					WSARecv(sock, &(ioInfo->wsaBuf),
						1, NULL, &flags, &(ioInfo->overlapped), NULL);

					continue;
				}
				else
				{
					// unpacking payload data(unmasking if M bit is true)
					ioInfo->wsaBuf.buf = ioInfo->buffer_ptr;  // 차라리 const char *값을 구조체에 추가해주어서 원래의 주소로 되돌려주면 경계오류가 뜨는 문제점을 해결할 수 있음.
					switch ((unsigned char)(ioInfo->buffer[1] & 0x7f))
					{
					case 126: // Extended Payload Length (2byte)
						if ((unsigned char)(ioInfo->buffer[1] & 0x80)) // Masked, 클라이언트는 마스킹을 무조건 한다고 MDN 문서에 적혀져있음.
						{
							for (k = 0; k < 4; k++)
								Websocket_masked[k] = ioInfo->buffer[k + 4];
							for (k = 0; k < ioInfo->payload_len; k++)
							{
								ioInfo->wsaBuf.buf[k] = (ioInfo->buffer[k + 8] ^ Websocket_masked[k % 4]);
							} ioInfo->wsaBuf.buf[k] = '\0';

						}
						else
						{
							for (k = 0; k < ioInfo->payload_len; k++)
								ioInfo->wsaBuf.buf[k] = ioInfo->buffer[k + 4]; ioInfo->wsaBuf.buf[k] = '\0';

						}
						break;
					case 127:
						if ((unsigned char)(ioInfo->buffer[1] & 0x80)) // Masked, 클라이언트는 마스킹을 무조건 한다고 MDN 문서에 적혀져있음.
						{
							for (k = 0; k < 4; k++)
								Websocket_masked[k] = ioInfo->buffer[k + 10];

							for (k = 0; k < ioInfo->payload_len; k++)
							{
								ioInfo->wsaBuf.buf[k] = (ioInfo->buffer[k + 14] ^ Websocket_masked[k % 4]);
							} ioInfo->wsaBuf.buf[k] = '\0';

						}
						else
						{
							for (k = 0; k < ioInfo->payload_len; k++)
								ioInfo->wsaBuf.buf[k] = ioInfo->buffer[k + 10]; ioInfo->wsaBuf.buf[k] = '\0';

						}

						break;
					default:
						if ((unsigned char)(ioInfo->buffer[1] & 0x80)) // Masked, 클라이언트는 마스킹을 무조건 한다고 MDN 문서에 적혀져있음.
						{
							for (k = 0; k < 4; k++)
								Websocket_masked[k] = ioInfo->buffer[k + 2];

							for (k = 0; k < ioInfo->payload_len; k++)
							{
								ioInfo->wsaBuf.buf[k] = (ioInfo->buffer[k + 6] ^ Websocket_masked[k % 4]);
							} ioInfo->wsaBuf.buf[k] = '\0';

						}
						else
						{
							for (k = 0; k < ioInfo->payload_len; k++)
								ioInfo->wsaBuf.buf[k] = ioInfo->buffer[k + 2]; ioInfo->wsaBuf.buf[k] = '\0';

						}

						break;
					}

					// ********************************************************************************************************************************************************************************
					// comleted to get all of payload data, You jus write down here whatever you want anything with read data - (message ptr : ioInfo->wsaBuf.buf, message len : ioInfo->payload_len)
					//ioInfo->wsaBuf.buf[ioInfo->payload_len] = '\0';
					//printf("len : %lld, message : %s\n", ioInfo->payload_len, ioInfo->wsaBuf.buf);

					// How to Send packet :
					// you just access buffer 'ioInfo->wsaBuf.buf' and input the length you will send the packet into ioInfo->payload_len
					// Example
					// strcpy(ioInfo->wsaBuf.buf,"Hello World");
					// ioInfo->payload_len = strlen(ioInfo->wsaBuf.buf);

					//printf("payload len: %lld\n", ioInfo->payload_len);
					//for (i = 0; i < ioInfo->payload_len; i++)
					//	printf("%x ", (unsigned char)ioInfo->wsaBuf.buf[i]); printf("\n");

					/**************************************************************************************************
					**   실제 패킷을 전송하는 부분                                                                 **
					**************************************************************************************************/
					/* Send down the packet */
					if (pcap_sendpacket(Mypcap_handler, (const u_char*)ioInfo->wsaBuf.buf, ioInfo->payload_len /* size */) != 0)
					{
						// 포인터 재할당해보고 또다시 오류가 나면 서버관리자에게 재실행볼것을 알리기
						 // ....하흥..Linked List사용해야됨...
						// Pop()이 필요함, insert()는 head나 tail중에 어디에 삽입하든 상관없음

						if (pcap_sendpacket(Mypcap_handler, (const u_char*)ioInfo->wsaBuf.buf, ioInfo->payload_len /* size */) != 0)
						{
							select_Adapter(Interface_name, strnlen_s(Interface_name, sizeof(Interface_name)));
							if (pcap_sendpacket(Mypcap_handler, (const u_char*)ioInfo->wsaBuf.buf, ioInfo->payload_len /* size */) != 0)
								sprintf_s(ioInfo->wsaBuf.buf, BUF_SIZE, "Error sending the packet: %s\n", pcap_geterr(Mypcap_handler));
						}
						// ErrorHandling("Error sending the packet");
					}
					else
					{
						sprintf_s(ioInfo->wsaBuf.buf, BUF_SIZE, "Generated the packet\n");
					}
					ioInfo->payload_len = strlen(ioInfo->wsaBuf.buf);

					// ********************************************************************************************************************************************************************************
					if (ioInfo->payload_len > 0)
					{
						if (ioInfo->payload_len > 65535)
						{
							for (i = ioInfo->payload_len - 1; i > -1; i--) // shift data
							{
								ioInfo->wsaBuf.buf[i + 10] = ioInfo->wsaBuf.buf[i];
							}ioInfo->wsaBuf.buf[ioInfo->payload_len + 10] = '\0';
							ioInfo->wsaBuf.buf[0] = (0x80 | ioInfo->WebSocket_opcode/*FIN bit opcode : followed the receive payload*/);
							ioInfo->wsaBuf.buf[1] = 127;// masked : 0, range : extended payload length 65536+
							for (k = 0, data = ioInfo->payload_len; k < 8; k++)
							{
								ioInfo->wsaBuf.buf[9 - k] = data & 0xff;
								data >>= 8;
							}

							memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
							ioInfo->wsaBuf.len = ioInfo->payload_len + 10;
							ioInfo->rwMode = WRITE;
							WSASend(sock, &(ioInfo->wsaBuf),
								1, NULL, 0, &(ioInfo->overlapped), NULL);
						}
						else if (ioInfo->payload_len > 125)
						{
							for (i = ioInfo->payload_len - 1; i > -1; i--) // shift data
							{
								ioInfo->wsaBuf.buf[i + 4] = ioInfo->wsaBuf.buf[i];
							}ioInfo->wsaBuf.buf[ioInfo->payload_len + 4] = '\0';

							ioInfo->wsaBuf.buf[0] = (0x80 | ioInfo->WebSocket_opcode/*FIN bit opcode : followed the receive payload*/);
							ioInfo->wsaBuf.buf[1] = 126;// masked : 0, range : extended payload length 126~65535
							data = htonll(ioInfo->payload_len);
							data >>= 48;
							ioInfo->wsaBuf.buf[2] = data & 0xff;
							data >>= 8;
							ioInfo->wsaBuf.buf[3] = data & 0xff;


							memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
							ioInfo->wsaBuf.len = ioInfo->payload_len + 4;
							ioInfo->rwMode = WRITE;
							WSASend(sock, &(ioInfo->wsaBuf),
								1, NULL, 0, &(ioInfo->overlapped), NULL);
						}
						else
						{
							for (i = ioInfo->payload_len - 1; i > -1; i--) // shift data
							{
								ioInfo->wsaBuf.buf[i + 2] = ioInfo->wsaBuf.buf[i];
							}ioInfo->wsaBuf.buf[ioInfo->payload_len + 2] = '\0';


							ioInfo->wsaBuf.buf[0] = (0x80 | ioInfo->WebSocket_opcode/*FIN bit opcode : followed the receive payload*/);
							data = htonll(ioInfo->payload_len);
							data >>= 56;
							ioInfo->wsaBuf.buf[1] = data & 0xff;// masked : 0, range : 0~125


							memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
							ioInfo->wsaBuf.len = ioInfo->payload_len + 2;
							ioInfo->rwMode = WRITE;
							WSASend(sock, &(ioInfo->wsaBuf),
								1, NULL, 0, &(ioInfo->overlapped), NULL);
						}
					}
					else
					{
						// This block is for when the length of which will be sent payload is under 0.
						memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
						ioInfo->wsaBuf.len = BUF_SIZE;
						ioInfo->wsaBuf.buf = ioInfo->buffer;
						ioInfo->rwMode = READ;
						ioInfo->WebSocket_Info = 0;
						WSARecv(sock, &(ioInfo->wsaBuf),
							1, NULL, &flags, &(ioInfo->overlapped), NULL);
					}
				}

			}
		}
		else
		{

			if (ioInfo->WebSocket_Info == WEBSOCKET_HANDSHAKE_NOT_END)
			{
				ioInfo->WebSocket_Info = WEBSOCKET_HANDSHAKE_END; //printf("\n클라이언트가 연결되었습니다.\n");

			}
			else
			{
				// 서버는 요청(Recv)이후 전송 완료가 되면 트랜잭션 하나를 끝낸것이라고 생각함
				ioInfo->WebSocket_Info = 0;
				closesocket(handleInfo->hClntSock);
				List_Pop(&L, handleInfo, ioInfo);
			}

			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf),
				1, NULL, &flags, &(ioInfo->overlapped), NULL);
			//puts("message sent!");
			//free(ioInfo);
		}
	}//printf("Worker쓰레드 종료\n");
	return 0;
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
