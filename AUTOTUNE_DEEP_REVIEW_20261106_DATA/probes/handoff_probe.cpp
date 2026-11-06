#include "AutoKeyHandoffReader.h"
#include <iostream>
int main(){struct Test{const char* label;const char* value;};
 Test tests[]{
 {"xml-full",R"(<PROPERTIES><VALUE name="KeyClass" val="9"/><VALUE name="KeyType" val="1"/></PROPERTIES>)"},
 {"key-only",R"(<VALUE name="KeyClass" val="9"/>)"},
 {"scale-only",R"(<VALUE name="KeyType" val="1"/>)"},
 {"json-minified",R"({"Key":"A","Scale":"Minor"})"},
 {"json-pretty","{\n\"Key\":\"A\",\n\"Scale\":\"Minor\"\n}"},
 {"unicode-flat",u8"Key=B♭\nScale=Major"},
 {"late-scale-name","Key=A\nScale=Hungarian Minor"},
 {"late-scale-number","Key=A\nScale=17"}
 };
 for(auto t:tests){auto d=AutoKeyHandoffReader::parseContent(t.value);std::cout<<t.label<<" valid="<<d.valid<<" key="<<d.keyClass<<" scale="<<d.scaleIndex<<"\n";}
}
