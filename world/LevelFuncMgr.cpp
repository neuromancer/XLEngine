#include "LevelFuncMgr.h"
#include "WorldCell.h"
#include "LevelFunc_Default.h"

std::vector<LevelFunc *> LevelFuncMgr::m_FuncList;
std::vector<LevelFunc *> LevelFuncMgr::m_Active;
std::vector<LevelFunc *> LevelFuncMgr::m_AddList;
std::vector<LevelFunc *> LevelFuncMgr::m_RemoveList;

WorldCell *LevelFuncMgr::m_pWorldCell;
std::map<std::string, LevelFuncMgr::LevelFuncCB> LevelFuncMgr::m_LevelFuncCB;

bool LevelFuncMgr::Init()
{
    SetupDefaultLevelFuncs();

    return true;
}

void LevelFuncMgr::Destroy()
{
    m_Active.clear();
    m_AddList.clear();
    m_RemoveList.clear();

    std::vector<LevelFunc *>::iterator iFunc = m_FuncList.begin();
    std::vector<LevelFunc *>::iterator eFunc = m_FuncList.end();
    for (; iFunc != eFunc; ++iFunc)
    {
        delete (*iFunc);
    }
    m_FuncList.clear();
}

void LevelFuncMgr::AddLevelFuncCB(const std::string& sFuncName, LevelFunc::LFunc_ActivateCB pActivate, LevelFunc::LFunc_SetValueCB pSetValue)
{
    std::map<std::string, LevelFuncCB>::iterator iFunc = m_LevelFuncCB.find(sFuncName);
    if ( iFunc == m_LevelFuncCB.end() )
    {
        //ok to add it...
        LevelFuncCB cb;
        cb.activateCB = pActivate;
        cb.setValueCB = pSetValue;

        m_LevelFuncCB[sFuncName] = cb;
    }
}

LevelFunc *LevelFuncMgr::CreateLevelFunc(const char *pszFuncName, int32_t nSector, int32_t nWall)
{
    //first allocate the function.
    LevelFunc *pFunc = xlNew LevelFunc(m_pWorldCell, nSector, nWall);

    //now find the appropriate callbacks.
    if ( pFunc )
    {
        std::map<std::string, LevelFuncCB>::iterator iFunc = m_LevelFuncCB.find(pszFuncName);
        if ( iFunc != m_LevelFuncCB.end() )
        {
            pFunc->SetActivateCB( iFunc->second.activateCB );
            pFunc->SetSetValueCB( iFunc->second.setValueCB );
        }

        //finally add the function to the list...
        m_FuncList.push_back( pFunc );
    }
    return pFunc;
}

void LevelFuncMgr::DestroyLevelFunc(LevelFunc *pFunc)
{
    if (!pFunc) { return; }

    RemoveFromActiveList(pFunc);

    std::vector<LevelFunc *>::iterator iter = m_FuncList.begin();
    std::vector<LevelFunc *>::iterator end  = m_FuncList.end();

    for (; iter != end; ++iter)
    {
        if ( *iter == pFunc )
        {
            m_FuncList.erase( iter );
            break;
        }
    }

    delete pFunc;
}

void LevelFuncMgr::AddToActiveList(LevelFunc *pFunc)
{
    m_AddList.push_back(pFunc);
}

void LevelFuncMgr::RemoveFromActiveList(LevelFunc *pFunc)
{
    m_RemoveList.push_back(pFunc);
}

void LevelFuncMgr::Update()
{
    std::vector<LevelFunc *>::iterator iActive = m_Active.begin();
    std::vector<LevelFunc *>::iterator eActive = m_Active.end();
    for (; iActive!=eActive; ++iActive)
    {
        if (*iActive) { (*iActive)->Update(); }
    }

    //go through the add list and add classes...
    if ( m_AddList.size() )
    {
        std::vector<LevelFunc *>::iterator iAdd = m_AddList.begin();
        std::vector<LevelFunc *>::iterator eAdd = m_AddList.end();
        for (; iAdd!=eAdd; ++iAdd)
        {
            if (*iAdd) { m_Active.push_back( *iAdd ); }
        }
        m_AddList.clear();
    }

    //go through the remove list and remove classes...
    if ( m_RemoveList.size() )
    {
        std::vector<LevelFunc *>::iterator iRem = m_RemoveList.begin();
        std::vector<LevelFunc *>::iterator eRem = m_RemoveList.end();
        for (; iRem!=eRem; ++iRem)
        {
            iActive = m_Active.begin();
            eActive = m_Active.end();

            for (; iActive!=eActive; ++iActive)
            {
                if ( *iActive == *iRem )
                {
                    m_Active.erase(iActive);
                    break;
                }
            }
        }
        m_RemoveList.clear();
    }
}
