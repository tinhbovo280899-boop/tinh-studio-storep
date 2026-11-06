#include "ReaderFixtureOnly.h"
#include <fstream>
#include <iostream>
int main(int argc,char**argv){if(argc<2)return 2;std::ofstream(argv[1])<<R"(<PROPERTIES><VALUE name="KeyClass" val="0"/><VALUE name="KeyType" val="0"/></PROPERTIES>)";
 AutoKeyHandoffReader reader;reader.addExtraSearchPath(argv[1]);reader.setAutoSyncEnabled(false);AutoKeyHandoffReader::HandoffData data;
 int savedKey=10,savedScale=1;
 bool normal=reader.pollForUpdate(data,false);std::cout<<"sync_off_normal_poll_updates="<<normal<<"\n";
 // Mirrors constructor pendingHandoffSync=2 followed by the first timer tick.
 int pending=2;bool forced=reader.pollForUpdate(data,pending==2);if(forced){savedKey=data.keyClass;savedScale=data.scaleIndex;}
 std::cout<<"sync_off_first_forced_tick_updates="<<forced<<" saved_key_after="<<savedKey<<" saved_scale_after="<<savedScale<<"\n";
 return !normal&&forced&&savedKey==0&&savedScale==0?0:1;
}
