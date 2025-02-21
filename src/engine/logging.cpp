/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2021 - 2022                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <ctime>

#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined( MACOS_APP_BUNDLE )
#include <syslog.h>
#endif

#include "logging.h"
#include "system.h"

namespace
{
    int g_debug = DBG_ALL_WARN + DBG_ALL_INFO;

    bool textSupportMode = false;

#if defined( _WIN32 )
    // Sets the Windows console codepage to the system codepage
    class ConsoleCPSwitcher
    {
    public:
        ConsoleCPSwitcher()
            : _consoleOutputCP( GetConsoleOutputCP() )
        {
            if ( _consoleOutputCP > 0 ) {
                SetConsoleOutputCP( GetACP() );
            }
        }

        ConsoleCPSwitcher( const ConsoleCPSwitcher & ) = delete;

        ~ConsoleCPSwitcher()
        {
            if ( _consoleOutputCP > 0 ) {
                SetConsoleOutputCP( _consoleOutputCP );
            }
        }

        ConsoleCPSwitcher & operator=( const ConsoleCPSwitcher & ) = delete;

    private:
        const UINT _consoleOutputCP;
    };

    const ConsoleCPSwitcher consoleCPSwitcher;
#endif
}

namespace Logging
{
#if defined( TARGET_NINTENDO_SWITCH ) || defined( _WIN32 )
    std::ofstream logFile;
    // This mutex protects operations with logFile
    std::mutex logMutex;
#endif

    const char * GetDebugOptionName( const int name )
    {
        if ( name & DBG_ENGINE )
            return "DBG_ENGINE";
        else if ( name & DBG_GAME )
            return "DBG_GAME";
        else if ( name & DBG_BATTLE )
            return "DBG_BATTLE";
        else if ( name & DBG_AI )
            return "DBG_AI";
        else if ( name & DBG_NETWORK )
            return "DBG_NETWORK";
        else if ( name & DBG_OTHER )
            return "DBG_OTHER";
        else if ( name & DBG_DEVEL )
            return "DBG_DEVEL";

        return "";
    }

    std::string GetTimeString()
    {
        const tm tmi = System::GetTM( std::time( nullptr ) );

        char buf[13] = {0};
        std::strftime( buf, sizeof( buf ) - 1, "%X", &tmi );

        return std::string( buf );
    }

    void InitLog()
    {
#if defined( TARGET_NINTENDO_SWITCH )
        const std::scoped_lock<std::mutex> lock( logMutex );

        logFile.open( "fheroes2.log", std::ofstream::out );
#elif defined( _WIN32 )
        const std::scoped_lock<std::mutex> lock( logMutex );
        const std::string logPath( System::ConcatePath( System::GetConfigDirectory( "fheroes2" ), "fheroes2.log" ) );

        logFile.open( logPath, std::ofstream::out );
#elif defined( MACOS_APP_BUNDLE )
        openlog( "fheroes2", LOG_CONS | LOG_NDELAY, LOG_USER );
        setlogmask( LOG_UPTO( LOG_WARNING ) );
#endif
    }

    void SetDebugLevel( const int debugLevel )
    {
        g_debug = debugLevel;
    }

    void setTextSupportMode( const bool enableTextSupportMode )
    {
        textSupportMode = enableTextSupportMode;
    }

    bool isTextSupportModeEnabled()
    {
        return textSupportMode;
    }
}

bool IS_DEBUG( const int name, const int level )
{
    return ( ( DBG_ENGINE & name ) && ( ( DBG_ENGINE & g_debug ) >> 2 ) >= level ) || ( ( DBG_GAME & name ) && ( ( DBG_GAME & g_debug ) >> 4 ) >= level )
           || ( ( DBG_BATTLE & name ) && ( ( DBG_BATTLE & g_debug ) >> 6 ) >= level ) || ( ( DBG_AI & name ) && ( ( DBG_AI & g_debug ) >> 8 ) >= level )
           || ( ( DBG_NETWORK & name ) && ( ( DBG_NETWORK & g_debug ) >> 10 ) >= level ) || ( ( DBG_DEVEL & name ) && ( ( DBG_DEVEL & g_debug ) >> 12 ) >= level );
}
