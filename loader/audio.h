#ifndef __AUDIO_H__
#define __AUDIO_H__

/**
 * @brief Port audio player, adapted from Zenonia 2 (same engine) with two real differences from Zenonia 3: 1.
 */

void audio_init(void);
void audio_play(int snd_id, int vol, int is_loop); // OnSoundPlay(id, vol, isLoop)
void audio_stop_all(void);                         // OnStopSound
void audio_stop_bgm(void);                         // OnSoundPlay(id, 0, true)

#endif
