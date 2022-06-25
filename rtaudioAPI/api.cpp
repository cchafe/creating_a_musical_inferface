#include "api.h"

#include "rtaudio/RtAudio.h"
#include <cctype>
#include <cstdlib>
#include <iostream>

API::API()
{

}

/******************************************/
/*
  apinames.cpp
  by Jean Pierre Cimalando, 2018.

  This program tests parts of RtAudio related
  to API names, the conversion from name to API
  and vice-versa.
*/
/******************************************/

int API::test_cpp() {
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi( apis );

    // ensure the known APIs return valid names
    std::cout << "API names by identifier (C++):\n";
    for ( size_t i = 0; i < apis.size() ; ++i ) {
        const std::string name = RtAudio::getApiName(apis[i]);
        if (name.empty()) {
            std::cout << "Invalid name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        const std::string displayName = RtAudio::getApiDisplayName(apis[i]);
        if (displayName.empty()) {
            std::cout << "Invalid display name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        std::cout << "* " << (int)apis[i] << " '" << name << "': '" << displayName << "'\n";
    }

    // ensure unknown APIs return the empty string
    {
        const std::string name = RtAudio::getApiName((RtAudio::Api)-1);
        if (!name.empty()) {
            std::cout << "Bad string for invalid API '" << name << "'\n";
            exit(1);
        }
        const std::string displayName = RtAudio::getApiDisplayName((RtAudio::Api)-1);
        if (displayName!="Unknown") {
            std::cout << "Bad display string for invalid API '" << displayName << "'\n";
            exit(1);
        }
    }

    // try getting API identifier by name
    std::cout << "API identifiers by name (C++):\n";
    for ( size_t i = 0; i < apis.size() ; ++i ) {
        std::string name = RtAudio::getApiName(apis[i]);
        if ( RtAudio::getCompiledApiByName(name) != apis[i] ) {
            std::cout << "Bad identifier for API '" << name << "'\n";
            exit( 1 );
        }
        std::cout << "* '" << name << "': " << (int)apis[i] << "\n";

        for ( size_t j = 0; j < name.size(); ++j )
            name[j] = (j & 1) ? toupper(name[j]) : tolower(name[j]);
        RtAudio::Api api = RtAudio::getCompiledApiByName(name);
        if ( api != RtAudio::UNSPECIFIED ) {
            std::cout << "Identifier " << (int)api << " for invalid API '" << name << "'\n";
            exit( 1 );
        }
    }

    // try getting an API identifier by unknown name
    {
        RtAudio::Api api;
        api = RtAudio::getCompiledApiByName("");
        if ( api != RtAudio::UNSPECIFIED ) {
            std::cout << "Bad identifier for unknown API name\n";
            exit( 1 );
        }
    }

    return 0;
}
