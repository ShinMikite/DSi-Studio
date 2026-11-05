"""Generate original PCM music and UI sounds with controlled playback headroom."""
from pathlib import Path
from array import array
import math, wave
ROOT=Path(__file__).resolve().parents[1]
RATE=16384
out=ROOT/'assets/audio'; out.mkdir(parents=True,exist_ok=True)
def note(n): return 440*2**((n-69)/12)
def save(name,samples,peak):
    gain=peak/max(abs(s) for s in samples)
    pcm=array('h',(round(s*gain*32767) for s in samples))
    # Sound hardware lengths are multiples of four bytes.
    if len(pcm)%2: pcm.append(0)
    (out/(name+'.bin')).write_bytes(pcm.tobytes())
    with wave.open(str(out/(name+'.wav')),'wb') as w:
        w.setnchannels(1);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(pcm.tobytes())
    assert max(abs(v) for v in pcm)<31000
chords=[(60,64,67,71),(57,60,64,67),(53,57,60,64),(55,59,62,69)]
music=[]
for i in range(RATE*12):
    t=i/RATE; block=int(t/3); local=t%3; chord=chords[block]
    pad=math.sin(math.pi*local/3)**2
    value=sum(math.sin(2*math.pi*note(n-12)*t)*.025*pad for n in chord)
    beat=int(local/.75); age=local%.75
    env=(1-math.exp(-age*45))*math.exp(-age*6)
    f=note(chord[(beat*2)%4]+12)
    value+=(math.sin(2*math.pi*f*age)+.15*math.sin(2*math.pi*f*2*age))*.08*env
    music.append(value)
save('studio_music',music,.50)
for name,notes,duration in [('studio_select',[76],.065),('studio_accept',[72,79],.17),('studio_back',[69,64],.14),('studio_success',[72,76,79],.36),('studio_error',[57,55],.22)]:
    sound=[]
    for i in range(round(RATE*duration)):
        t=i/RATE; segment=duration/len(notes); part=min(int(t/segment),len(notes)-1); age=t-part*segment
        envelope=math.sin(math.pi*age/segment)**2
        sound.append(.17*envelope*math.sin(2*math.pi*note(notes[part])*age))
    save(name,sound,.68)
# Native playback gains: even coincident positive peaks retain headroom.
assert .50*85/127 + .68*110/127 < .95
print('Original 12-second music loop and five UI sounds generated at 16384 Hz; combined peak ceiling below 95%.')
