// ClienteGrafico.cpp : Define o ponto de entrada para o aplicativo.
//
//Includes
#include "framework.h"
#include "ClienteGrafico.h"
#include <windowsx.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <mmsystem.h>
#include "..\Dll\dll.h"

#define MAX_LOADSTRING 100


//Prototipos para Cliente
void createPipeCliente();
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam);
BOOL verifica_ON();
int getPlayer();

//THREADS
DWORD WINAPI leMensagemJogo(void);

//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
int pontosPlayer = 0;
TCHAR nomePlayer[100] = TEXT(" ");
MensagemJogo msgJogo;
HWND hWnd;


//PROVISORIO
int x = 10;
int y = 10;

//thread
HANDLE thread_mensagem_jogo;
HANDLE thread_mensagem;


TCHAR informacoes[100] = TEXT("");

// Variáveis Globais:
HINSTANCE hInst;                                // instância atual
WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal


// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Login(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Top(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Coloque o código aqui.
	//PIPE
	createPipeCliente();
	DWORD mode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hpipe, &mode, NULL, NULL);

	login = false;

	// Inicializar cadeias de caracteres globais
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CLIENTEGRAFICO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Realize a inicialização do aplicativo:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTEGRAFICO));

	MSG msg;

	// Loop de mensagem principal:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  FUNÇÃO: MyRegisterClass()
//
//  FINALIDADE: Registra a classe de janela.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTEGRAFICO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(0, 0, 255)));
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENTEGRAFICO);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNÇÃO: InitInstance(HINSTANCE, int)
//
//   FINALIDADE: Salva o identificador de instância e cria a janela principal
//
//   COMENTÁRIOS:
//
//        Nesta função, o identificador de instâncias é salvo em uma variável global e
//        crie e exiba a janela do programa principal.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Armazenar o identificador de instância em nossa variável global

	//HWND 
//	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);



	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, // segundo parametro szTitle
		CW_USEDEFAULT, 0, LIMITE_DIREITO + 30, LIMITE_INFERIOR + 110, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNÇÃO: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  FINALIDADE: Processa as mensagens para a janela principal.
//
//  WM_COMMAND  - processar o menu do aplicativo
//  WM_PAINT    - Pintar a janela principal
//  WM_DESTROY  - postar uma mensagem de saída e retornar
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Variaveis
	COMANDO_SHARED comando;
	HANDLE ioReady;
	OVERLAPPED ov;
	DWORD tam = 0;

	static HBRUSH bg = NULL;
	static int nx = 0, ny = 0;

	static HDC hdc = NULL;
	static HDC auxDC = NULL;
	static HBITMAP auxBM = NULL;

	//Tijolo NORMAL
	static HBITMAP hTijolo = NULL;
	static BITMAP bmTijolo;
	static HDC hdcTijolo;

	//RESISTENTE
	static HBITMAP hTijoloR = NULL;
	static BITMAP bmTijoloR;
	static HDC hdcTijoloR;

	//RESISTENTE
	static HBITMAP hTijoloR1 = NULL;
	static BITMAP bmTijoloR1;
	static HDC hdcTijoloR1;

	//RESISTENTE
	static HBITMAP hTijoloR2 = NULL;
	static BITMAP bmTijoloR2;
	static HDC hdcTijoloR2;

	//RESISTENTE
	static HBITMAP hTijoloR3 = NULL;
	static BITMAP bmTijoloR3;
	static HDC hdcTijoloR3;


	//MAGICO
	static HBITMAP hTijoloM = NULL;
	static BITMAP bmTijoloM;
	static HDC hdcTijoloM;


	//Barreira
	static HBITMAP hBarreira = NULL;
	static BITMAP bmBarreira;
	static HDC hdcBarreira;

	//Brindes
	//SPEED-UP (Spup)
	static HBITMAP hSpup;
	static BITMAP bmSpup;
	static HDC hdcSpup;

	//SLOW-DOWN 
	static HBITMAP hSldwn;
	static BITMAP bmSldwn;
	static HDC hdcSldwn;

	//Vida-Extra
	static HBITMAP hVextra;
	static BITMAP bmVextra;
	static HDC hdcVextra;

	//Triple
	static HBITMAP hTriple;
	static BITMAP bmTriple;
	static HDC hdcTriple;


	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	switch (message)
	{
	case WM_CREATE: //Quando é chamado o createWindow
		//Obter as dimensões do Ecra
		bg = CreateSolidBrush(RGB(153, 153, 0));
		nx = GetSystemMetrics(SM_CYSCREEN);
		ny = GetSystemMetrics(SM_CYSCREEN);


		// Preparação de 'BITMAP'
		hdc = GetDC(hWnd);
		auxDC = CreateCompatibleDC(hdc);
		auxBM = CreateCompatibleBitmap(hdc, nx, ny);
		SelectObject(auxDC, auxBM);
		SelectObject(auxDC, bg);
		PatBlt(auxDC, 0, 0, nx, ny, PATCOPY);
		ReleaseDC(hWnd, hdc);

		// Carregar "BITMAP's"
		hTijolo = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIJOLO), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTijoloR = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIJOLOR), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTijoloR1 = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIJOLOR1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTijoloR2 = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIJOLOR2), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTijoloR3 = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIJOLOR3), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTijoloM = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIJOLOM), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hBarreira = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BARREIRA), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hSpup = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_SPUP), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hSldwn = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_Sldwn), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hVextra = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_Vextra), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTriple = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_Triple), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

		hdc = GetDC(hWnd);

		// GETOBJECT ** Returns a reference to an object provided by an ActiveX component.
		GetObject(hTijolo, sizeof(bmTijolo), &bmTijolo);
		GetObject(hTijoloR, sizeof(bmTijoloR), &bmTijoloR);
		GetObject(hTijoloR1, sizeof(bmTijoloR1), &bmTijoloR1);
		GetObject(hTijoloR2, sizeof(bmTijoloR2), &bmTijoloR2);
		GetObject(hTijoloR3, sizeof(bmTijoloR3), &bmTijoloR3);
		GetObject(hTijoloM, sizeof(bmTijoloM), &bmTijoloM);
		GetObject(hBarreira, sizeof(bmBarreira), &bmBarreira);
		GetObject(hSpup, sizeof(bmSpup), &bmSpup);
		GetObject(hSldwn, sizeof(bmSldwn), &bmSldwn);
		GetObject(hVextra, sizeof(bmVextra), &bmVextra);
		GetObject(hTriple, sizeof(bmTriple), &bmTriple);

		// The CreateCompatibleDC function creates a memory device context (DC) compatible with the specified device.
		hdcTijolo = CreateCompatibleDC(hdc);
		hdcTijoloR = CreateCompatibleDC(hdc);
		hdcTijoloR1 = CreateCompatibleDC(hdc);
		hdcTijoloR2 = CreateCompatibleDC(hdc);
		hdcTijoloR3 = CreateCompatibleDC(hdc);
		hdcTijoloM = CreateCompatibleDC(hdc);
		hdcBarreira = CreateCompatibleDC(hdc);
		hdcSpup = CreateCompatibleDC(hdc);
		hdcSldwn = CreateCompatibleDC(hdc);
		hdcVextra = CreateCompatibleDC(hdc);
		hdcTriple = CreateCompatibleDC(hdc);


		// The SelectObject function selects an object into the specified device context (DC). The new object replaces the previous object of the same type.
		SelectObject(hdcTijolo, hTijolo);
		SelectObject(hdcTijoloR, hTijoloR);
		SelectObject(hdcTijoloR1, hTijoloR1);
		SelectObject(hdcTijoloR2, hTijoloR2);
		SelectObject(hdcTijoloR3, hTijoloR3);
		SelectObject(hdcTijoloM, hTijoloM);
		SelectObject(hdcBarreira, hBarreira);
		SelectObject(hdcSpup, hSpup);
		SelectObject(hdcSldwn, hSldwn);
		SelectObject(hdcVextra, hVextra);
		SelectObject(hdcTriple, hTriple);

		ReleaseDC(hWnd, hdc);

		break;
	case WM_COMMAND:
	{

		if (verifica_ON()) {
			login = true;
		}
		else {
			login = false;
		}
			

		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;

		int wmId = LOWORD(wParam);
		// Analise as seleções do menu:
		switch (wmId)
		{
		case IDM_LOGIN:
			if (login == false) {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN), hWnd, Login);
			}
			else {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGINFAIL), hWnd, About);
			}

			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		case IDM_TOP10:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_TOP10), hWnd, Top);
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		case IDM_EXIT:
			//mandar mensagem ao servidor para inserir no ranking e dizer que desconectou !
			comando.idUser = 0;
			comando.tipo = CMD_LOGOUT;
			comando.idHandle = hpipe;
			login = false;
			escrevePipe(comando, ioReady, ov, tam);
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{ //pintar
		PAINTSTRUCT ps;
		// HDC hdc = BeginPaint(hWnd, &ps);
		 // TODO: Adicione qualquer código de desenho que use hdc aqui...

		 // **The PatBlt function** paints the specified rectangle using the brush that is currently 
		 //selected into the specified device context. The brush color and the surface color 
		 //or colors are combined by using the specified raster operation.
		//PatBlt(auxDC, 0, 0, nx, ny, PATCOPY);WHITENESS 

		PatBlt(auxDC, 0, 0, nx, ny, BLACKNESS);
		//The SetStretchBltMode function sets the bitmap stretching mode in the specified device context.
		//BLACKONWHITE Perserva os valores pretos acima dos brancos (Pixeis)
		SetStretchBltMode(auxDC, BLACKONWHITE);

		//Imprimir tijolos enum Tipo_Tijolo{normal, resistente, magico};
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			if (msgJogo.tijolos[i].vida != 0) {

				switch (msgJogo.tijolos[i].tipo) {
				case normal:
					StretchBlt(auxDC, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y, LARG_TIJOLO, ALT_TIJOLO, hdcTijolo, 0, 0, bmTijolo.bmWidth, bmTijolo.bmHeight, SRCCOPY);
					break;
				case resistente:
					switch (msgJogo.tijolos[i].vida) {
					case 1:
						StretchBlt(auxDC, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y, LARG_TIJOLO, ALT_TIJOLO, hdcTijoloR1, 0, 0, bmTijoloR1.bmWidth, bmTijoloR1.bmHeight, SRCCOPY);
						break;
					case 2:
						StretchBlt(auxDC, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y, LARG_TIJOLO, ALT_TIJOLO, hdcTijoloR2, 0, 0, bmTijoloR2.bmWidth, bmTijoloR2.bmHeight, SRCCOPY);
						break;
					case 3:
						StretchBlt(auxDC, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y, LARG_TIJOLO, ALT_TIJOLO, hdcTijoloR3, 0, 0, bmTijoloR3.bmWidth, bmTijoloR3.bmHeight, SRCCOPY);
						break;
					case 4:
						StretchBlt(auxDC, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y, LARG_TIJOLO, ALT_TIJOLO, hdcTijoloR, 0, 0, bmTijoloR.bmWidth, bmTijoloR.bmHeight, SRCCOPY);
						break;
					}


					break;
				case magico:
					StretchBlt(auxDC, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y, LARG_TIJOLO, ALT_TIJOLO, hdcTijoloM, 0, 0, bmTijoloM.bmWidth, bmTijoloM.bmHeight, SRCCOPY);
					break;

				}
			}
			//StretchBlt(auxDC, msgJogo.bola.coord.X + 10 , msgJogo.bola.coord.Y + 10, ALT_TIJOLO, ALT_TIJOLO, hdcTijolo, 0, 0, bmTijolo.bmWidth, bmTijolo.bmHeight, SRCCOPY);
		}

		//Barreira
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) {
				if (msgJogo.players[i].barreira.ativa) {
					StretchBlt(auxDC, msgJogo.players[i].barreira.coord.X, msgJogo.players[i].barreira.coord.Y, msgJogo.players[i].barreira.dimensao, ALT_BARREIRA, hdcBarreira, 0, 0, bmBarreira.bmWidth, bmBarreira.bmHeight, SRCCOPY);
				}
			}
		}

		//Brindes enum Tipo_Brinde{speed_up, slow_down, vida_extra, triple, barreira}; //Adicionar outros brindes consoante a originalidade

		for (int i = 0; i < MAX_NUM_BRINDES; i++) {
			if (msgJogo.brindes[i].ativo != 0) {
				switch (msgJogo.brindes[i].tipo)
				{
				case speed_up:
					StretchBlt(auxDC, msgJogo.brindes[i].coord.X, msgJogo.brindes[i].coord.Y, msgJogo.brindes[i].dimensao, ALT_BRINDE, hdcSpup, 0, 0, bmSpup.bmWidth, bmSpup.bmHeight, SRCCOPY);
					break;
				case slow_down:
					StretchBlt(auxDC, msgJogo.brindes[i].coord.X, msgJogo.brindes[i].coord.Y, msgJogo.brindes[i].dimensao, ALT_BRINDE, hdcSldwn, 0, 0, bmSldwn.bmWidth, bmSldwn.bmHeight, SRCCOPY);
					break;
				case vida_extra:
					StretchBlt(auxDC, msgJogo.brindes[i].coord.X, msgJogo.brindes[i].coord.Y, msgJogo.brindes[i].dimensao, ALT_BRINDE, hdcVextra, 0, 0, bmVextra.bmWidth, bmVextra.bmHeight, SRCCOPY);
					break;
				case triple:
					StretchBlt(auxDC, msgJogo.brindes[i].coord.X, msgJogo.brindes[i].coord.Y, msgJogo.brindes[i].dimensao, ALT_BRINDE, hdcTriple, 0, 0, bmTriple.bmWidth, bmTriple.bmHeight, SRCCOPY);
					break;
				default:
					//IMPRIMIR UM NORMAL!
					break;
				}

			}
		}


		//BOLA
		for (int i = 0; i < MAX_NUM_BOLAS; i++) {
			if (msgJogo.bolas[i].ativa == 1)
				Ellipse(auxDC, msgJogo.bolas[i].coord.X, msgJogo.bolas[i].coord.Y, msgJogo.bolas[i].coord.X + msgJogo.bolas[i].raio, msgJogo.bolas[i].coord.Y + msgJogo.bolas[i].raio);
		}
		

		//swprintf_s(informacoes, TEXT("Posicao da Bola : (% d, % d)\n"), msgJogo.bolas[0].coord.X, msgJogo.bolas[0].coord.Y);
	

		int aux_global = getPlayer();
		
		swprintf_s(informacoes, TEXT("Nome: %s Vidas: %d \t Pontuacao: %d Velocidade: %f Dimensao: %d\n"), msgJogo.players[aux_global].nome, msgJogo.players[aux_global].vidas, msgJogo.players[aux_global].pontos, msgJogo.players[aux_global].barreira.velocidade, msgJogo.players[aux_global].barreira.dimensao);
		TextOut(auxDC, LIMITE_ESQUERDO + 10, LIMITE_INFERIOR + 20, informacoes, _tcslen(informacoes));
		//Copia a informação que está no 'DC' para a memória do Display ;)
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, nx, ny, auxDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_KEYDOWN:
		//Lidar com as teclas primidas
		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;

		switch (wParam) {
		case VK_LEFT:
			comando.idUser = 0;
			comando.tipo = CMD_MOVE_ESQ;
			comando.idHandle = hpipe;
			//PlaySound(TEXT("sound4.wav"), NULL, SND_SYNC | SND_RESOURCE);
			escrevePipe(comando, ioReady, ov, tam);
			break;
		case VK_RIGHT:
			comando.idUser = 0;
			comando.tipo = CMD_MOVE_DIR;
			//PlaySound(TEXT("sound4.wav"), NULL, SND_SYNC | SND_RESOURCE);
			comando.idHandle = hpipe;
			escrevePipe(comando, ioReady, ov, tam);
			break;
		case 0x1B: //Process an escape
			//Tecnicamente carregando escape o jogador baza :O
			comando.idUser = 0;
			comando.tipo = CMD_LOGOUT;
			comando.idHandle = hpipe;
			escrevePipe(comando, ioReady, ov, tam);
			Sleep(20);
			exit(0);
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		exit(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Manipulador de mensagem para a caixa 'sobre'.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Manipulador de mensagem para a caixa 'login'.
INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			COMANDO_SHARED comando;
			HANDLE ioReady;
			OVERLAPPED ov;
			DWORD tam = 0;
			ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);

			HWND edit;
			TCHAR buff[1024];

			edit = GetDlgItem(hDlg, IDC_USER);
			GetWindowText(edit, buff, 1024);
			_tcscpy_s(comando.nome, sizeof(TCHAR[25]), buff);

			comando.tipo = CMD_LOGIN;
			comando.idHandle = hpipe;
			ZeroMemory(&ov, sizeof(ov));
			ResetEvent(ioReady);
			ov.hEvent = ioReady;
			escrevePipe(comando, ioReady, ov, tam);
			thread_mensagem_jogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)leMensagemJogo, NULL, 0, NULL);
			login = true;

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Manipulador de mensagem para a caixa 'sobre'.
INT_PTR CALLBACK Top(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	COMANDO_SHARED comando;
	HANDLE ioReady;
	OVERLAPPED ov;
	DWORD tam = 0;
	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	TCHAR str[100] = TEXT("");
	HWND hwndList;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:

		hwndList = GetDlgItem(hDlg, IDC_LISTBOX);

		comando.idUser = 0;
		comando.tipo = CMD_REGISTRY;
		comando.idHandle = hpipe;
		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;
		escrevePipe(comando, ioReady, ov, tam);

	
		for (int i = 0; i < 10; i++) {
			swprintf_s(str, TEXT("%d º Nome: %s Score: %d"), i, msgJogo.ranking.jogadores[i].nome, msgJogo.ranking.jogadores[i].pontos);

			//int pos = (int)//
				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)str);
		
			//SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)i);
		}

		SetFocus(hwndList);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
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
		//Está a dar erro a escrever para o pipe
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
			WaitForSingleObject(IoReady, INFINITE);
			GetOverlappedResult(hpipe, &ov, &tam, FALSE);


			//fazer um refresh
		//	Sleep(0166); // 1 / 60 para meter 60hz

			InvalidateRect(hWnd, NULL, FALSE);

		}
	}
	return 0;
}


BOOL verifica_ON() { //Meter no lado do servidor a funcionar
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle == hpipe)
			return true;
	}
	return false;
}

int getPlayer() {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle == hpipe) {
			return i;
		}
	}
	return 0;
}