from pathlib import Path
import numpy as np,json,csv
r=Path(__file__).resolve().parents[1];data=json.loads((r/'report/deep-results.json').read_text());result=[]
for c in data['cases']:
 if not (c['name'].startswith('pitch-') or c['name'].startswith('transpose-')):continue
 sr=c['sample_rate'];expected=440*2**((c['base_midi']+c['transpose']-69)/12);x=np.fromfile(r/'qa/audio'/f"{c['name']}.f32",np.float32);y=x[int(sr):];N=len(y)*16;spec=np.abs(np.fft.rfft(y*np.hanning(len(y)),n=N));freq=np.fft.rfftfreq(N,1/sr);a,b=np.searchsorted(freq,[expected*.9,expected*1.1]);k=a+np.argmax(spec[a:b]);s0,s1,s2=np.log(np.maximum(spec[k-1:k+2],1e-20));delta=.5*(s0-s2)/(s0-2*s1+s2);hz=(k+delta)*sr/N
 result.append(dict(name=c['name'],sample_rate=sr,expected_hz=expected,actual_hz=float(hz),residual_cents=float(1200*np.log2(hz/expected))))
(r/'report/output-pitch-results.json').write_text(json.dumps({'scope':'Spectral estimates on last second of 2 s synthetic harmonic inputs, Flex=0; expected note known. Formant on/off when named.','cases':result},indent=2)+'\n')
print('steady cases',len([x for x in result if x['name'].startswith('pitch-')]),'worst cents',max(abs(x['residual_cents']) for x in result if x['name'].startswith('pitch-')))
for x in result:
 if x['name'].startswith('transpose-'):print(x['name'],round(x['residual_cents'],3))
for name in ['signal-2-plain','frozen-target-step']:
 rows=list(csv.DictReader((r/'qa/audio'/f'{name}.csv').open()));samples=[]
 for t in [.45,.85,1.35,1.85]:
  row=min(rows,key=lambda x:abs(float(x['t'])-t));inputMidi=float(row['input_midi']);corrected=float(row['detected_midi'])+float(row['command_cents'])*.01
  samples.append(dict(time=t,input_midi=inputMidi,target_midi=float(row['target_midi']),inferred_corrected_midi=corrected))
 print(name,samples)
