// -*-c++-*-

/*!
  \file log_player.cpp
  \brief log player class Header File.
*/

/*
 *Copyright:

 Copyright (C) The RoboCup Soccer Server Maintenance Group.
 Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QApplication>
#include <QTimer>

#include "disp_holder.h"
#include "log_player.h"
#include "options.h"

#include <iostream>

/*-------------------------------------------------------------------*/
/*!

*/
LogPlayer::LogPlayer( DispHolder & disp_holder,
                      QObject * parent )
    : QObject( parent ),
      M_disp_holder( disp_holder ),
      M_timer( new QTimer( this ) ),
      M_forward( true ),
      M_live_mode( false ),
      M_first_cache( false ),
      M_need_caching( false )
{
    connect( M_timer, SIGNAL( timeout() ),
             this, SLOT( handleTimer() ) );

    M_timer->setInterval( Options::instance().timerInterval() );
}

/*-------------------------------------------------------------------*/
/*!

*/
LogPlayer::~LogPlayer()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::clear()
{
    if ( M_timer->isActive() )
    {
        M_timer->stop();
    }

    M_forward = true;
    M_live_mode = false;
    M_first_cache = false;
    M_need_caching = false;
    M_timer->setInterval( Options::instance().timerInterval() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::handleTimer()
{
    if ( M_forward )
    {
        stepForwardImpl();
    }
    else
    {
        stepBackImpl();
    }

    adjustTimer();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
LogPlayer::isLiveMode() const
{
    return M_live_mode;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::stepBackImpl()
{
    if ( M_disp_holder.setIndexStepBack() )
    {
        emit updated();
    }
    else
    {
        M_timer->stop();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::stepForwardImpl()
{
    if ( M_disp_holder.setIndexStepForward() )
    {
        emit updated();
    }
    else
    {
        M_timer->stop();

        if ( M_disp_holder.playmode() == rcss::rcg::PM_TimeOver
             && Options::instance().autoQuitMode() )
        {
            int wait_msec = ( Options::instance().autoQuitWait() > 0
                              ? Options::instance().autoQuitWait() * 1000
                              : 100 );
            QTimer::singleShot( wait_msec,
                                qApp, SLOT( quit() ) );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::stepBack()
{
    M_live_mode = false;
    M_forward = false;
    M_timer->stop();

    stepBackImpl();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::stepForward()
{
    M_live_mode = false;
    M_forward = true;
    M_timer->stop();

    stepForwardImpl();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::playOrStop()
{
    M_live_mode = false;

    if ( M_timer->isActive() )
    {
        M_timer->stop();
    }
    else if ( M_forward )
    {
        playForward();
    }
    else
    {
        playBack();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::stop()
{
    M_live_mode = false;
    M_timer->stop();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::playBack()
{
    M_live_mode = false;
    M_forward = false;

    if ( ! M_timer->isActive()
         || M_timer->interval() != Options::instance().timerInterval() )
    {
        M_timer->start( Options::instance().timerInterval() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::playForward()
{
    M_live_mode = false;
    M_forward = true;

    if ( ! M_timer->isActive()
         || M_timer->interval() != Options::instance().timerInterval() )
    {
        M_timer->start( Options::instance().timerInterval() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::accelerateBack()
{
    int interval = 0;
    if ( M_forward
         || ! M_timer->isActive() )
    {
        interval = Options::instance().timerInterval() / 2;
    }
    else
    {
        interval = M_timer->interval() / 2;
        if ( interval < 10 ) interval = 10;
    }

    M_live_mode = false;
    M_forward = false;
    M_timer->start( interval );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::accelerateForward()
{
    int interval = 0;
    if ( ! M_forward
        || ! M_timer->isActive() )
    {
        interval = Options::instance().timerInterval() / 2;
    }
    else
    {
        interval = M_timer->interval() / 2;
        if ( interval < 10 ) interval = 10;
    }

    M_live_mode = false;
    M_forward = true;
    M_timer->start( interval );
}

/*-------------------------------------------------------------------*/
/*!

*/
// void
// LogPlayer::goToPrevScore()
// {
//     M_live_mode = false;

//     const DispHolder & holder = M_main_data.dispHolder();

//     const std::size_t cur_idx = M_main_data.index();
//     std::vector< std::size_t >::const_reverse_iterator rend = holder.scoreChangedIndex().rend();
//     for ( std::vector< std::size_t >::const_reverse_iterator it = holder.scoreChangedIndex().rbegin();
//           it != rend;
//           ++it )
//     {
//         if ( *it < cur_idx )
//         {
//             std::size_t new_idx = *it;
//             new_idx -= ( new_idx < 50 ? new_idx : 50 );
//             goToIndex( new_idx );
//             return;
//         }
//     }
// }

/*-------------------------------------------------------------------*/
/*!

*/
// void
// LogPlayer::goToNextScore()
// {
//     M_live_mode = false;

//     const DispHolder & holder = M_main_data.dispHolder();

//     const std::size_t cur_idx = M_main_data.index();
//     std::vector< std::size_t >::const_iterator end = holder.scoreChangedIndex().end();
//     for ( std::vector< std::size_t >::const_iterator it = holder.scoreChangedIndex().begin();
//           it != end;
//           ++it )
//     {
//         if ( 50 < *it && cur_idx < *it - 50 )
//         {
//             std::size_t new_idx = *it;
//             new_idx -= ( new_idx < 50 ? 0 : 50 );
//             goToIndex( new_idx );
//             return;
//         }
//     }
// }

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::goToFirst()
{
    if ( M_disp_holder.setIndexFirst() )
    {
        M_live_mode = false;
        M_timer->stop();

        emit updated();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::goToLast()
{
    if ( M_disp_holder.setIndexLast() )
    {
        M_live_mode = false;
        M_timer->stop();

        emit updated();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::decelerate()
{
    if ( M_timer->isActive() )
    {
        int interval = M_timer->interval() * 2;
        if ( 5000 < interval ) interval = 5000;
        M_timer->start( interval );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::accelerate()
{
    if ( M_timer->isActive() )
    {
        int interval = M_timer->interval() / 2;
        if ( interval < 5 ) interval = 5;
        M_timer->start( interval );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::goToIndex( int index )
{
    if ( M_disp_holder.setIndex( index ) )
    {
        M_live_mode = false;
        //M_timer->stop();

        emit updated();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::goToCycle( int cycle )
{
    if ( M_disp_holder.setCycle( cycle ) )
    {
        M_live_mode = false;
        //M_timer->stop();

        emit updated();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::showLive()
{
    if ( Options::instance().bufferingMode() )
    {
        if ( M_disp_holder.setIndexLast() )
        {
            M_timer->stop();

            emit updated();
        }
    }
    else
    {
        emit updated();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::setLiveMode()
{
    M_disp_holder.setIndexLast();
    M_live_mode = true;
    M_timer->stop();

    //emit updated();
}


/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::startTimer()
{
    if ( ! M_disp_holder.dispCont().empty()
         && ! M_timer->isActive() )
    {
        M_timer->start( Options::instance().timerInterval() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LogPlayer::adjustTimer()
{
    const Options & opt = Options::instance();

    M_live_mode = false;

    const int buffer_size = M_disp_holder.dispCont().size();
    size_t current = M_disp_holder.currentIndex();

    if ( buffer_size == 0 )
    {
        return;
    }

    if ( current == DispHolder::INVALID_INDEX )
    {
        current = 0;
    }

    const rcss::rcg::ServerParamT & SP = M_disp_holder.serverParam();
    const int cache_size = std::max( 1, opt.bufferSize() );
    const int current_cache = buffer_size - current - 1;

    if ( ! M_first_cache )
    {
        if ( current_cache >= cache_size )
        {
            M_first_cache = true;
        }
    }

    int interval = std::min( opt.timerInterval(),
                             SP.simulator_step_ );

    if ( ! M_first_cache )
    {
        interval = opt.timerInterval() * 5;
    }
    else if ( current_cache >= cache_size )
    {
        M_need_caching = false;
    }
    else if ( M_need_caching )
    {
        interval = std::max( opt.timerInterval() * 11 / 10,
                             SP.simulator_step_ * 11 / 10 );
    }
    else if ( current_cache <= cache_size / 2
              && M_disp_holder.playmode() != rcss::rcg::PM_TimeOver )
    {
        M_need_caching = true;
        interval = std::max( opt.timerInterval() * 11 / 10,
                             SP.simulator_step_ * 11 / 10 );
    }
    else if ( current_cache > cache_size / 2 )
    {
        interval = std::max( opt.timerInterval(),
                             SP.simulator_step_ );
    }
#if 0
    else if ( cache_size > 1
              && current_cache < cache_size
              && M_disp_holder.playmode() != rcss::rcg::PM_TimeOver )
    {
        if ( M_timer->interval() <= opt.timerInterval()
             && current_cache > cache_size * 0.8 )
        {
            // keep default interval
        }
        else
        {
            const int max_timer_interval = opt.timerInterval() * 3 / 2;

            if ( current_cache <= 1 )
            {
                interval = max_timer_interval;
            }
            else
            {
                double rate
                    = 1.0
                    - rint( static_cast< double >( current_cache ) / static_cast< double >( cache_size )
                            / 0.2 ) * 0.2;
                int interval_offset = max_timer_interval - opt.timerInterval();
                interval += static_cast< int >( rint( interval_offset * rate ) );
                interval = std::min( max_timer_interval, interval );
                std::cerr << "interval_offset=" << interval_offset
                          << " rate = " << rate
                          << " value=" << static_cast< int >( rint( interval_offset * rate ) )
                          << std::endl;
            }
        }
    }
#endif

#if 0
    if ( M_timer->interval() != interval )
    {
        std::cerr << "change interval " << interval << std::endl;
    }
#endif

//     std::cout << "disp_size=" << buffer_size
//               << " current_cache=" << current_cache
//               << " interval=" << interval
//               << std::endl;

    if ( M_timer->interval() != interval
         || ! M_timer->isActive() )
    {
        M_timer->start( interval );
    }
}
