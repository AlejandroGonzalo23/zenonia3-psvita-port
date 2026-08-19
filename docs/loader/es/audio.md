# `loader/audio.h` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `audio_init` (line ~4)

**Source File:** `loader/audio.h`

> Reproductor de audio del port, adaptado del de Zenonia 2 (mismo motor) con
> dos diferencias reales de Zenonia 3:
>
> 1. Los .ogg del APK NO son todos 22050 Hz: hay 44100 Hz mono (60) y
> 16000 Hz mono (17). El mezclador corre a 44100 y resamplea (lineal)
> las voces que lo necesiten.
> 2. El despacho replica ZenoniaUIControllerView.OnSoundPlay (jadx):
> - vol == 0 && isLoop  -> stopBGMSound() (comando de "parar musica")
> - sndID 1..15         -> SFX (SoundPool, se superponen)
> - resto               -> BGM (isLoop) o stream one-shot (!isLoop)
> y NexusSound.setVolume(vol/10): mVolume = (vol/10)/10.0f.
>
> Los archivos van en ux0:data/zenonia3/sound/sNNN.ogg donde NNN es el INDICE
> ordinal del recurso contando desde s000 en orden alfabetico del res/raw
> original (los resource IDs de Android son consecutivos y la numeracion de
> archivos original tiene huecos -- ver port_progress.md Fase 5). El script
> de stage del repo ya los copia renombrados a ux0_data/zenonia3/sound/.

---
