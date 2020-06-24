#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <fcntl.h>
#include <tchar.h>
#include <io.h>
#include "..\Dll\dll.h"

//Constantes
#define PIPE_NAME TEXT("\\\\.\\pipe\\connect")

//Variaveis
HANDLE cliente[MAX_NUM_PLAYERS];
HANDLE hPipe;
HANDLE thread_cliente;
HANDLE thread_read_msg_jogo;
BOOL login = FALSE;
int termina = 0;
dataCr memoriaPartilhadaGateway; 
MensagemJogo msgJogo;

static int id_user = 1;

/*Eventos*/
HANDLE id_evento_comeco;
HANDLE id_evento_memoria;

//Protótipos funções
DWORD WINAPI recebe_comando_cliente(LPVOID param);
DWORD WINAPI aceita_cliente(LPVOID param);
DWORD WINAPI leMsgJogo(void);
BOOL existePlayer();
void inicializaVectorClientes();
BOOL verifica_e_coloca_handle_pipe(HANDLE pipe);
void eliminaHandlePlayer(HANDLE aux);

//Funções
int _tmain(void) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	_tprintf(TEXT("Gateway Começou!\n"));

	//Abrir memoria para interpretrar comandos cliente
	if (!createSharedMemory(&memoriaPartilhadaGateway)) {
		_tprintf(TEXT("\Erro a criar memoria Partilhada!\n"));
		system("pause");
		exit(0);
	}


	//Inicializar clientes
	inicializaVectorClientes();
	thread_cliente = CreateThread(NULL, 0, aceita_cliente, NULL, 0, NULL);

	id_evento_memoria = OpenEvent(EVENT_ALL_ACCESS, TRUE, nomeEventoArrancaMemoria);
	id_evento_comeco = OpenEvent(EVENT_ALL_ACCESS, TRUE, nomeEventoComecoJogo);

	if (id_evento_memoria == NULL || id_evento_comeco == NULL) {
		_tprintf(TEXT("[ERRO] Eventos!\n"));
		system("pause");
		exit(-1);
	}
	
	SetEvent(id_evento_memoria);

	//Abrir memoria da parte do servidor
	openSharedMemoryJogo(&memoriaPartilhadaGateway);
	thread_read_msg_jogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)leMsgJogo, NULL, 0, NULL);


	HANDLE ioReady;
	OVERLAPPED ov;
	DWORD n;
	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	Sleep(1500);
	do {
		
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		
			if (cliente[i] != INVALID_HANDLE_VALUE) {
				ZeroMemory(&ov, sizeof(ov));
				ResetEvent(ioReady);
				ov.hEvent = ioReady;

				WriteFile(cliente[i], &msgJogo, sizeof(MensagemJogo), &n, &ov);

				WaitForSingleObject(ioReady, INFINITE);
				GetOverlappedResult(cliente[i], &ov, &n, FALSE);
			}
		
		}

		//_tprintf(TEXT("."));
	} while (existePlayer() || login == FALSE);

	//Processo de Fecho do Gateway
	_tprintf(TEXT("\nGateway Terminou!\n"));
	termina = 1;

	WaitForSingleObject(thread_read_msg_jogo, INFINITE);
	WaitForSingleObject(thread_cliente, INFINITE);
	
	CloseHandle(thread_cliente);
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		DisconnectNamedPipe(cliente[i]);
		CloseHandle(cliente[i]);
	}

	hPipe = CreateFile(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DisconnectNamedPipe(hPipe);

	CloseHandle(hPipe);
	UnmapViewOfFile(memoriaPartilhadaGateway.shared);
	exit(0);
}

DWORD WINAPI recebe_comando_cliente(LPVOID param) {
	HANDLE x = (HANDLE*)param;
	DWORD n;

	HANDLE ioReady;
	OVERLAPPED ov;
	COMANDO_SHARED aux;
	DWORD tam = 0;
	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);

//	_tprintf(TEXT("HANDLE: %d\n"), x);
	
	do {
		
		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;
	//	_tprintf(TEXT("handele dentro thread: %d\n"), x);
		ReadFile(x, &aux, sizeof(COMANDO_SHARED), &n, &ov);
		WaitForSingleObject(ioReady, INFINITE);

		GetOverlappedResult(hPipe, &ov, &n, FALSE);
		if (!n) {
			_tprintf(TEXT("[Escritor leu] %d bytes... (ReadFile)\n"), n);
			break;
		}

		_tprintf(TEXT("[ESCRITOR leu] Recebi %d bytes tipo %d: (ReadFile)\n"), n, aux.tipo);

		if (aux.tipo == CMD_LOGOUT) {
			_tprintf(TEXT("[ESCRITOR leu] Vou eliminar um jogador!\n"));
			eliminaHandlePlayer(x);
		}

		aux.idHandle = x;

	 writeMensagem(&memoriaPartilhadaGateway, &aux);
	
	 SetEvent(id_evento_comeco);

	} while (aux.tipo != CMD_LOGOUT);

	return 0;
}

DWORD WINAPI aceita_cliente(LPVOID param) {


	while (termina == 0) {
		_tprintf(TEXT("[ESCRITOR] Criar uma copia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);
		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 5, sizeof(COMANDO_SHARED) + sizeof(MensagemJogo) , sizeof(COMANDO_SHARED) + sizeof(MensagemJogo), 1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}
		_tprintf(TEXT("[ESCRITOR] Esperar liga��o de um leitor... (ConnectNamedPipe)\n"));

		if (!ConnectNamedPipe(hPipe, NULL)) {
			CloseHandle(hPipe);
			_tprintf(TEXT("[ERRO] Liga��o ao leitor! (ConnectNamedPipe\n"));
			exit(-1);
		}
		else {

			if (!verifica_e_coloca_handle_pipe(hPipe)) {
				DisconnectNamedPipe(hPipe);
			}
		}
		login = TRUE;
		_tprintf(TEXT("[ESCRITOR]  liga��o a um leitor com sucesso... (ConnectNamedPipe)\n"));
		//criar uma thread para cada cliente para ler msg vindas deles.
		if ((CreateThread(NULL, 0, recebe_comando_cliente, (LPVOID)hPipe, 0, NULL)) == 0) {
			_tprintf(TEXT("Erro ao criar Thread!"));
			exit(-1);
		}
		//_tprintf(TEXT("Handle criar thread: %d"), hPipe);
	
	
	}
	return 0;
	
}

BOOL existePlayer() {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (cliente[i] != INVALID_HANDLE_VALUE) {
			return TRUE;
		}
	}

	return false;
}

void inicializaVectorClientes() {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		cliente[i] = INVALID_HANDLE_VALUE;
	}
}

BOOL verifica_e_coloca_handle_pipe(HANDLE pipe) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (cliente[i] == INVALID_HANDLE_VALUE) {
			cliente[i] = pipe;
			return true;
		}
	}
	return false;
}

void eliminaHandlePlayer(HANDLE aux) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (cliente[i] == aux) {
			_tprintf(TEXT("[Escritor leu] Eliminado!"));
			cliente[i] = INVALID_HANDLE_VALUE;
		}
	}
}

DWORD WINAPI leMsgJogo(void) {
	while (termina == 0) {
		CopyMemory(&msgJogo, &memoriaPartilhadaGateway.sharedJogo->jogo, sizeof(MensagemJogo)); //meter funcao no dll
		SetEvent(id_evento_comeco);
	}
	return 0;
}