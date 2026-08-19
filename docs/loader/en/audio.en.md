# `loader/audio.h` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `audio_init` (line ~4)

**Source File:** `loader/audio.h`

> Port audio player, adapted from Zenonia 2 (same engine) with
> two real differences from Zenonia 3:
>
> 1. The APK .oggs are NOT all 22050 Hz: there are 44100 Hz mono (60) and
> 16000 Hz mono (17). Mixer runs at 44100 and resamples (linear)
> the voices that need it.
> 2. The dispatch replicates ZenoniaUIControllerView.OnSoundPlay (jadx):
> - vol == 0 && isLoop -> stopBGMSound() ("stop music" command)
> - sndID 1..15 -> SFX (SoundPool, overlap)
> - rest -> BGM (isLoop) or stream one-shot (!isLoop)
> and NexusSound.setVolume(vol/10): mVolume = (vol/10)/10.0f.
>
> The files go in ux0:data/zenonia3/sound/sNNN.ogg where NNN is the INDEX
> ordinal of the resource counting from s000 in alphabetical order of res/raw
> original (the Android resource IDs are consecutive and the numbering of
> original files have gaps -- see port_progress.md Phase 5). The script
> from stage of the repo and copies them renamed to ux0_data/zenonia3/sound/.

---