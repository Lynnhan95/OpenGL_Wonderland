#pragma once
// Audio source based on OpenAL
// Based on PlayStream2005 sample, a very old implementation... but will do for now.
// A better tutorial may be https://ffainelli.github.io/openal-example/

#include <Framework.h>
#include <CWaves.h>
#include <stdio.h>
#include <iostream>
//#include <glm/glm.hpp>
//#include "Camera.h"

#define NUMBUFFERS              (4)
#define	SERVICE_UPDATE_PERIOD	(20)

// Currently implemented for BGM only
class AudioSource
{
private:
	ALuint		    uiBuffers[NUMBUFFERS];
	ALuint		    uiSource;
	ALuint			uiBuffer;
	ALint			iState;
	CWaves*			pWaveLoader = NULL;
	WAVEID			WaveID;
	ALint			iLoop;
	ALint			iBuffersProcessed, iTotalBuffersProcessed, iQueuedBuffers;
	WAVEFORMATEX	wfex;
	unsigned long	ulDataSize = 0;
	unsigned long	ulFrequency = 0;
	unsigned long	ulFormat = 0;
	unsigned long	ulBufferSize;
	unsigned long	ulBytesWritten;
	void*			pData = NULL;
	bool			init_success = false;
	bool			is_playing = false;
	bool			loop = true;

	bool LoadBuffer();

public:
	//glm::vec3 position;
	//Camera*   listener;

	bool Initialize(bool looping, const char* file);
	void Play();
	void Start();
	void Stop();
	void Shutdown();
};