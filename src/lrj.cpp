//! Includes
#define __SCROLL_IMPL__
#include "lrj.h"
#undef __SCROLL_IMPL__

#include "orxArchive.c"


//! Defines


//! Constants


//! Structs/Classes
class Player : public ScrollObject
{
public:

  orxBOOL IsInputActive(const orxSTRING _zInput, orxBOOL _bHasNewStatus = orxFALSE) const;

protected:

  void    OnCreate();
  void    OnDelete();
  orxBOOL OnCollide(ScrollObject *_poCollider, const orxSTRING _zPartName, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal);
  void    Update(const orxCLOCK_INFO &_stInfo);


private:

  const orxSTRING mzID;
        orxU64    mu64GunID;
        orxU64    mu64HeadID;
};

class Enemy : public ScrollObject
{
public:

protected:

  void    OnCreate();
  void    OnDelete();
  orxBOOL OnCollide(ScrollObject *_poCollider, const orxSTRING _zPartName, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal);
  void    Update(const orxCLOCK_INFO &_stInfo);


private:
};


//! Code

orxBOOL Player::IsInputActive(const orxSTRING _zInput, orxBOOL _bHasNewStatus /* = orxFALSE */) const
{
  orxCHAR acInput[64];
  orxBOOL bResult;

  // Gets input name
  orxString_NPrint(acInput, sizeof(acInput) - 1, "%s%s", mzID, _zInput);

  // Updates result
  bResult = orxInput_IsActive(acInput) && (!_bHasNewStatus || orxInput_HasNewStatus(acInput));

  // Done!
  return bResult;
}

void Player::OnCreate()
{
  orxCHAR       acBuffer[64];
  ScrollObject *poCamera;

  // Inits ID
  mzID = orxConfig_GetString("ID");

  // Pushes RunTime section
  orxConfig_PushSection("RunTime");

  // Retrieves related object IDs
  orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s%s", mzID, "Gun");
  mu64GunID = orxConfig_GetU64(acBuffer);
  orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s%s", mzID, "Head");
  mu64HeadID = orxConfig_GetU64(acBuffer);

  // Retrieves camera
  poCamera = LRJ::GetInstance().GetObject(orxConfig_GetU64("Camera"));

  // Found and not already attached to a player?
  if(poCamera && !orxObject_GetParent(poCamera->GetOrxObject()))
  {
    // Sets ourself as parent
    orxObject_SetParent(poCamera->GetOrxObject(), GetOrxObject());
  }

  // Pops config section
  orxConfig_PopSection();
}

void Player::OnDelete()
{
  // Stops the game
  orxInput_SetValue("Stop", orxFLOAT_1);
}

orxBOOL Player::OnCollide(ScrollObject *_poCollider, const orxSTRING _zPartName, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal)
{
  ScrollObject *poExplosion;

  // Creates explosion
  poExplosion = LRJ::GetInstance().CreateObject("ExplosionObject");

  // Success?
  if(poExplosion)
  {
    orxVECTOR vPos;

    // Updates it
    poExplosion->SetPosition(GetPosition(vPos));
  }
  
  // Death!
  SetLifeTime(orxFLOAT_0);

  // Done!
  return orxTRUE;
}

void Player::Update(const orxCLOCK_INFO &_stInfo)
{
  orxVECTOR     vSpeed = {}, vPos;
  ScrollObject *poGun;
  orxFLOAT      fSpeed, fRotation = -orxFLOAT_1;

  // Gets speed
  PushConfigSection();
  fSpeed = orxConfig_GetFloat("Speed");
  PopConfigSection();

  // Left?
  if(IsInputActive("Left"))
  {
    // Updates speed & orientation
    vSpeed.fX -= fSpeed;
    fRotation = orxMATH_KF_PI + orxMATH_KF_PI_BY_2;
  }
  // Right?
  if(IsInputActive("Right"))
  {
    // Updates speed & orientation
    vSpeed.fX += fSpeed;
    fRotation = orxMATH_KF_PI_BY_2;
  }
  // Down?
  if(IsInputActive("Down"))
  {
    // Updates speed & orientation
    vSpeed.fY += fSpeed;
    fRotation = orxMATH_KF_PI;
  }
  // Up?
  if(IsInputActive("Up"))
  {
    // Updates speed & orientation
    vSpeed.fY -= fSpeed;
    fRotation = orxFLOAT_0;
  }

  // Applies speed
  SetSpeed(vSpeed);

  // Enforces proper alignment
  GetPosition(vPos);
  vPos.fX = orxMath_Floor(vPos.fX) + 0.5f;
  vPos.fY = orxMath_Floor(vPos.fY) + 0.5f;
  SetPosition(vPos);

  // Applies orientation
  if(fRotation >= orxFLOAT_0)
  {
    ScrollObject *poHead;

    // Finds head
    poHead = LRJ::GetInstance().GetObject(mu64HeadID);

    // Found?
    if(poHead)
    {
      poHead->SetRotation(fRotation);
    }
  }

  // Finds gun
  poGun = LRJ::GetInstance().GetObject(mu64GunID);

  // Valid?
  if(poGun)
  {
    // Updates its status
    poGun->Enable(IsInputActive("Shoot"));
  }
}

void Enemy::OnCreate()
{
}

void Enemy::OnDelete()
{
}

orxBOOL Enemy::OnCollide(ScrollObject *_poCollider, const orxSTRING _zPartName, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal)
{
  // Done!
  return orxTRUE;
}

void Enemy::Update(const orxCLOCK_INFO &_stInfo)
{
}

static orxBOOL orxFASTCALL SaveCallback(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  //! Done!
  return (orxString_Compare(_zSectionName, "Save") == 0) ? orxTRUE : orxFALSE;
}

orxSTATUS LRJ::Load()
{
  const orxSTRING zSavePath;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  // Pushes game config section
  orxConfig_PushSection("Game");

  // Gets save path
  zSavePath = orxFile_GetApplicationSaveDirectory(orxConfig_GetString("SaveFile"));

  // Pops config section
  orxConfig_PopSection();

  // Success?
  if(zSavePath != orxSTRING_EMPTY)
  {
    // Saves to disk
    eResult = orxConfig_Load(zSavePath);
  }

  // Done!
  return eResult;
}

orxSTATUS LRJ::Save()
{
  const orxSTRING zSavePath;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  // Pushes game config section
  orxConfig_PushSection("Game");

  // Gets save path
  zSavePath = orxFile_GetApplicationSaveDirectory(orxConfig_GetString("SaveFile"));

  // Pops config section
  orxConfig_PopSection();

  // Success?
  if(zSavePath != orxSTRING_EMPTY)
  {
    // Saves to disk
    eResult = orxConfig_Save(zSavePath, orxFALSE, SaveCallback);
  }

  // Done!
  return eResult;
}

void LRJ::DeleteRunTimeObject(const orxSTRING _zObjectName)
{
  orxU64        u64ObjectGUID;
  ScrollObject *poObject;

  // Pushes runtime section
  orxConfig_PushSection("RunTime");

  // Gets object GUID
  u64ObjectGUID = orxConfig_GetU64(_zObjectName);

  // Gets object
  poObject = GetObject(u64ObjectGUID);

  // Valid?
  if(poObject != orxNULL)
  {
    // Deletes it
    DeleteObject(poObject);

    // Clears GUID
    orxConfig_SetU64(_zObjectName, orxU64_UNDEFINED);
  }

  // Pops config section
  orxConfig_PopSection();
}

orxSTATUS LRJ::Bootstrap() const
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Disables some debug messages
  orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, orxFALSE);
  orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_PROFILER, orxFALSE);

  // Inits custom Zip archive handler
  orxArchive_Init();

  // Adds default release config paths
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data.lrj", orxFALSE);
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "data.lrj", orxFALSE);
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../../data/config", orxFALSE);
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

  // Done!
  return eResult;
}

void LRJ::Reset()
{
  // Updates state
  meGameState = GameStateReset;
}

void LRJ::UpdateInteraction(const orxCLOCK_INFO &_rstInfo)
{
  orxVECTOR     vMousePos, vPickPos;
  orxU64        u64PickedID = 0;
  ScrollObject *poPickedObject = orxNULL, *poInteractionObject;

  // Gets world mouse position
  if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos) != orxNULL)
  {
    // Stores it
    orxVector_Set(&mvMousePosition, vMousePos.fX, vMousePos.fY, orxFLOAT_0);
  }

  // Gets picking position
  orxVector_Set(&vPickPos, mvMousePosition.fX, mvMousePosition.fY, -orxFLOAT_1);

  // For all pickable groups
  for(orxU32 i = 0, u32Number = orxConfig_GetListCounter("PickGroupList"); i < u32Number; i++)
  {
    // Picks object in it
    poPickedObject = PickObject(vPickPos, orxString_GetID(orxConfig_GetListString("PickGroupList", i)));

    // Found?
    if(poPickedObject)
    {
      // Updates picked ID
      u64PickedID = poPickedObject->GetGUID();

      // Stops
      break;
    }
  }

  // Gets current interaction object
  poInteractionObject = (mu64InteractionID != 0) ? GetObject(mu64InteractionID) : orxNULL;

  // New picking?
  if(u64PickedID != mu64InteractionID)
  {
    // Has current interaction object?
    if(poInteractionObject)
    {
      // Adds contextual track
      poInteractionObject->AddConditionalTrack("OnLeaveTrack");
    }

    // Picked new object?
    if(poPickedObject)
    {
      // Adds conditional track
      poPickedObject->AddConditionalTrack(orxInput_IsActive("Action") ? "OnClickTrack" : "OnEnterTrack");
    }

    // Stores new interaction ID
    mu64InteractionID = u64PickedID;
  }
  else
  {
    // Has interaction object?
    if(poInteractionObject)
    {
      // Change of action?
      if(orxInput_HasNewStatus("Action"))
      {
        // Adds conditional track
        poInteractionObject->AddConditionalTrack(orxInput_IsActive("Action") ? "OnClickTrack" : "OnReleaseTrack");
      }
    }
  }
}

void LRJ::UpdateShader(const orxCLOCK_INFO &_rstInfo)
{
  // For all shaders
  for(orxSHADER *pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));
      pstShader != orxNULL;
      pstShader = orxSHADER(orxStructure_GetNext(pstShader)))
  {
    // Updates its time
    orxShader_SetFloatParam(pstShader, "Time", 0, &_rstInfo.fTime);
  }
}

void LRJ::UpdateGame(const orxCLOCK_INFO &_rstInfo)
{
  // Updates in-game time
  mdTime += orx2D(_rstInfo.fDT);

  // Updates input
  UpdateInput(_rstInfo);
}

void LRJ::UpdateInput(const orxCLOCK_INFO &_rstInfo)
{
  // Pausing?
  if(orxInput_IsActive("Pause"))
  {
    // Pause game
    PauseGame(!IsGamePaused());
  }
}

void LRJ::Update(const orxCLOCK_INFO &_rstInfo)
{
  // Updates interaction
  UpdateInteraction(_rstInfo);

  // Updates shader
  UpdateShader(_rstInfo);

  // Depending on game state
  switch(meGameState)
  {
    case GameStateInit:
    {
      // Shoud start?
      if(orxInput_IsActive("Start"))
      {
        // Updates state
        meGameState = GameStateRun;
      }

      break;
    }

    case GameStateRun:
    {
      // Updates in-game
      UpdateGame(_rstInfo);

      // Should end?
      if(orxInput_IsActive("Stop"))
      {
        // Creates game over object
        CreateObject("GameOverTitle");

        // Updates state
        meGameState = GameStateGameOver;
      }
      // Shoud reset?
      else if(orxInput_IsActive("Reset"))
      {
        // Resets
        Reset();
      }

      // Leaving?
      if(meGameState != GameStateRun)
      {
        // Deletes scene
        DeleteRunTimeObject("Scene");
      }

      break;
    }

    case GameStateGameOver:
    {
      // Shoud reset?
      if(orxInput_IsActive("Reset"))
      {
        // Resets
        Reset();
      }

      break;
    }

    case GameStateReset:
    {
      // Creates splash object
      CreateObject("O-Reset");

      // Updates state
      meGameState = GameStateInit;

      break;
    }
  }
}

void LRJ::CameraUpdate(const orxCLOCK_INFO &_rstInfo)
{
  ScrollObject *poCamera;

  // Pushes RunTime section
  orxConfig_PushSection("RunTime");

  // Gets camera
  poCamera = GetObject(orxConfig_GetU64("Camera"));

  // Pops config section
  orxConfig_PopSection();

  // Found?
  if(poCamera)
  {
    orxVECTOR   vPos;
    orxOBJECT  *pstParent;

    // Gets its parent
    pstParent = orxOBJECT(orxObject_GetParent(poCamera->GetOrxObject()));

    // Valid?
    if(pstParent)
    {
      orxVECTOR vSpeed;

      // Gets its speed
      orxObject_GetSpeed(pstParent, &vSpeed);

      // Non null?
      if(!orxVector_IsNull(&vSpeed))
      {
        orxVECTOR vOffset = {};

        // Gets offset
        poCamera->GetPosition(vPos);
        vOffset.fX = (vSpeed.fX != orxFLOAT_0) ? vSpeed.fX / orxMath_Abs(vSpeed.fX) * orxConfig_GetFloat("CameraOffset") : vPos.fX;
        vOffset.fY = (vSpeed.fY != orxFLOAT_0) ? vSpeed.fY / orxMath_Abs(vSpeed.fY) * orxConfig_GetFloat("CameraOffset") : vPos.fY;

        // Applies it
        poCamera->SetPosition(*orxVector_Lerp(&vOffset, &vPos, &vOffset, orxConfig_GetFloat("CameraCoef")));
      }
    }

    poCamera->GetPosition(vPos);
    vPos.fX = orxMath_Floor(vPos.fX) + 0.5f;
    vPos.fY = orxMath_Floor(vPos.fY) + 0.5f;
    poCamera->SetPosition(vPos);
  }
}

orxSTATUS LRJ::Init()
{
  orxVECTOR vPosition;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Init values
  meGameState       = GameStateInit;
  mu64InteractionID = 0;
  mdTime            = orx2D(0.0);
  orxVector_Copy(&mvMousePosition, &orxVECTOR_0);

  // Loads config
  Load();

  // Pushes main section
  orxConfig_PushSection("Game");

  // For all viewports
  for(orxU32 i = 0, iCount = orxConfig_GetListCounter("ViewportList");
      i < iCount;
      i++)
  {
    // Creates it
    orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
  }

  // Updates listener position
  orxCamera_GetPosition(GetMainCamera(), &vPosition);
  orxSoundSystem_SetListenerPosition(&vPosition);

  // Creates splash object
  CreateObject("O-Splash");

  // Sets camera parent
  orxCamera_SetParent(GetMainCamera(), CreateObject("O-Camera")->GetOrxObject());

  // Adds event handler
  orxEvent_AddHandler(orxEVENT_TYPE_SHADER, &EventHandler);

  // Done!
  return eResult;
}

orxSTATUS LRJ::Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Quitting?
  if(orxInput_IsActive("Quit"))
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

void LRJ::Exit()
{
  // Saves data
  Save();
}

void LRJ::BindObjects()
{
  // Binds objects
  ScrollBindObject<Player>("PlayerObject");
  ScrollBindObject<Enemy>("EnemyObject");
}

orxSTATUS orxFASTCALL LRJ::EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Depending on type
  switch(_pstEvent->eType)
  {
    case orxEVENT_TYPE_SHADER:
    {
      // Param?
      if(_pstEvent->eID == orxSHADER_EVENT_SET_PARAM)
      {
        orxSHADER_EVENT_PAYLOAD *pstPayload;

        // Gets payload
        pstPayload = (orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        // Camera position?
        if(!orxString_Compare(pstPayload->zParamName, "CameraPos"))
        {
          ScrollObject *poCamera;

          // Retrieves camera
          orxConfig_PushSection("RunTime");
          poCamera = LRJ::GetInstance().GetObject(orxConfig_GetU64("Camera"));
          orxConfig_PopSection();

          // Found?
          if(poCamera)
          {
            // Updates parem
            poCamera->GetPosition(pstPayload->vValue, orxTRUE);
          }
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  // Done!
  return eResult;
}


int main(int argc, char **argv)
{
  // Executes game
  LRJ::GetInstance().Execute(argc, argv);

  // Done!
  return EXIT_SUCCESS;
}

#ifdef __orxWINDOWS__
#include "windows.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Executes game
  LRJ::GetInstance().Execute();

  // Done!
  return EXIT_SUCCESS;
}
#endif // __orxWINDOWS__
