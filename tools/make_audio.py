"""Generate original PCM music and UI sounds with controlled playback headroom."""
from pathlib import Path
from array import array
import math, wave
import numpy as np
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
def music_samples(bpm,chords,melodies):
    beat=60/bpm
    length=round(32*beat*RATE)
    mix=np.zeros(length,dtype=np.float64)
    rng=np.random.default_rng(310)
    def add(start,signal,gain):
        index=(round(start*beat*RATE)+np.arange(len(signal)))%length
        np.add.at(mix,index,signal*gain)
    def voice(start,midi,duration,gain,kind):
        seconds=duration*beat
        t=np.arange(round((seconds+0.24)*RATE))/RATE
        f=note(midi); phase=2*np.pi*f*t
        release=np.exp(-np.maximum(0,t-seconds)*24)
        attack=1-np.exp(-t*180)
        if kind=='keys':
            sound=np.sin(phase+1.3*np.exp(-t*9)*np.sin(2*phase))
            env=attack*np.exp(-t*3.6)*release
        elif kind=='bell':
            sound=np.sin(phase)+.23*np.sin(2*phase)*np.exp(-t*11)+.08*np.sin(3*phase)*np.exp(-t*17)
            env=attack*np.exp(-t*4.8)*release
        elif kind=='bass':
            sound=np.sin(phase)+.14*np.sin(2*phase)
            env=(1-np.exp(-t*85))*np.exp(-t*2.6)*release
        add(start,sound*env,gain)
    def drum(start,kind,gain):
        duration={'kick':.15,'rim':.065,'hat':.038}[kind]
        t=np.arange(round(RATE*duration))/RATE
        noise=rng.uniform(-1,1,len(t))
        if kind=='kick':
            phase=2*np.pi*(49*t+51*.025*(1-np.exp(-t/.025)))
            v=np.sin(phase)*np.exp(-t*29)*(1-np.exp(-t*900))
        elif kind=='rim':
            v=(np.sin(2*np.pi*760*t)+.4*np.sin(2*np.pi*1190*t)+.13*noise)*np.exp(-t*95)
        else:
            bright=np.r_[0,np.diff(noise)]
            v=bright*np.exp(-t*110)*(1-np.exp(-t*1500))
        v*=np.maximum(0,1-t/duration)
        add(start,v,gain)
    for bar,chord in enumerate(chords):
        base=bar*4
        for b in (0,1.5,2.5):
            for n in chord: voice(base+b,n,.7,.055,'keys')
        for b,n in [(0,chord[0]-24),(1.75,chord[2]-24),(2.5,chord[0]-12)]: voice(base+b,n,.65,.15,'bass')
        for b in (0,2): drum(base+b,'kick',.13)
        for b in (1,3): drum(base+b,'rim',.045)
        for b in np.arange(.5,4,.5): drum(base+b,'hat',.017)
        for at,midi,duration in melodies[bar]:
            voice(base+at,midi,duration,.16,'bell')
    # Circular delays preserve the tails across the loop point.
    mix += .12*np.roll(mix,round(.75*beat*RATE))+.055*np.roll(mix,round(.031*RATE))
    mix-=mix.mean()
    mix=np.tanh(mix*2.4)
    mix*=.5/np.max(np.abs(mix))
    # An even sample count is required for the native channel's word length.
    if len(mix)%2: mix=mix[:-1]
    return mix

music=music_samples(112,
 [(60,64,67,71),(57,60,64,67),(62,65,69,72),(55,59,62,69),
  (64,67,71,74),(57,61,64,67),(62,65,69,72),(55,59,62,65)],
 [[(.5,76,.5),(1.25,79,.5),(2,81,.75),(3,79,.5)],
  [(0,76,.75),(1.5,72,.5),(2.5,74,.4),(3.25,76,.5)],
  [(.25,77,.5),(1,81,.75),(2.5,79,.5),(3.25,77,.4)],
  [(0,74,1),(1.5,71,.5),(2.5,69,.7)],
  [(.5,79,.5),(1.25,83,.5),(2,86,.75),(3,83,.5)],
  [(0,81,.75),(1.5,76,.5),(2.5,73,.5),(3.25,76,.5)],
  [(.25,77,.5),(1,74,.75),(2.5,72,.5),(3.25,69,.5)],
  [(0,71,.75),(1.25,74,.5),(2.25,79,.6),(3.25,74,.35)]])
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
print('Original 17-second music loop and five UI sounds generated at 16384 Hz; combined peak ceiling below 95%.')
