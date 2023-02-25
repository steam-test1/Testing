# BLT XAudioAPI

XAudio API (named before I found out that's what DirectX's audio system is called), or
the eXtra Audio API, is an OpenAL-based API to allow mods to use positional audio. In the future
it will also likely support HRTF, allowing for more accurate 3D sound.

# Units
In some parts of this API, units probably won't matter if they're consistent.

However, in other parts they will. Use meters as your standard unit.

# API:
`blt.xaudio`:
- `setup()`: Run this to start XAudio. This needs to be called at least once before making use of XAudio functionality.
- `issetup()`: `true` if the audio API has been setup, otherwise `false`.
- `loadbuffer(filename...)`: Create (or load, if cached) buffers containing the contents of
the specified OGG files.
- `newsource([num])`: Creates an audio source. By default this does not have any sounds connected. You
may optionally specify a number of sources to create, and they will be returned as individual arguments.
- `setworldscale(scale)`: Sets a world scale. All positions passed in are divided by this value.
- `getworldscale()`: Gets the current world scale.

Buffer:
- `close([force])`: Closes this buffer. If you don't close it, the buffer will be reused if you later request the same file
and will automatically be closed when the game exits. If the `force` parameter is supplied and it's `true`, the buffer will be unloaded
(assuming there is not another instance of the object open). If not specified or `false`, the buffer will remain cached to speed up loading
for when the source is next loaded.
- `getsamplecount()`: Returns the amount of samples in this buffer.
- `getsamplerate()`: Returns the sample rate of this buffer.

Audio Source:
- `close()`: Closes this audio source. Probably a bit buggy, I haven't yet tested it. If you don't close it, it will
automatically be destroyed when the game exits.
- `setbuffer(buffer)`: Sets this source to play whatever audio is contained by the specified buffer.
- `play()`: Play this audio source.
- `pause()`: Pauses this audio source.
- `stop()`: Stops this audio source.
- `getstate()`: One of `playing`, `paused`, `stopped`, or `other`. More states will be added when streaming is implemented.
- `setposition(x,y,z)`: Sets the position of this audio source in 3D space.
- `setvelocity(x,y,z)`: Sets the velocity of this audio source. This is solely used for applying the doppler effect (stuff moving
towards and away from you have differnt pitches).
- `setdirection(x,y,z)`: Sets the direction vector of this sound. Currently doesn't (seem to) do anything.
- `setmindis(min_dis)`: Sets the distance at which the sound starts getting quieter.
- `setmaxdis(max_dis)`: Sets the distance at which the sound stops being audible.
- `getgain()`: Returns the volume at which the sound is played.
- `setgain(gain)`: Sets the volume at which the sound is played (0-1).

`blt.xaudio.listener`:
- `setposition(x,y,z)`: Sets the position in 3D space of the player.
- `setvelocity(x,y,z)`: Sets the velocity of the player. As per audio sources, this is used for the doppler effect.
- `setorientation(x,y,z, x,y,z)`: Sets the orientation of the player, with a forward-up vector pair.
