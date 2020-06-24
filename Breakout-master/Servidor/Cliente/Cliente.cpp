//Includes
#include <windowsx.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include "..\Dll\dll.h"

//Prototipos para Cliente
void createPipeCliente();
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam);

//THREADS
DWORD WINAPI leMensagemJogo(void);

//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
int pontosPlayer = 0;
TCHAR nomePlayer[100] = TEXT(" ");
MensagemJogo msgJogo;

//thread
HANDLE thread_mensagem_jogo;
HANDLE thread_mensagem;

//FUNÇÕES
//Função Principal!
int _tmain(int argc, LPTSTR argv[]) {

	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	//PIPE
	createPipeCliente();
	DWORD mode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hpipe, &mode, NULL, NULL);

	//Login
	TCHAR buf[256];

	COMANDO_SHARED comando;
	HANDLE ioReady;
	OVERLAPPED ov;
	DWORD tam = 0;

	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	do {
		_tprintf(TEXT("Nome utilizador: "));
		_fgetts(buf, 256, stdin);
		comando.idUser = 0;
		comando.tipo = CMD_LOGIN;
		comando.idHandle = hpipe;
		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;
		escrevePipe(comando, ioReady, ov, tam);
		login = true;
	} while (login == false);

	thread_mensagem_jogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)leMensagemJogo, NULL, 0, NULL);


	//while (1) {
		
	//}


	_gettchar();
	_tprintf(TEXT("\Terminei!\n"));
	//Mandar mensagem ao gateway e servidor a dizer que o cliente foi desconnectado
	system("pause");
}

// **** PIPES ****
//Criar Pipe
void createPipeCliente() {
	_tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);
	/*
	BOOL WaitNamedPipeA(
		LPCSTR lpNamedPipeName,
		DWORD  nTimeOut
	);
	*/
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	/*
	HANDLE CreateFileA(
		LPCSTR                lpFileName,
		DWORD                 dwDesiredAccess,
		DWORD                 dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD                 dwCreationDisposition,
		DWORD                 dwFlagsAndAttributes,
		HANDLE                hTemplateFile
	);
	*/
	hpipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
	if (hpipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Liguei-me...\n"));

}

//Escreve Pipe
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam) {

	if (!WriteFile(hpipe, &comando, sizeof(COMANDO_SHARED), &tam, &ov)) {
		_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
		exit(-1);
	}

	WaitForSingleObject(ioReady, INFINITE);
	/*
	BOOL GetOverlappedResult(
		HANDLE       hFile,
		LPOVERLAPPED lpOverlapped,
		LPDWORD      lpNumberOfBytesTransferred,
		BOOL         bWait
	);
	*/
	GetOverlappedResult(hpipe, &ov, &tam, FALSE);
	_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao pipe ...\n"), tam);
}


DWORD WINAPI leMensagemJogo(void) {
	HANDLE IoReady;
	OVERLAPPED ov;
	DWORD tam;
	IoReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	int aux = 0;
	while (1) {
		if (login == TRUE) {
			ZeroMemory(&ov, sizeof(ov));
			ResetEvent(IoReady);
			ov.hEvent = IoReady;

			ReadFile(hpipe, &msgJogo, sizeof(MensagemJogo), &tam, &ov);

			_tprintf(TEXT("Posicao da Bola: (%d , %d)\n"), msgJogo.bolas[0].coord.X, msgJogo.bolas[0].coord.Y);
			Sleep(500);
			WaitForSingleObject(IoReady, INFINITE);
			GetOverlappedResult(hpipe, &ov, &tam, FALSE);
		}
	}
	return 0;
}