#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#define TAM 256
#define BUFFER_SIZE 100
#define BUFFER 10

#define MAX_NUM_PLAYERS 20
#define MAX_REGISTO 10
//Variaveis configuraveis para o jogo
#define MAX_NUM_VIDAS 3
#define MAX_NUM_TIJOLOS 66
#define MAX_NUM_TIJOLOS_LINHA 11
#define MAX_NUM_BRINDES 66
#define MAX_NUM_BOLAS 3

//LARGURA
#define LARG_TIJOLO 50

//Altura Barreira
#define ALT_BARREIRA 10
#define ALT_TIJOLO 20
#define ALT_BRINDE 20


//DIMENSÔES MAPA
#define LIMITE_SUPERIOR 0
#define LIMITE_INFERIOR 600
#define LIMITE_ESQUERDO 0
#define LIMITE_DIREITO 700

#define CMD_MOVE_CIMA 1
#define CMD_MOVE_BAIXO 2
#define CMD_MOVE_ESQ 3 
#define CMD_MOVE_DIR 4
#define CMD_LOGIN 5
#define CMD_LOGOUT 6
#define CMD_REGISTRY 7

#define PIPE_NAME TEXT("\\\\.\\pipe\\connect")

#define REGKEY TEXT("SOFTWARE\\Arkanoid")
LPCTSTR value = TEXT("Scores");

HKEY hKey;

//Mensagens (SINCRONIZAÇÃO)
HANDLE podeEscrever;
HANDLE podeLer;
HANDLE hMutexLer;
HANDLE hMutexEscrever;

//JOGO
HANDLE mutex_player;

//Objetos JOGO (SINCRONIZAÇÃO)
HANDLE mutex_bola;

//Arrays para controlar brindes e secalhar tijolos

//Memoria Partilhada 
TCHAR nomeMemoriaComandos[] = TEXT("Nome da Memoria Partilhada Comandos");
TCHAR nomeMemoriaJogo[] = TEXT("Nome da Mem�ria Partilhada Jogo");

TCHAR nomeSemaforoPodeEscrever[] = TEXT("Semaforo Pode Escrever");
TCHAR nomeSemaforoPodeLer[] = TEXT("Semaforo Pode Ler");

//EVENTOS
TCHAR nomeEventoComecoJogo[] = TEXT("EventoComeco");
TCHAR nomeEventoArrancaMemoria[] = TEXT("EventoMemoria");
TCHAR nomeEventoTerminaJogo[] = TEXT("EventoTermina");

//Tipo de cenas
enum Tipo_Tijolo{normal, resistente, magico};
enum Tipo_Brinde{speed_up, slow_down, vida_extra, triple, barreira}; //Adicionar outros brindes consoante a originalidade

//Estruturas
typedef struct {
	bool ativa;
	int id;
	COORD coord;
	int dimensao;
	float velocidade;
}Barreira;

//COMANDO PARTILHADO (COMANDO_SHARED)
typedef struct {
	int id;
	TCHAR nome[BUFFER_SIZE];
	Barreira barreira;
	int vidas;
	INT pontos;
	bool login; //Verificar se foi login
	HANDLE idHandle;
}Player;

typedef struct {
	int idUser;
	TCHAR nome[BUFFER_SIZE];
	int tipo;
	bool login;
	HANDLE idHandle;
} COMANDO_SHARED;

typedef struct {
	int ativa;
	int jogador;
	bool cima;
	bool direita;
	//int velocidade;
	//int velocidade_inicial;
	float velocidade;
	float velocidade_inicial;
	int raio;
	COORD coord;
}Bola;


typedef struct {
	int id;
	COORD coord;
	int dimensao;
	Tipo_Tijolo tipo;
	int vida;
}Tijolo;

typedef struct {
	int ativo;
	int id;
	COORD coord;
	DWORD threadId;
	int dimensao;
	Tipo_Brinde tipo;
	float velocidade;
	int duracao;
	bool conf[MAX_NUM_BOLAS];
}Brinde;

typedef struct {
	Player jogadores[MAX_REGISTO];
}Scores;

typedef struct {
	Player players[MAX_NUM_PLAYERS];
	Scores ranking;
	Bola bolas[MAX_NUM_BOLAS];  // Ver se só existe uma bola
	Tijolo tijolos[MAX_NUM_TIJOLOS];
	Brinde brindes[MAX_NUM_BRINDES];

}MensagemJogo; //Jogo

typedef struct {
	MensagemJogo jogo;
}DadosJogo;


//DADOS
typedef struct {
	COMANDO_SHARED PtrMemoria[BUFFER];
	int posRead, posWrite;
} Dados;

//DATACR
typedef struct {
	int posE, posL;
	Dados* shared;
	HANDLE hMapFileMSG;
	HANDLE hSemafroPodeEscrever;
	HANDLE hSemafroPodeLer;
	
	DadosJogo *sharedJogo;
	HANDLE hMapFileJogo;

}dataCr;


#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif


extern "C"
{
	//Váriavel global da DLL
	extern DLL_IMP_API int nDLL;

	//Protótipos Funções
	DLL_IMP_API bool createSharedMemory(dataCr* d);
	DLL_IMP_API bool openSharedMemory(dataCr* d);
	DLL_IMP_API void readMensagem(dataCr* d, COMANDO_SHARED* s);
	DLL_IMP_API void writeMensagem(dataCr* d, COMANDO_SHARED* s);

	DLL_IMP_API void createSharedMemoryJogo(dataCr* d);
	DLL_IMP_API bool openSharedMemoryJogo(dataCr* d);
	DLL_IMP_API void readMensagemJogo(dataCr* d, MensagemJogo* s);
	DLL_IMP_API void writeMensagemJogo(dataCr* d, MensagemJogo* s);


	//Obter posição jogadores
	DLL_IMP_API void gotoxy(int x, int y);

}