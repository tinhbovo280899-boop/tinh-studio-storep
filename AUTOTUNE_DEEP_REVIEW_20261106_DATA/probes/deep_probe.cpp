#include "ChannelOnly.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <string>
#include <algorithm>
struct Config {std::string name;double sr=48000,baseMidi=57,offset=30,retune=25,flex=0,human=35,transpose=0,throat=100,mix=100;int voice=1,pattern=0,freeze=-1;bool formant=false,bypass=false;};
constexpr double pi=3.141592653589793;
int main(int argc,char**argv){std::string dir=argc>1?argv[1]:".";std::vector<Config> cases;
 for(double sr:{44100.,48000.,96000.,192000.})for(int m:{40,45,57,69,81}){Config c;c.name="pitch-"+std::to_string(int(sr))+"-"+std::to_string(m);c.sr=sr;c.baseMidi=m;c.voice=m<50?2:m>=75?0:1;cases.push_back(c);}
 for(int pattern:{1,2,3,4,5,6,7,8,9,10})for(bool formant:{false,true}){Config c;c.name="signal-"+std::to_string(pattern)+(formant?"-formant":"-plain");c.pattern=pattern;c.formant=formant;cases.push_back(c);}
 for(double t:{-12.,-5.,5.,12.})for(bool formant:{false,true}){Config c;c.name="transpose-"+std::to_string(int(t))+(formant?"-formant":"-plain");c.transpose=t;c.formant=formant;cases.push_back(c);}
 for(double throat:{50.,100.,190.}){Config c;c.name="throat-"+std::to_string(int(throat));c.throat=throat;c.formant=true;c.pattern=5;cases.push_back(c);}
 Config hold;hold.name="frozen-target-step";hold.pattern=2;hold.freeze=57;cases.push_back(hold);
 if(argc>2&&(std::string(argv[2])=="high-formant"||std::string(argv[2])=="high-plain")) {cases.clear();for(double sr:{96000.,192000.})for(int note:{40,57,69})for(int signal:{5,6}){Config c;c.name=std::string(argv[2])+"-"+std::to_string(int(sr))+"-"+std::to_string(note)+"-"+std::to_string(signal);c.sr=sr;c.baseMidi=note;c.voice=note<50?2:1;c.pattern=signal;c.formant=std::string(argv[2])=="high-formant";cases.push_back(c);}}
 std::cout<<"{\"cases\":[\n";bool first=true;
 for(auto c:cases){TAutoTuneChannelDSP dsp;dsp.prepare(c.sr);std::array<int,12> notes{},micro{};notes.fill(2);notes[size_t((int(c.baseMidi)%12+12)%12)]=0;bool dynamic=c.pattern==2||c.pattern==3;if(dynamic)notes.fill(0);int n=int(c.sr*2);std::ofstream audio(dir+"/"+c.name+".f32",std::ios::binary),trace;
  if(c.pattern||c.transpose)trace.open(dir+"/"+c.name+".csv");if(trace)trace<<"t,input_midi,detected_midi,target_midi,command_cents,ratio,voiced,formant_recovery\n";
  double phase=0,peak=0,diff=0,last=0,detSum=0,commandSum=0,rms=0;int bad=0,voiced=0,finalVoiced=0,recovery=0,switches=0,lastTarget=-999;double maxBlock=0,totalBlock=0;int blockCount=0;auto begin=std::chrono::steady_clock::now();
  for(int start=0;start<n;start+=64){auto blockStart=std::chrono::steady_clock::now();for(int i=start;i<std::min(n,start+64);++i){double t=i/c.sr,inputMidi=c.baseMidi+c.offset*.01;double amp=.2;
   if(c.pattern==1)inputMidi+=.25*std::sin(2*pi*5.5*t);
   if(c.pattern==2)inputMidi=c.baseMidi+(t<.5?0:t<1?2:t<1.5?4:0)+.3;
   if(c.pattern==3)inputMidi=c.baseMidi+.3+4*std::clamp((t-.4)/.7,0.,1.);
   if(c.pattern==4)amp=t<.2||t>1.7?0:.2;
   if(c.pattern==6)amp=t<.5?.02:t<1.0?.4:t<1.5?.03:.2;
   double freq=440*std::exp2((inputMidi-69)/12);phase+=2*pi*freq/c.sr;
   double x=amp*(std::sin(phase)+.4*std::sin(2*phase+.3)+.175*std::sin(3*phase+.7));
   if(c.pattern==5){x=0;for(int h=1;h<=18&&h*freq<c.sr*.45;++h){double f=h*freq;double form=.05+.8*std::exp(-.5*std::pow((f-850)/180,2))+.6*std::exp(-.5*std::pow((f-2400)/350,2));x+=.22*form/std::sqrt(h)*std::sin(h*phase+.1*h);}}
   if(c.pattern==7)x=i%240==0?.6:0;
   if(c.pattern==8)x=.12*std::sin(2*phase)+.25*std::sin(3*phase)+.025*std::sin(phase); // weak fundamental
   if(c.pattern==9)x=.0002*std::sin(phase);
   if(c.pattern==10){x=i<48000?x:0;if(i==int(c.sr*.7))x=std::numeric_limits<double>::quiet_NaN();if(i==int(c.sr*.9))x=std::numeric_limits<double>::infinity();}
   float y=dsp.processSample(float(x),c.voice,0,2,notes,micro,false,c.formant,c.throat,false,c.transpose,0,50,c.retune,c.flex,0,c.human,false,0,5.5,500,500,30,0,30,70,c.mix,c.freeze,c.bypass);audio.write((char*)&y,4);
   if(!std::isfinite(y))++bad;peak=std::max(peak,double(std::abs(y)));diff=std::max(diff,std::abs(y-last));last=y;rms+=double(y)*y;if(dsp.isCurrentlyVoiced())++voiced;
   if(dsp.formantRecoverySamples==int(c.sr*.05))++recovery;
   int target=int(std::lround(dsp.getTargetMidiNote()));if(target>=0&&lastTarget>=0&&target!=lastTarget)++switches;if(target>=0)lastTarget=target;
   if(i>int(c.sr*1.5)&&dsp.isCurrentlyVoiced()){detSum+=dsp.getDetectedMidiNote();commandSum+=dsp.getCentsCorrection();++finalVoiced;}
   if(trace&&i%std::max(1,int(c.sr*.002))==0)trace<<t<<","<<inputMidi<<","<<dsp.getDetectedMidiNote()<<","<<dsp.getTargetMidiNote()<<","<<dsp.getCentsCorrection()<<","<<dsp.resampleRatio<<","<<dsp.isCurrentlyVoiced()<<","<<dsp.formantRecoverySamples<<"\n";
  }double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-blockStart).count();maxBlock=std::max(maxBlock,seconds);totalBlock+=seconds;++blockCount;}
  double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();if(!first)std::cout<<",\n";first=false;
  std::cout<<std::setprecision(9)<<"{\"name\":\""<<c.name<<"\",\"sample_rate\":"<<c.sr<<",\"base_midi\":"<<c.baseMidi<<",\"offset_cents\":"<<c.offset<<",\"transpose\":"<<c.transpose<<",\"formant\":"<<c.formant<<",\"peak\":"<<peak<<",\"max_adjacent_difference\":"<<diff<<",\"rms\":"<<std::sqrt(rms/n)<<",\"nonfinite\":"<<bad<<",\"voiced_fraction\":"<<double(voiced)/n<<",\"last_detected_midi_mean\":"<<(finalVoiced?detSum/finalVoiced:-1)<<",\"last_command_cents_mean\":"<<(finalVoiced?commandSum/finalVoiced:0)<<",\"formant_recoveries\":"<<recovery<<",\"target_switches\":"<<switches<<",\"wall_seconds\":"<<elapsed<<",\"max64_block_ms\":"<<maxBlock*1000<<",\"mean64_block_ms\":"<<totalBlock/blockCount*1000<<"}";
 }
 std::cout<<"\n]}\n";
}
