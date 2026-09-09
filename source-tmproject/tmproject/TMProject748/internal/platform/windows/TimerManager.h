#pragma once

// Mantém os relógios local, real e sincronizado com o servidor. O timer de alta
// resolução é usado quando QPF está disponível, sem alterar o tempo autoritativo.
class TimerManager
{
public:
	TimerManager();
	virtual ~TimerManager();

	// Inicializa a base do relógio local; deve preceder as leituras de tempo.
	void StartTimer();
	// Ajusta/lê a estimativa de tempo do servidor usada pelas animações.
	virtual void SetServerTime(DWORD dwServerTime);
	virtual DWORD GetServerTime();
	// Ajusta/lê o relógio real mantido separadamente da sincronização do servidor.
	virtual void SetRealTime(DWORD dwTime);
	virtual DWORD GetRealTime();
public:
	// Campos QPF armazenam ticks de alta resolução; os DWORD preservam os
	// relógios legados e suas bases. Não há recursos externos com ownership.
	int m_bTimerInitialized;
	DWORD m_dwCurrentIndexNumber;
	long double m_fLastElapsedTime;
	long double m_fBaseTime;
	long double m_fStopTime;
	int m_bUsingQPF;
	LONGLONG m_llStopTime;
	LONGLONG m_llLastElapsedTime;
	LONGLONG m_llBaseTime;
	LONGLONG m_llQPFTicksPerSec;
	DWORD m_dwLastSetServerTime;
	DWORD m_dwServerTime;
	DWORD m_dwBaseTime;
	DWORD m_dwRealTime;
	DWORD m_dwDelayTime;
	LONGLONG m_time;
	DWORD m_dwLastMin;
};

// Retorna o relógio global de alta resolução usado por medições locais.
double GetTimerTime();
