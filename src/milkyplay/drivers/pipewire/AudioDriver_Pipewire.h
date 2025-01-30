#ifndef __AUDIODRIVER_PIPEWIRE_H__
#define __AUDIODRIVER_PIPEWIRE_H__
#include "AudioDriver_COMPENSATE.h"
#include <cstdint>
#include <pipewire/pipewire.h>
#include <pipewire/thread-loop.h>

class AudioDriver_Pipewire : public AudioDriver_COMPENSATE
{
private:
	struct pw_stream *stream;
	struct pw_thread_loop *thread_loop;
	struct pw_context *context;
	struct pw_core *core;
	static void on_process(void *userdata);
	static void on_stream_param_change(void *userdata, uint32_t id, const struct spa_pod *param);
	struct pw_stream_events *events;
	mp_sint32 period;
	bool initialized = false;
	
public:

	AudioDriver_Pipewire();

	virtual ~AudioDriver_Pipewire();

	virtual     mp_sint32   initDevice(mp_sint32 periodSizeAsSamples, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     mp_sint32	start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual		const char* getDriverID() { return "Pipewire"; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 1024; }
};

#endif
