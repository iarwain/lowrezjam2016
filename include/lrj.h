#ifndef __LRJ_H__
#define __LRJ_H__

//! Includes
#define __NO_SCROLLED__
#include "Scroll.h"


//! LRJ
class LRJ : public Scroll<LRJ>
{
public:

  enum GameState
  {
    GameStateInit = 0,
    GameStateRun,
    GameStateGameOver,
    GameStateReset,

    GameStateNumber,

    GameStateNone = orxENUM_NONE
  };


                orxDOUBLE       GetTime() const             {return mdTime;}
                GameState       GetGameState() const        {return meGameState;}

                orxSTATUS       Load();
                orxSTATUS       Save();

                void            DeleteRunTimeObject(const orxSTRING _zObjectName);


private:

                orxSTATUS       Bootstrap() const;

                void            Reset();

                void            InitSplash();

                void            UpdateInteraction(const orxCLOCK_INFO &_rstInfo);
                void            UpdateShader(const orxCLOCK_INFO &_rstInfo);

                void            UpdateGame(const orxCLOCK_INFO &_rstInfo);
                void            UpdatePause(const orxCLOCK_INFO &_rstInfo);
                void            UpdateInput(const orxCLOCK_INFO &_rstInfo);

                void            Update(const orxCLOCK_INFO &_rstInfo);
                void            CameraUpdate(const orxCLOCK_INFO &_rstInfo);

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            BindObjects();


private:

                orxVECTOR       mvMousePosition;
                orxU64          mu64InteractionID;
                orxDOUBLE       mdTime;
                GameState       meGameState;
};

#endif // __LRJ_H__
