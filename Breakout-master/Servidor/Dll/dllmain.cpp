// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"
#include <windows.h>
#include "dll.h"

char ponteiro[40960];
//Definição de variavel global
int nDLL = 1234;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

bool createSharedMemory(dataCr* d) {
	d->hMapFileMSG = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Dados), nomeMemoriaComandos);
	if (d->hMapFileMSG == NULL) {
		_tprintf(TEXT("[Erro]Cria��o de objectos do Windows(%d)\n"), GetLastError());
		return false;
	}

	d->shared = (Dados*)MapViewOfFile(d->hMapFileMSG, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Dados));
	if (d->shared == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		return false;
	}


	d->hSemafroPodeEscrever = CreateSemaphore(NULL, 1000, 1000, nomeSemaforoPodeEscrever);
	if (d->hSemafroPodeEscrever == NULL) {
		_tprintf(TEXT("O semafro correu mal\n"));
		return false;
	}
	
	d->hSemafroPodeLer = CreateSemaphore(NULL, 0, BUFFER, nomeSemaforoPodeLer);
	if (d->hSemafroPodeLer == NULL) {
		_tprintf(TEXT("O semafro deu problemas\n"));
		return false;
	}

	d->shared->posWrite = 0;
	d->shared->posRead = 0;
	return true;
}

bool openSharedMemory(dataCr* d) {
	d->hMapFileMSG = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nomeMemoriaComandos);
	if (d->hMapFileMSG == NULL) {
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		return false;
	}

	d->shared = (Dados*)MapViewOfFile(d->hMapFileMSG, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Dados));
	if (d->shared == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		CloseHandle(d->hMapFileMSG);
		return false;
	}


	d->hSemafroPodeEscrever = OpenSemaphore(SYNCHRONIZE, TRUE, nomeSemaforoPodeEscrever);
	if (d->hSemafroPodeEscrever == NULL) {
		_tprintf(TEXT("Erro Semaphoro!\n"));
		return false;
	}

	d->hSemafroPodeLer = OpenSemaphore(SYNCHRONIZE, TRUE, nomeSemaforoPodeLer);
	if (d->hSemafroPodeLer == NULL) {
		_tprintf(TEXT("O Semaforo da erro!\n"));
		return false;
	}
	return true;
}

void readMensagem(dataCr* d, COMANDO_SHARED* s) {
	 WaitForSingleObject(d->hSemafroPodeLer, INFINITE);

	d->posL = d->shared->posRead;
	d->shared->posRead = (d->shared->posRead + 1) % BUFFER;
	WaitForSingleObject(hMutexLer, INFINITE);

	CopyMemory(s, &d->shared->PtrMemoria[d->posL], sizeof(COMANDO_SHARED));

	ReleaseMutex(hMutexLer);
	ReleaseSemaphore(d->hSemafroPodeEscrever, 1, NULL);
}

void writeMensagem(dataCr* d, COMANDO_SHARED* s) {
	 WaitForSingleObject(d->hSemafroPodeEscrever, INFINITE);
	// DWORD teste =
	 //if (teste == WAIT_OBJECT_0) {
		//_tprintf(TEXT("AGUA!\n"));
	//}
	//else {
		//_tprintf(TEXT("AGUIA MIDANADA!\n"));
	//}

	WaitForSingleObject(hMutexEscrever, INFINITE);

	d->posE = d->shared->posWrite;
	d->shared->posWrite = (d->shared->posWrite + 1) % BUFFER;
	
	CopyMemory(&d->shared->PtrMemoria[d->posE], s, sizeof(COMANDO_SHARED));

	ReleaseMutex(hMutexEscrever);

	ReleaseSemaphore(d->hSemafroPodeLer, 1, NULL);
	
}

void createSharedMemoryJogo(dataCr* d) {
	d->hMapFileJogo = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DadosJogo), nomeMemoriaJogo);

	if (d->hMapFileJogo == NULL) {
		_tprintf(TEXT("[Erro]Cria��o de objectos do Windows(%d)\n"), GetLastError());
		return;
	}

	d->sharedJogo = (DadosJogo*)MapViewOfFile(d->hMapFileJogo, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DadosJogo));
	if (d->sharedJogo == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		return;
	}

	}

bool openSharedMemoryJogo(dataCr* d) {
	d->hMapFileJogo = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nomeMemoriaJogo);

	if (d->hMapFileJogo == NULL) {
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		return false;
	}

	d->sharedJogo = (DadosJogo*)MapViewOfFile(d->hMapFileJogo, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DadosJogo));

	if (d->sharedJogo == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		CloseHandle(d->hMapFileJogo);
		return false;
	}
	return true;
}

void readMensagemJogo(dataCr* d, MensagemJogo* s) {

}

void writeMensagemJogo(dataCr* d, MensagemJogo* s) {
	CopyMemory(&d->sharedJogo->jogo, s , sizeof(MensagemJogo));
}

void assertHandleIsNot(HANDLE qual, HANDLE valor, TCHAR* msg) {
	if (qual == valor) {
		_tprintf(TEXT("\nAssert: %s\n"), msg);
		exit(1);
	}
}

void gotoxy(int x, int y) {
	static HANDLE hStdout = NULL;
	COORD coord;
	coord.X = x;
	coord.Y = y;

	if (hStdout == NULL) {
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		assertHandleIsNot(hStdout, NULL, (TCHAR*)TEXT("GetStdHandle falhou"));
	}
	SetConsoleCursorPosition(hStdout, coord);

}