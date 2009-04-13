/**
 * This program is from the pthreads-win32 package.
 * It is derived from the program called tennis.cpp described by Alexander
 * Terekhov in the README.CV file. 
 * The pthreads-win32 package can be obtained from:
 * http://sources.redhat.com/pthreads-win32/
 */

#include "dm1/monitor.h"

#include <assert.h>
#include <stdio.h>

namespace test_dm1_monitor2 {

using namespace dm1;

enum GAME_STATE {

  START_GAME,
  PLAYER_A,     // Player A playes the ball
  PLAYER_B,     // Player B playes the ball
  GAME_OVER,
  ONE_PLAYER_GONE,
  BOTH_PLAYERS_GONE

};

enum GAME_STATE             XGameState;
Monitor                     XGameStateChange;

class XPlayerA : public Runnable {
public:
	void run();
};

class XPlayerB : public Runnable {
public:
	void run();
};

void XPlayerA::run() {

  // For access to game state variable
  XGameStateChange.lock();

  // Play loop
  while ( XGameState < GAME_OVER ) {

    // Play the ball
    fprintf(stdout, "\nPLAYER-A\n");
    //cout << endl << "PLAYER-A" << endl;

    // Now its PLAYER-B's turn
    XGameState = PLAYER_B;

    // Signal to PLAYER-B that now it is his turn
    XGameStateChange.notify();

    // Wait until PLAYER-B finishes playing the ball
    do {

      XGameStateChange.wait();

      if ( PLAYER_B == XGameState ) {
        fprintf(stdout, "\n----PLAYER-A: SPURIOUS WAKEUP!!!\n");
        //cout << endl << "----PLAYER-A: SPURIOUS WAKEUP!!!" << endl;
	assert(0);
      }

    } while ( PLAYER_B == XGameState );

  }

  // PLAYER-A gone
  XGameState = (GAME_STATE)(XGameState+1);
  fprintf(stdout, "\nPLAYER-A GONE\n");
  //cout << endl << "PLAYER-A GONE" << endl;

  // No more access to state variable needed
  XGameStateChange.unlock();

  // Signal PLAYER-A gone event
  XGameStateChange.notifyAll();
}

void XPlayerB::run() {

  // For access to game state variable
  XGameStateChange.lock();

  // Play loop
  while ( XGameState < GAME_OVER ) {

    // Play the ball
    fprintf(stdout, "\nPLAYER-B\n");
    //cout << endl << "PLAYER-B" << endl;

    // Now its PLAYER-A's turn
    XGameState = PLAYER_A;

    // Signal to PLAYER-A that now it is his turn
    XGameStateChange.notify();

    // Wait until PLAYER-A finishes playing the ball
    do {

      XGameStateChange.wait();

      if ( PLAYER_A == XGameState ) {
        fprintf(stdout, "\n----PLAYER-B: SPURIOUS WAKEUP!!!\n");
        //cout << endl << "----PLAYER-B: SPURIOUS WAKEUP!!!" << endl;
	assert(0);
      }

    } while ( PLAYER_A == XGameState );

  }

  // PLAYER-B gone
  XGameState = (GAME_STATE)(XGameState+1);
  fprintf(stdout, "\nPLAYER-B GONE\n");
  //cout << endl << "PLAYER-B GONE" << endl;

  // No more access to state variable needed
  XGameStateChange.unlock();

  // Signal PLAYER-B gone event
  XGameStateChange.notifyAll();
}


int test()
{

  // Set initial state
  XGameState = START_GAME;

  // Create players
  Thread *playerA = ThreadFactory::createThread(new XPlayerA(), "playerA");
  Thread *playerB = ThreadFactory::createThread(new XPlayerB(), "playerB");

  playerA->start();
  playerB->start();

  // Give them 5 sec. to play
  sleep(5);

  // Set game over state
  XGameStateChange.lock();
  XGameState = GAME_OVER;

  // Let them know
  XGameStateChange.notifyAll();

  // Wait for players to stop
  do {
    XGameStateChange.wait();
  } while ( XGameState < BOTH_PLAYERS_GONE );

  // Cleanup
  fprintf(stdout, "\nGAME OVER\n");
  //cout << endl << "GAME OVER" << endl;
  XGameStateChange.unlock();

  playerA->join();
  playerB->join();

  return 0;
}

}

