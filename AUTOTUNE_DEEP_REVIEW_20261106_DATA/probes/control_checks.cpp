#include "ChannelOnly.h"
#include <cstdio>
#include <cstdlib>
#include <new>
#include <limits>
static int errors=0,checks=0,allocations=0;static bool countAlloc=false;
void* operator new(std::size_t n){if(countAlloc)++allocations;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}void operator delete(void* p,std::size_t)noexcept{std::free(p);}void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
#define CHECK(x) do{++checks;if(!(x)){if(errors<12)std::printf("FAIL %d %s\n",__LINE__,#x);++errors;}}while(false)
struct Param{double mix=100,trans=0;bool bypass=false,formant=false;};
float sample(TAutoTuneChannelDSP& d,float x,const std::array<int,12>& states,Param p={}){std::array<int,12> micro{};return d.processSample(x,1,9,2,states,micro,false,p.formant,100,false,p.trans,0,50,25,0,0,35,false,0,5.5,500,500,30,0,30,70,p.mix,-1,p.bypass);}
int main(){
 for(double sr:{44100.,48000.,96000.,192000.}){
  std::array<int,12> notes{};notes.fill(1);TAutoTuneChannelDSP d;d.prepare(sr);int delay=TAutoTuneChannelDSP::liveLatencySamples(sr);float peak=0;
  for(int i=0;i<delay+20;++i){float y=sample(d,i==0?.5f:0.f,notes);CHECK(std::abs(y-(i==delay?.5f:0.f))<1e-7);peak=std::max(peak,std::abs(y));}CHECK(peak==.5f);
  // Same neutral impulse response after reset, independent of prior audio history.
  d.reset();for(int i=0;i<delay+20;++i)CHECK(std::abs(sample(d,i==0?.5f:0.f,notes)-(i==delay?.5f:0.f))<1e-7);
  notes.fill(0);Param p;p.bypass=true;d.reset();std::vector<float> input(size_t(sr*.1));for(size_t i=0;i<input.size();++i)input[i]=.15f*std::sin(float(i*.11));for(size_t i=0;i<input.size();++i){float y=sample(d,input[i],notes,p);if(i>size_t(sr*.06))CHECK(std::abs(y-input[i-size_t(delay)])<1e-6);}
  d.reset();sample(d,std::numeric_limits<float>::quiet_NaN(),notes);sample(d,std::numeric_limits<float>::infinity(),notes);for(int i=0;i<1000;++i)CHECK(std::isfinite(sample(d,.1f*std::sin(float(i*.1)),notes)));
 }
 std::array<int,12> notes{};TAutoTuneChannelDSP a,b;a.prepare(48000);b.prepare(48000);std::vector<float>x(48000),y(48000);for(size_t i=0;i<x.size();++i)x[i]=.1f*std::sin(float(i*.029));
 for(size_t i=0;i<x.size();++i)y[i]=sample(a,x[i],notes);
 for(size_t i=0;i<x.size();++i)CHECK(sample(b,x[i],notes)==y[i]);
 a.reset();allocations=0;countAlloc=true;for(int i=0;i<12800;++i)sample(a,.1f*std::sin(float(i*.029)),notes);countAlloc=false;CHECK(allocations==0);
 std::printf("CONTROL CHECKS: %d checks, %d failures; C++ new/new[] during 12800 DSP samples: %d\n",checks,errors,allocations);return errors?1:0;
}
