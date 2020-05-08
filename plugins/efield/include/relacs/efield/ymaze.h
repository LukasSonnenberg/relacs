/*
  efield/ymaze.h
  Repro fro controlling discrimination experiments in a y-maze

  RELACS - Relaxed ELectrophysiological data Acquisition, Control, and Stimulation
  Copyright (C) 2002-2015 Jan Benda <jan.benda@uni-tuebingen.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  RELACS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _RELACS_EFIELD_YMAZE_H_
#define _RELACS_EFIELD_YMAZE_H_ 1

#include <relacs/repro.h>
#include <relacs/efield/traces.h>
#include <relacs/efield/eodtools.h>

#include <QLabel>
#include <QObject>
#include <QGridLayout>


using namespace relacs;

namespace efield {


/*!
\class YMaze
\brief [RePro] Repro fro controlling discrimination experiments in a y-maze
\author Jan Grewe
\version 1.0 (Mar 18, 2020)
*/
enum class MazeOrientation {UPRIGHT = 0, BOTTOMUP = 1, LEFT = 2, RIGHT = 3};
enum class MazeArm {NONE = -1, A = 0, B = 1, C = 2};
enum class ArmCondition {REWARDED = 0, UNREWARDED = 1, NEUTRAL = 2};
enum class BtnActions {NEXT_TRIAL = 11, START_TRIAL = 12, STOP_TRIAL = 13, STIM_READY = 14};

struct MazeCondition {
  MazeArm rewarded;
  MazeArm unrewarded;
  MazeArm neutral;
};

struct StimulusCondition {
  double eodf;
  double rewardedFreq;
  double unrewardedFreq;
  double rewardedAmplitude;
  double unrewardedAmplitude;
  bool valid;
};

struct TrialCondition {
  MazeCondition mazeCondition;
  StimulusCondition stimCondition;
};
  
class YMazeSketch : public QLabel
{
  Q_OBJECT;

public:
  YMazeSketch( MazeOrientation orientation=MazeOrientation::UPRIGHT, QWidget *parent = nullptr );

  void setCondition( const MazeCondition &mazeCondition );

private:
  void setupUI();
  void drawSketch();
  void resizeEvent( QResizeEvent *event ) override; 

  MazeOrientation orientation;
  MazeArm lastRewarded = MazeArm::NONE;
  QLabel *l, *lbl1, *lbl2, *lbl3;
  QLabel *labels[3];
  QStringList colors = {"green","red","grey"};
};

  
  class YMaze : public RePro, public Traces, public EODTools
{
  Q_OBJECT

public:

  YMaze( void );
  virtual int main( void );

private:
  YMazeSketch *sketch;
  double duration, eodf;
  double rewardedFreq;
  double freqRangeMin, freqRangeMax, minFreqDiff, deltaf;
  int numberOfTrials;
  int lastRewardPosition = -1;
  int currentRewardPosition;
  bool start;
  QLabel *conditionA, *conditionApast, *conditionB, *conditionBpast, *conditionC, *conditionCpast;
  QPushButton *nextBtn, *startBtn, *stopBtn;
  OutData signal_A, signal_B, signal_C;
  OutList outList;
  std::map<std::string, MazeArm> channelArmMap = {{"Arm-A", MazeArm::A},
						  {"Arm-B", MazeArm::B},
						  {"Arm-C", MazeArm::C}};
  std::map<MazeArm, OutData> armSignalMap = {{MazeArm::A, signal_A},
					    {MazeArm::B, signal_B},
					    {MazeArm::C, signal_C}};

  void setupTable( QGridLayout *grid );
  void updateTable( const TrialCondition &tc );
  void updateUI( const TrialCondition &tc );

  void populateOptions();
  bool configureOutputTraces();
  
  MazeCondition nextMazeCondition();
  TrialCondition nextTrialCondition();
  StimulusCondition nextStimulusConditions();

  void createStimuli( const TrialCondition &tc );
  bool estimateEodFrequency( double &fisheodf );
  bool drawNonRewardedFrequency( double &freq );
				       
private slots:
  void startTrial();
  void stopTrial();
  void prepareNextTrial();
  
protected:
  virtual void customEvent( QEvent *qce );
};


}; /* namespace efield */

#endif /* ! _RELACS_EFIELD_YMAZE_H_ */
