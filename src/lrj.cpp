//! Includes
#define __SCROLL_IMPL__
#include "lrj.h"
#undef __SCROLL_IMPL__

#include "orxArchive.c"


//! Defines


//! Constants


//! Structs/Classes


//! Code
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

  if (orxInput_IsActive("Test"))
  {
	  orxVECTOR position = { 4, 4, 0.5 };
	  ScrollObject *explosion = CreateObject("ExplosionObject");
	  explosion->SetPosition(position);
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
		  CreateObject("PlayerObject");
		  CreateObject("EnemyObject");
		  CreateObject("EnemyObjectB");

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

  // Registers event handler
  orxEvent_AddHandler(orxEVENT_TYPE_SHADER, &EventHandler);

  // Creates splash object
  CreateObject("O-Splash");
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

        // Ratio?
        if(!orxString_Compare(pstPayload->zParamName, "ratio"))
        {
          orxFLOAT    fRatio = orxFLOAT_1;
          orxTEXTURE *pstTexture;

          // Gets object's texture
          pstTexture = orxObject_GetWorkingTexture(orxOBJECT(_pstEvent->hSender));

          // Valid?
          if(pstTexture)
          {
            orxFLOAT fCameraWidth, fTextureWidth, fTextureHeight;

            // Gets camera width
            orxConfig_PushSection("MainCamera");
            fCameraWidth = orxConfig_GetFloat("FrustumWidth");
            orxConfig_PopSection();

            // Gets texture's size
            orxTexture_GetSize(pstTexture, &fTextureWidth, &fTextureHeight);

            // Valid?
            if(fTextureWidth > orxFLOAT_0)
            {
              // Updates ratio
              fRatio = fCameraWidth / fTextureWidth;
            }
          }

          // Updates param
          pstPayload->fValue = fRatio;
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
