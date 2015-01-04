#include "Player.hpp"

int main(int argc, char** argv)
{
	// init kinect
	DWORD sensorFlags = NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON; //NUI_INITIALIZE_FLAG_USES_AUDIO
	//DWORD skeletonFlags = NULL; //NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE | NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT
	DWORD skeletonFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE | NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;

	Kinect kinect;
	kinect.init(sensorFlags, skeletonFlags);

	string dataDirPath = "resource/";
	printf("recording...\n\n");
	kinect.record(dataDirPath);

	/*string colorFilePath = "resource/color_1374669425";
	string depthFilePath = "resource/depth_1374669425";
	string skeletonFilePath = "resource/skeleton_1374669425";
	string faceFilePath = "resource/face_1374669425";
	
	printf("playing...\n\n");
	kinect.play(colorFilePath, depthFilePath, skeletonFilePath, faceFilePath);*/

	//// start server
	//WSADATA wsaData;
	//int iResult;
	//
	//SOCKET listenSocket = INVALID_SOCKET;
	//SOCKET clientSocket = INVALID_SOCKET;
	//
	//struct addrinfo* result = NULL;
	//struct addrinfo hints;
	//
	//// init Winsock
	//iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//if(iResult != 0)
	//{
	//	printf("WSAStartup() failed with error: %d\n", iResult);
	//	return 1;
	//}
	//
	//ZeroMemory(&hints, sizeof(hints));
	//hints.ai_family = AF_INET;
	//hints.ai_socktype = SOCK_STREAM;
	//hints.ai_protocol = IPPROTO_TCP;
	//hints.ai_flags = AI_PASSIVE;
	//
	//// resolve the server address and port
	//iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	//if(iResult != 0)
	//{
	//	printf("getaddrinfo() failed with error: %d\n", iResult);
	//	WSACleanup();
	//	return 1;
	//}
	//
	//// create a SOCKET for connecting to server
	//listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//if(listenSocket == INVALID_SOCKET)
	//{
	//	printf("socket() failed with error: %ld\n", WSAGetLastError());
	//	freeaddrinfo(result);
	//	WSACleanup();
	//	return 1;
	//}
	//
	//// setup the TCP listening socket
	/////// When you say "using namespace std;", you make all of the names in namespace std available to unqualified name lookup.
	/////// Therefore, both Winsock's bind() and <functional>'s bind() are considered during overload resolution, and because <functional>'s bind() is a template, 
	/////// it will usually win, but fail to compile later. To fix this, call ::bind() when you want Winsock's bind(), and std::bind() if you ever want <functional>'s bind().
	/////// When you call ::bind(), you're asking the compiler to look in the global namespace only.
	//iResult = ::bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	//if(iResult == SOCKET_ERROR)
	//{
	//	printf("bind() failed with error: %d\n", WSAGetLastError());
	//	freeaddrinfo(result);
	//	closesocket(listenSocket);
	//	WSACleanup();
	//	return 1;
	//}
	//
	//freeaddrinfo(result);
	//
	//iResult = listen(listenSocket, SOMAXCONN);
	//if(iResult == SOCKET_ERROR)
	//{
	//	printf("listen() failed with error: %d\n", WSAGetLastError());
	//	closesocket(listenSocket);
	//	WSACleanup();
	//	return 1;
	//}
	//else
	//	printf("RECOGNIZER: server started on _localhost:%s_\n\n\n", DEFAULT_PORT);
	//
	////closesocket(listenSocket);
	//
	//while(true)
	//{
	//	// accept a client socket
	//	clientSocket = accept(listenSocket, NULL, NULL);
	//	if(clientSocket == INVALID_SOCKET)
	//	{
	//		printf("accept() failed with error: %d\n", WSAGetLastError());
	//		closesocket(listenSocket);
	//		WSACleanup();
	//		return 1;
	//	}
	//
	//	char receivedBuf[512] = {'\0'};
	//	int receivedBufLen = DEFAULT_BUFLEN;
	//
	//	iResult = recv(clientSocket, receivedBuf, receivedBufLen, 0);
	//	if(iResult > 0)
	//	{
	//		string received(receivedBuf);
	//		printf("client sent: %s\n", received.c_str());
	//		
	//		string sent = "";

	//		if(received == "run")
	//		{	
	//			printf("running...\n\n");
	//			kinect.run();
	//
	//			sent = "stopped";
	//		}

	//		else if(received == "record")
	//		{
	//			string dataDirPath = "resource/";
	//
	//			printf("extracting...\n\n");
	//			kinect.record(dataDirPath);
	//
	//			sent = "stopped";
	//		}

	//		else if(received == "play")
	//		{
	//			string colorFilePath = "resource/color_0";
	//			string depthFilePath = "resource/depth_0";
	//			string skeletonFilePath = "resource/skeleton_0";
	//			string faceFilePath = "resource/face_0";
	//
	//			printf("playing...\n\n");
	//			kinect.play(colorFilePath, depthFilePath, skeletonFilePath, faceFilePath);
	//
	//			sent = "stopped";
	//		}

	//		else if(received == "stop")
	//		{
	//			printf("stopping...\n\n");
	//			sent = "exited";
	//
	//			char sentBuf[512] = {'\0'};
	//			int sentBufLen = DEFAULT_BUFLEN;
	//			memcpy(sentBuf, sent.c_str(), sent.size());
	//
	//			// send the buffer back to the client
	//			iResult = send(clientSocket, sentBuf, sentBufLen, 0);
	//			if(iResult == SOCKET_ERROR)
	//			{
	//				printf("send() failed with error: %d\n", WSAGetLastError());
	//				closesocket(clientSocket);
	//				WSACleanup();
	//
	//				return 1;
	//			}
	//
	//			break;
	//		}
	//
	//		else
	//		{
	//			printf("unknown command...\n\n");
	//			sent = "unknown";
	//		}
	//
	//		char sentBuf[512] = {'\0'};
	//		int sentBufLen = DEFAULT_BUFLEN;
	//		memcpy(sentBuf, sent.c_str(), sent.size());
	//
	//		// send the buffer back to the client
	//		iResult = send(clientSocket, sentBuf, sentBufLen, 0);
	//		if(iResult == SOCKET_ERROR)
	//		{
	//			printf("send() failed with error: %d\n", WSAGetLastError());
	//			closesocket(clientSocket);
	//			WSACleanup();
	//
	//			return 1;
	//		}
	//	}
	//
	//	else if(iResult == 0)
	//		continue;
	//
	//	else
	//	{
	//		printf("recv() failed with error: %d\n", WSAGetLastError());
	//		closesocket(clientSocket);
	//		WSACleanup();
	//
	//		return 1;
	//	}
	//}
	//
	//// stop server
	//iResult = shutdown(clientSocket, SD_SEND);
	//
	//if(iResult == SOCKET_ERROR)
	//{
	//	printf("shutdown() failed with error: %d\n", WSAGetLastError());
	//	closesocket(clientSocket);
	//	WSACleanup();
	//
	//	return 1;
	//}
	//
	//// clean
	//closesocket(clientSocket);
	//WSACleanup();

 //   return 18;
}