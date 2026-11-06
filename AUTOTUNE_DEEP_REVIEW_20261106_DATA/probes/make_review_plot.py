from pathlib import Path
import json,csv,os
r=Path(__file__).resolve().parents[1];(r/'qa/plot-cache').mkdir(exist_ok=True);os.environ['MPLCONFIGDIR']=str(r/'qa/plot-cache');os.environ['XDG_CACHE_HOME']=str(r/'qa/plot-cache')
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt,numpy as np
fig,ax=plt.subplots(2,2,figsize=(11,7),layout='constrained')
x=json.load(open(r/'report/previous-pitch-results.json'))['cases']
for name,label in [('live_default','Flex 15 (source default)'),('flex_zero','Flex 0')]:
 rows=[c for c in x if c['name']==name];ax[0,0].plot([c['input_offset_cents'] for c in rows],[c['output_cents_from_A3'] for c in rows],marker='o',label=label)
ax[0,0].axhline(0,color='#777',lw=.7);ax[0,0].set(xlabel='Input error (cents)',ylabel='Output error (cents)',title='Steady target A3, transpose 0');ax[0,0].legend(fontsize=8)
for name,label in [('signal-2-plain','Normal target'),('frozen-target-step','Frozen target A3')]:
 rows=list(csv.DictReader((r/'qa/audio'/f'{name}.csv').open()));ax[0,1].plot([float(z['t']) for z in rows],[float(z['target_midi']) for z in rows],label=label)
ax[0,1].set(xlabel='Time (s)',ylabel='Target MIDI note',title='Changing melody while FREEZE is active',ylim=(56.5,61.5));ax[0,1].set_yticks([57,59,61],['A3','B3','C#4']);ax[0,1].legend(fontsize=8)
limits=json.load(open(r/'report/transpose-limit-results.json'));labels=[f"{v['transpose']:+d} st\ninput {v['input_offset_cents']:+d} c" for v in limits];ax[1,0].bar(labels,[v['residual_cents'] for v in limits],color=['#6c9e8c','#cd8765','#cd8765','#6c9e8c']);ax[1,0].set(ylabel='Residual error (cents)',title='Correction hits the ratio limit at +/-12 st');ax[1,0].axhline(0,color='#777',lw=.7)
sr=96000;name='high-formant-96000-69-6';wet=np.fromfile(r/'qa/audio'/f'{name}.f32',np.float32);plain=np.fromfile(r/'qa/audio'/f'{name.replace("high-formant","high-plain")}.f32',np.float32);t=np.arange(len(wet))/sr;phase=2*np.pi*440*2**(.3/12)*(np.arange(len(wet))+1)/sr;amp=np.where(t<.5,.02,np.where(t<1,.4,np.where(t<1.5,.03,.2)));inp=amp*(np.sin(phase)+.4*np.sin(2*phase+.3)+.175*np.sin(3*phase+.7))
start,end=int(sr*.490),int(sr*.535);block=48
for signal,label in [(inp,'Input'),(plain,'Formant Off'),(wet,'Formant On')]:
 peaks=[];times=[]
 for i in range(start,end-block,block):peaks.append(float(np.max(abs(signal[i:i+block]))));times.append(1000*(i/sr-.5))
 ax[1,1].plot(times,peaks,label=label)
ax[1,1].axhline(1,color='#bd4940',ls='--',lw=.8,label='0 dBFS');ax[1,1].set(xlabel='Time after amplitude step (ms)',ylabel='Local absolute peak',title='96 kHz synthetic transient, no gain added');ax[1,1].legend(fontsize=8)
for a in ax.flat:a.grid(alpha=.17)
fig.savefig(r/'report/AUTOTUNE_DEEP_REVIEW_20261106.png',dpi=150)
