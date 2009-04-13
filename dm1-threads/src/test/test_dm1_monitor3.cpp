/**
 * This program is from the pthreads-win32 package.
 * It is derived from the program called tennisb.cpp described by Alexander
 * Terekhov in the README.CV file. 
 * The pthreads-win32 package can be obtained from:
 * http://sources.redhat.com/pthreads-win32/
 */

#include "dm1/monitor.h"
//#include <iostream.h>
#include <stdio.h>

namespace test_dm1_monitor3 {

using namespace dm1;

enum GAME_STATE {

  START_GAME,
  PLAYER_A,     // Player A playes the ball
  PLAYER_B,     // Player B playes the ball
  GAME_OVER,
  ONE_PLAYER_GONE,
  BOTH_PLAYERS_GONE

};

enum GAME_STATE             YGameState;
Monitor                     YGameStateChange;

class YPlayerA : public Runnable {
public:
	void run();
};

class YPlayerB : public Runnable {
public:
	void run();
};

void YPlayerA::run() {

  // For access to game state variable
  YGameStateChange.lock();

  // Play loop
  while ( YGameState < GAME_OVER ) {

    // Play the ball
    fprintf(stdout, "\nPLAYER-A\n");
    //cout << endl << "PLAYER-A" << endl;

    // Now its PLAYER-B's turn
    YGameState = PLAYER_B;

    // Signal to PLAYER-B that now it is his turn
    YGameStateChange.notifyAll();

    // Wait until PLAYER-B finishes playing the ball
    do {

      YGameStateChange.wait();

      if ( PLAYER_B == YGameState )
        //cout << endl << "----PLAYER-A: SPURIOUS WAKEUP!!!" << endl;
        fprintf(stdout, "\n----PLAYER-A: SPURIOUS WAKEUP!!!\n");

    } while ( PLAYER_B == YGameState );

  }

  // PLAYER-A gone
  YGameState = (GAME_STATE)(YGameState+1);
  //cout << endl << "PLAYER-A GONE" << endl;
  fprintf(stdout, "\nPLAYER-A GONE\n");

  // No more access to state variable needed
  YGameStateChange.unlock();

  // Signal PLAYER-A gone event
  YGameStateChange.notifyAll();
}

void YPlayerB::run() {

  // For access to game state variable
  YGameStateChange.lock();

  // Play loop
  while ( YGameState < GAME_OVER ) {

    // Play the ball
    //cout << endl << "PLAYER-B" << endl;
    fprintf(stdout, "\nPLAYER-B\n");

    // Now its PLAYER-A's turn
    YGameState = PLAYER_A;

    // Signal to PLAYER-A that now it is his turn
    YGameStateChange.notifyAll();

    // Wait until PLAYER-A finishes playing the ball
    do {

      YGameStateChange.wait();

      if ( PLAYER_A == YGameState )
        //cout << endl << "----PLAYER-B: SPURIOUS WAKEUP!!!" << endl;
        fprintf(stdout, "\n----PLAYER-B: SPURIOUS WAKEUP!!!\n");

    } while ( PLAYER_A == YGameState );

  }

  // PLAYER-B gone
  YGameState = (GAME_STATE)(YGameState+1);
  //cout << endl << "PLAYER-B GONE" << endl;
  fprintf(stdout, "\nPLAYER-B GONE\n");

  // No more access to state variable needed
  YGameStateChange.unlock();

  // Signal PLAYER-B gone event
  YGameStateChange.notifyAll();
}


int test()
{
  setbuf(stdout,0);

  // Set initial state
  YGameState = START_GAME;

  // Create players
  // Create players
  Thread *playerA = ThreadFactory::createThread(new YPlayerA(), "playerA");
  Thread *playerB = ThreadFactory::createThread(new YPlayerB(), "playerB");

  playerA->start();
  playerB->start();

  // Give them 5 sec. to play
  sleep(5);

  // Make some noise
  YGameStateChange.lock();
  //cout << endl << "---Noise ON..." << endl;
  fprintf(stdout, "\n---Noise ON...\n");
  YGameStateChange.unlock();
  for ( int i = 0; i < 5000; i++ ) {
    YGameStateChange.notifyAll();
  }
  //cout << endl << "---Noise OFF" << endl;
  fprintf(stdout, "\n---Noise OFF\n");

  // Set game over state
  YGameStateChange.lock();
  YGameState = GAME_OVER;

  // Let them know
  YGameStateChange.notifyAll();

  // Wait for players to stop
  do {
    YGameStateChange.wait();
  } while ( YGameState < BOTH_PLAYERS_GONE );

  // Cleanup
  //cout << endl << "GAME OVER" << endl;
  fprintf(stdout, "\nGAME OVER\n");
  YGameStateChange.unlock();

  playerA->join();
  playerB->join();
  return 0;
}

}

