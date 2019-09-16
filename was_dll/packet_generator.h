#pragma once


#include <pcap.h> // this header is based on c++, so you must not use c
#include <Windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment (lib, "wpcap.lib")
#pragma comment (lib, "Iphlpapi.lib")

#define ETHER_ADDR_LEN			6   
#define ETHERTYPE_IP            0x0800   /* IP Protocol */   
#define ETHERTYPE_ARP           0x0806   /* Address Resolution Protocol */   
#define ETHERTYPE_REVARP        0x8035   /* reverse Address Resolution Protocol */   

#define ETHERNET_TYPE_INDEX_H				12
#define ETHERNET_TYPE_INDEX_L				13
#define IP_PROTOCOL_TYPE_INDEX				23
#define ARP_OPCODE_TYPE_INDEX_H				20
#define ARP_OPCODE_TYPE_INDEX_L				21
#define IP_ADDITIONAL_FISRT_INDEX			34

#define PACKET_SIZE							16000
#define DATA_BUFFER_SIZE					10
#define ARP_PACKET_SCANNING_NUMBER 3000


typedef struct ether_header {
	unsigned char   ether_dhost[ETHER_ADDR_LEN];
	unsigned char   ether_shost[ETHER_ADDR_LEN];
	unsigned short  ether_type;
} ETHER_HDR;
/* IP Header Structure */
typedef struct ip_hdr
{
	unsigned char  version_hlen; //the unit of hlen is 4bytes.so if the total len of header is 20, then you just divide 20 by 4.
	unsigned char  ip_tos;           // IP type of service   
	unsigned short ip_total_length;  // Total length   

	unsigned short ip_id;            // Unique identifier    
	/*unsigned char  ip_more_fragment;
	unsigned char  ip_dont_fragment;
	unsigned char  ip_reserved_zero;
	unsigned char  ip_frag_offset;    //fragment offset
	*/
	unsigned short ip_fragment;

	unsigned char  ip_ttl;           // Time to live   
	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)   
	unsigned short ip_checksum;      // IP checksum   

	unsigned int   ip_srcaddr;       // Source address   

	unsigned int   ip_destaddr;      // Source address   
} IP_HDR;


typedef struct arp_hdr
{
	unsigned short hardware_type;//2
	unsigned short protocol_type;//2

	unsigned char hardware_len;//1
	unsigned char protocol_len;//1
	unsigned short arp_opcode;//2

	//because of the structure of an allocation of memory, the addresses are made for array.
	unsigned char set_address[20];//i designed the size is 20, but i do not know the others is exactly.
}ARP_HDR;

typedef struct icmp_hdr
{
	unsigned char type;
	unsigned char code;
	unsigned short checksum;
	unsigned short identification;
	unsigned short sequence_number;
}ICMP_HDR;

typedef struct receive_packet
{
	ETHER_HDR eth;
	IP_HDR ip;
	ARP_HDR arp;
	ICMP_HDR icmp;
	unsigned char data_buffer[DATA_BUFFER_SIZE];
	unsigned int packet_type;
	pcap_pkthdr received_Info;
}RECEIVE_PACKET;

enum networkClass
{
	Class_A = 1, Class_B, Class_C, Class_D, Class_E
};

UINT8 Get_myAdaptersInfo(char* pcap_description, PIP_ADAPTER_INFO dst)
{
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	DWORD dwRetVal = 0;
	PIP_ADAPTER_INFO pAdapter;
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			if (strcmp(pcap_description, pAdapter->AdapterName) == 0)
			{
				memcpy(dst, pAdapter, sizeof(IP_ADAPTER_INFO));
				free(pAdapterInfo);
				return 1;
			}
			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		fprintf(stderr, "GetAdaptersInfo failed with error: %d\n", dwRetVal);

	}

	free(pAdapterInfo);
	return 0;
}


IP_ADAPTER_INFO selectedAdapter;
pcap_t* Mypcap_handler;
char Interface_name[150] = { 0 };

char** AdapterDescription_Ptr = NULL;
int* ip;
UINT AdaptionDescription_num = 0;
extern "C" __declspec(dllexport) char* _GetAdapterDescription(int* size, UINT index/*0 : reallocate, >0 : index pointer*/, int* _ip)
{
	IP_ADAPTER_INFO pAdapter_ptr;
	pcap_if_t* alldevs;
	pcap_if_t* d;
	char errbuf[PCAP_ERRBUF_SIZE];
	int i = 0, k;
	char* ptr;

	*_ip = 0;

	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return NULL;
	}

	if (index < 1)
	{
		if (AdaptionDescription_num > 0)
		{
			int k;
			for (k = 0; k < AdaptionDescription_num; k++)
				free(AdapterDescription_Ptr[k]);
			free(AdapterDescription_Ptr);
			free(ip);
			AdaptionDescription_num = 0;
		}

		AdapterDescription_Ptr = (char**)malloc(sizeof(char*) * 30);
		ip = (int*)malloc(sizeof(int) * 30);

		for (i = 0, k = 0, AdaptionDescription_num = 0, d = alldevs; d; d = d->next, i++)
		{
			if (Get_myAdaptersInfo(&d->name[12], &pAdapter_ptr) && strcmp(pAdapter_ptr.IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				AdapterDescription_Ptr[k] = (char*)malloc(strlen(pAdapter_ptr.Description) + 1);
				strcpy_s(AdapterDescription_Ptr[k], strlen(pAdapter_ptr.Description) + 1, pAdapter_ptr.Description);
				inet_pton(AF_INET, pAdapter_ptr.IpAddressList.IpAddress.String, &ip[k]);// printf("C++에서 출력하는 IP : %0000000x\n", ip[k]);
				*_ip = ip[k];

				AdaptionDescription_num++;
				k++;
			}
		}

		index = 1;
	}

	ptr = (char*)LocalAlloc(LPTR, strlen(AdapterDescription_Ptr[--index]) + 1); // 인덱스 1부터 복사해주는 부분
	strcpy_s(ptr, strlen(AdapterDescription_Ptr[index]) + 1, AdapterDescription_Ptr[index]);
	*_ip = ip[index];// printf("\tC++에서 출력하는 IP : %0000000x\n", ip[index]);
	LocalFree(ptr);

	if (alldevs)
		pcap_freealldevs(alldevs);

	*size = AdaptionDescription_num;

	return ptr;
}
extern "C" __declspec(dllexport) int _SetAdapter(char ptr[133])
{

	IP_ADAPTER_INFO pAdapter_ptr;
	pcap_if_t* alldevs;
	pcap_if_t* d;
	char errbuf[PCAP_ERRBUF_SIZE];
	int i = 0;
	int return_value = 0;


	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return -1;
	}

	for (i = 0, d = alldevs; d; d = d->next, i++)
	{
		if (Get_myAdaptersInfo(&d->name[12], &pAdapter_ptr) && strcmp(pAdapter_ptr.Description, ptr) == 0)
		{
			break;
		}
	}

	if (strcmp(pAdapter_ptr.Description, ptr) == 0)
	{
		/* Open the output device */
		if ((Mypcap_handler = pcap_open_live(d->name,            // name of the device   
			65536,                              // portion of the packet to capture (only the first 100 bytes)   
			0,  // promiscuous mode   
			1000,               // read timeout   
			errbuf              // error buffer   
		)) == NULL)
		{
			return_value = 0;
		}
		else
		{
			/**/for (i = 0; i < strlen(&d->name[12]) && i < sizeof(Interface_name); i++)
				Interface_name[i] = d->name[12 + i];
			Interface_name[i] = '\0';
			return_value = 1;
		}

	}

	if (alldevs)
		pcap_freealldevs(alldevs);

	return return_value;
}

INT8 select_Adapter(char* Interface_Name = NULL, UINT Interface_Name_buffer_size = 0)
{
	IP_ADAPTER_INFO pAdapter_ptr;
	pcap_if_t* alldevs;
	pcap_if_t* d;
	char errbuf[PCAP_ERRBUF_SIZE];
	int i = 0, interface_number, input_value;


	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		return -1;
	}

	if (Interface_Name_buffer_size == 0)
	{
		for (i = 0, d = alldevs; d; d = d->next, i++)
		{
			if (Get_myAdaptersInfo(&d->name[12], &pAdapter_ptr) && strcmp(pAdapter_ptr.IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				printf("%d. %s\n", i + 1, pAdapter_ptr.Description);

			}
		}


		if (i == 0)
		{
			printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
			return -1;
		}

		interface_number = i;
		if (interface_number > 1)
		{
			printf("Enter the interface number (1-%d):", interface_number);
			scanf_s("%d", &input_value);
		}
		else if (interface_number == 1)
			input_value = 1;


		while (input_value < 1 || input_value > i)
		{
			printf("\nInterface number out of range.\nEnter the interface number (1-%d):", interface_number);
			scanf_s("%d", &input_value);
		}

		/* Jump to the selected adapter */
		for (d = alldevs, i = 0; i < input_value - 1; d = d->next, i++);
		Get_myAdaptersInfo(&d->name[12], &pAdapter_ptr);
		memcpy(&selectedAdapter, &pAdapter_ptr, sizeof(selectedAdapter));


		/* Open the output device */
		if ((Mypcap_handler = pcap_open_live(d->name,            // name of the device   
			65536,                              // portion of the packet to capture (only the first 100 bytes)   
			0,  // promiscuous mode   
			1000,               // read timeout   
			errbuf              // error buffer   
		)) == NULL)
		{
			fprintf(stderr, "\nUnable to open the adapter\n");
			return -1;
		}
		else
		{
			for (i = 0; i < Interface_Name_buffer_size || i < strlen(&d->name[12]); i++)
				Interface_Name[i] = d->name[12 + i];
			Interface_Name[i] = '\0';
		}
	}
	else
	{
		for (i = 0, d = alldevs; d; d = d->next, i++)
		{
			if (strcmp(&d->name[12], Interface_Name) == 0)
			{
				/* Open the output device */
				if ((Mypcap_handler = pcap_open_live(d->name,            // name of the device   
					65536,                              // portion of the packet to capture (only the first 100 bytes)   
					0,  // promiscuous mode   
					1000,               // read timeout   
					errbuf              // error buffer   
				)) == NULL)
				{
					fprintf(stderr, "\nUnable to open the adapter\n");
					return -1;
				}
			}
		}

	}
	if (alldevs)
		pcap_freealldevs(alldevs);
}
