#include "ChannelOnly.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
int main(int argc,char**argv){if(argc<7)return 2;double sr=std::stod(argv[2]),offset=std::stod(argv[3]),flex=std::stod(argv[4]),human=std::stod(argv[5]),retune=std::stod(argv[6]);int mode=argc>7?std::stoi(argv[7]):0;double trans=argc>8?std::stod(argv[8]):0;
 std::array<int,12> states{},micro{};states.fill(2);states[9]=0; // isolate A only; target A3=220 Hz, no target ambiguity
 TAutoTuneChannelDSP dsp;dsp.prepare(sr);double phase=0;int n=int(sr*2);std::ofstream output(argv[1],std::ios::binary);double detect=0,correct=0,target=0;int voiced=0;
 for(int i=0;i<n;++i){double t=i/sr;double cents=offset;
 if(mode==1)cents+=25*std::sin(6.283185307179586*5.5*t);
 if(mode==2)cents=offset*(t<.6?1:t<1.2?-1:1);
 double freq=220*std::exp2(cents/1200);phase+=6.283185307179586*freq/sr;
 double input=.2*std::sin(phase)+.08*std::sin(2*phase+.3)+.035*std::sin(3*phase+.7);
 float out=dsp.processSample(float(input),1,9,2,states,micro,false,false,100,false,trans,0,50,retune,flex,0,human,false,0,5.5,500,500,30,0,30,70,100);
 output.write((char*)&out,4);if(i>int(sr*1.5)&&dsp.isCurrentlyVoiced()){detect+=dsp.getDetectedMidiNote();correct+=dsp.getCentsCorrection();target+=dsp.getTargetMidiNote();++voiced;}
 }
 std::cout<<std::setprecision(10)<<"{\"voiced_samples\":"<<voiced<<",\"avg_detected_midi\":"<<(voiced?detect/voiced:-1)<<",\"avg_command_correction_cents\":"<<(voiced?correct/voiced:0)<<",\"avg_target_midi\":"<<(voiced?target/voiced:-1)<<"}\n";
}
