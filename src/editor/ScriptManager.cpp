#include "ScriptManager.h"
#include "Renderer.h"
#include "BspRenderer.h"
#include "Gui.h"
#include "Bsp.h"
#include "Entity.h"
#include "util.h"
#include "globals.h"

#include "angelscript.h"
#include "scriptbuilder/scriptbuilder.h"
#include "scriptstdstring/scriptstdstring.h"
#include "scriptarray/scriptarray.h"
#include "scriptmath/scriptmath.h"
#include "scripthelper/scripthelper.h"

#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
// Undefine Windows macros that conflict with std::min/std::max
#undef min
#undef max
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global script manager instance
ScriptManager* g_scriptManager = nullptr;

// ============================================================================
// ScriptEntity Implementation
// ============================================================================

ScriptEntity::ScriptEntity() : entity(nullptr), map(nullptr), entityIndex(-1), refCount(1) {}

ScriptEntity::ScriptEntity(Entity* ent, Bsp* m, int index) 
    : entity(ent), map(m), entityIndex(index), refCount(1) {}

void ScriptEntity::addRef() {
    refCount++;
}

void ScriptEntity::release() {
    if (--refCount == 0) {
        delete this;
    }
}

std::string ScriptEntity::getKeyvalue(const std::string& key) const {
    if (!entity) return "";
    return entity->getKeyvalue(key);
}

void ScriptEntity::setKeyvalue(const std::string& key, const std::string& value) {
    if (!entity) return;
    entity->setOrAddKeyvalue(key, value);
}

bool ScriptEntity::hasKey(const std::string& key) const {
    if (!entity) return false;
    return entity->hasKey(key);
}

void ScriptEntity::removeKeyvalue(const std::string& key) {
    if (!entity) return;
    entity->removeKeyvalue(key);
}

std::string ScriptEntity::getClassname() const {
    if (!entity) return "";
    return entity->getClassname();
}

std::string ScriptEntity::getTargetname() const {
    if (!entity) return "";
    return entity->getTargetname();
}

float ScriptEntity::getOriginX() const {
    if (!entity) return 0.0f;
    return entity->getOrigin().x;
}

float ScriptEntity::getOriginY() const {
    if (!entity) return 0.0f;
    return entity->getOrigin().y;
}

float ScriptEntity::getOriginZ() const {
    if (!entity) return 0.0f;
    return entity->getOrigin().z;
}

void ScriptEntity::setOrigin(float x, float y, float z) {
    if (!entity) return;
    char buf[128];
    snprintf(buf, sizeof(buf), "%g %g %g", x, y, z);
    entity->setOrAddKeyvalue("origin", buf);
}

float ScriptEntity::getAnglesPitch() const {
    if (!entity) return 0.0f;
    return entity->getAngles().x;
}

float ScriptEntity::getAnglesYaw() const {
    if (!entity) return 0.0f;
    return entity->getAngles().y;
}

float ScriptEntity::getAnglesRoll() const {
    if (!entity) return 0.0f;
    return entity->getAngles().z;
}

void ScriptEntity::setAngles(float pitch, float yaw, float roll) {
    if (!entity) return;
    char buf[128];
    snprintf(buf, sizeof(buf), "%g %g %g", pitch, yaw, roll);
    entity->setOrAddKeyvalue("angles", buf);
}

int ScriptEntity::getBspModelIdx() const {
    if (!entity) return -1;
    return entity->getBspModelIdx();
}

bool ScriptEntity::isBspModel() const {
    if (!entity) return false;
    return entity->isBspModel();
}

int ScriptEntity::getKeyCount() const {
    if (!entity) return 0;
    return (int)entity->keyOrder.size();
}

std::string ScriptEntity::getKeyAt(int index) const {
    if (!entity || index < 0 || index >= (int)entity->keyOrder.size()) return "";
    return entity->keyOrder[index];
}

std::string ScriptEntity::getValueAt(int index) const {
    if (!entity || index < 0 || index >= (int)entity->keyOrder.size()) return "";
    return entity->getKeyvalue(entity->keyOrder[index]);
}

int ScriptEntity::getIndex() const {
    return entityIndex;
}

// ============================================================================
// ScriptManager Implementation
// ============================================================================

ScriptManager::ScriptManager() 
    : engine(nullptr), context(nullptr), currentModule(nullptr), app(nullptr) {
    
    // Get Documents folder path
#ifdef _WIN32
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path))) {
        std::wstring wpath(path);
        scriptsFolder = std::string(wpath.begin(), wpath.end()) + "\\bspguy\\scripts";
        CoTaskMemFree(path);
    } else {
        // Fallback: use USERPROFILE environment variable
        const char* userProfile = getenv("USERPROFILE");
        if (userProfile) {
            scriptsFolder = std::string(userProfile) + "\\Documents\\bspguy\\scripts";
        } else {
            // Last resort: use current directory
            scriptsFolder = ".\\scripts";
        }
    }
#else
    const char* home = getenv("HOME");
    if (home) {
        scriptsFolder = std::string(home) + "/Documents/bspguy/scripts";
    } else {
        scriptsFolder = "./scripts";
    }
#endif
}

ScriptManager::~ScriptManager() {
    shutdown();
}

void ScriptManager::messageCallback(const asSMessageInfo* msg, void* param) {
    const char* type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";
    
    logf("[AngelScript %s] %s (%d, %d): %s\n", type, msg->section, msg->row, msg->col, msg->message);
}

bool ScriptManager::init(Renderer* renderer) {
    app = renderer;
    
    // Create the script engine
    engine = asCreateScriptEngine();
    if (!engine) {
        logf("Failed to create AngelScript engine\n");
        return false;
    }
    
    // Set the message callback
    engine->SetMessageCallback(asFUNCTION(messageCallback), nullptr, asCALL_CDECL);
    
    // Register standard add-ons
    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMath(engine);
    RegisterExceptionRoutines(engine);
    
    // Register custom types and functions
    registerTypes();
    registerEntityMethods();
    registerGlobalFunctions();
    
    // Create a context for execution
    context = engine->CreateContext();
    if (!context) {
        logf("Failed to create AngelScript context\n");
        return false;
    }
    
    // Create scripts folder if it doesn't exist
    if (!dirExists(scriptsFolder)) {
        createDir(scriptsFolder);
        logf("Created scripts folder: %s\n", scriptsFolder.c_str());
    }
    
    refreshScriptList();
    
    logf("AngelScript initialized. Scripts folder: %s\n", scriptsFolder.c_str());
    return true;
}

void ScriptManager::shutdown() {
    clearEntityCache();
    
    if (context) {
        context->Release();
        context = nullptr;
    }
    
    if (engine) {
        engine->ShutDownAndRelease();
        engine = nullptr;
    }
    
    currentModule = nullptr;
}

void ScriptManager::registerTypes() {
    int r;
    
    // Register ScriptEntity as a reference type
    r = engine->RegisterObjectType("Entity", 0, asOBJ_REF); assert(r >= 0);
    
    // Register reference counting behaviors
    r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_ADDREF, "void f()", 
        asMETHOD(ScriptEntity, addRef), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_RELEASE, "void f()", 
        asMETHOD(ScriptEntity, release), asCALL_THISCALL); assert(r >= 0);
}

void ScriptManager::registerEntityMethods() {
    int r;
    
    // Keyvalue methods
    r = engine->RegisterObjectMethod("Entity", "string getKeyvalue(const string &in) const", 
        asMETHOD(ScriptEntity, getKeyvalue), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void setKeyvalue(const string &in, const string &in)", 
        asMETHOD(ScriptEntity, setKeyvalue), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "bool hasKey(const string &in) const", 
        asMETHOD(ScriptEntity, hasKey), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void removeKeyvalue(const string &in)", 
        asMETHOD(ScriptEntity, removeKeyvalue), asCALL_THISCALL); assert(r >= 0);
    
    // Classname and targetname
    r = engine->RegisterObjectMethod("Entity", "string getClassname() const", 
        asMETHOD(ScriptEntity, getClassname), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "string getTargetname() const", 
        asMETHOD(ScriptEntity, getTargetname), asCALL_THISCALL); assert(r >= 0);
    
    // Origin
    r = engine->RegisterObjectMethod("Entity", "float getOriginX() const", 
        asMETHOD(ScriptEntity, getOriginX), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "float getOriginY() const", 
        asMETHOD(ScriptEntity, getOriginY), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "float getOriginZ() const", 
        asMETHOD(ScriptEntity, getOriginZ), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void setOrigin(float, float, float)", 
        asMETHOD(ScriptEntity, setOrigin), asCALL_THISCALL); assert(r >= 0);
    
    // Angles
    r = engine->RegisterObjectMethod("Entity", "float getAnglesPitch() const", 
        asMETHOD(ScriptEntity, getAnglesPitch), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "float getAnglesYaw() const", 
        asMETHOD(ScriptEntity, getAnglesYaw), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "float getAnglesRoll() const", 
        asMETHOD(ScriptEntity, getAnglesRoll), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void setAngles(float, float, float)", 
        asMETHOD(ScriptEntity, setAngles), asCALL_THISCALL); assert(r >= 0);
    
    // Model
    r = engine->RegisterObjectMethod("Entity", "int getBspModelIdx() const", 
        asMETHOD(ScriptEntity, getBspModelIdx), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "bool isBspModel() const", 
        asMETHOD(ScriptEntity, isBspModel), asCALL_THISCALL); assert(r >= 0);
    
    // Key iteration
    r = engine->RegisterObjectMethod("Entity", "int getKeyCount() const", 
        asMETHOD(ScriptEntity, getKeyCount), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "string getKeyAt(int) const", 
        asMETHOD(ScriptEntity, getKeyAt), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "string getValueAt(int) const", 
        asMETHOD(ScriptEntity, getValueAt), asCALL_THISCALL); assert(r >= 0);
    
    // Index
    r = engine->RegisterObjectMethod("Entity", "int getIndex() const", 
        asMETHOD(ScriptEntity, getIndex), asCALL_THISCALL); assert(r >= 0);
}

// Wrapper functions for global functions (AngelScript requires specific calling convention)
static void Script_print(const std::string& msg) {
    if (g_scriptManager) g_scriptManager->print(msg);
}

static void Script_printWarning(const std::string& msg) {
    if (g_scriptManager) g_scriptManager->printWarning(msg);
}

static void Script_printError(const std::string& msg) {
    if (g_scriptManager) g_scriptManager->printError(msg);
}

static int Script_getEntityCount() {
    if (g_scriptManager) return g_scriptManager->getEntityCount();
    return 0;
}

static ScriptEntity* Script_getEntity(int index) {
    if (g_scriptManager) return g_scriptManager->getEntity(index);
    return nullptr;
}

static ScriptEntity* Script_getEntityByTargetname(const std::string& name) {
    if (g_scriptManager) return g_scriptManager->getEntityByTargetname(name);
    return nullptr;
}

static ScriptEntity* Script_getEntityByClassname(const std::string& name) {
    if (g_scriptManager) return g_scriptManager->getEntityByClassname(name);
    return nullptr;
}

static ScriptEntity* Script_createEntity(const std::string& classname) {
    if (g_scriptManager) return g_scriptManager->createEntity(classname);
    return nullptr;
}

static void Script_deleteEntity(int index) {
    if (g_scriptManager) g_scriptManager->deleteEntity(index);
}

static std::string Script_getMapName() {
    if (g_scriptManager) return g_scriptManager->getMapName();
    return "";
}

static void Script_refreshEntities() {
    if (g_scriptManager) g_scriptManager->refreshEntityDisplay();
}

static float Script_degToRad(float degrees) {
    return ScriptManager::degToRad(degrees);
}

static float Script_radToDeg(float radians) {
    return ScriptManager::radToDeg(radians);
}

static float Script_sin(float x) {
    return std::sin(x);
}

static float Script_cos(float x) {
    return std::cos(x);
}

static float Script_tan(float x) {
    return std::tan(x);
}

static float Script_sqrt(float x) {
    return std::sqrt(x);
}

static float Script_abs(float x) {
    return std::abs(x);
}

static float Script_floor(float x) {
    return std::floor(x);
}

static float Script_ceil(float x) {
    return std::ceil(x);
}

static float Script_round(float x) {
    return std::round(x);
}

static float Script_min(float a, float b) {
    return std::min(a, b);
}

static float Script_max(float a, float b) {
    return std::max(a, b);
}

static float Script_clamp(float value, float minVal, float maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

static float Script_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static std::string Script_intToString(int value) {
    return std::to_string(value);
}

static std::string Script_floatToString(float value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}

static int Script_stringToInt(const std::string& str) {
    return std::atoi(str.c_str());
}

static float Script_stringToFloat(const std::string& str) {
    return (float)std::atof(str.c_str());
}

void ScriptManager::registerGlobalFunctions() {
    int r;
    
    // Print functions
    r = engine->RegisterGlobalFunction("void print(const string &in)", 
        asFUNCTION(Script_print), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void printWarning(const string &in)", 
        asFUNCTION(Script_printWarning), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void printError(const string &in)", 
        asFUNCTION(Script_printError), asCALL_CDECL); assert(r >= 0);
    
    // Entity access functions
    r = engine->RegisterGlobalFunction("int getEntityCount()", 
        asFUNCTION(Script_getEntityCount), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Entity@ getEntity(int)", 
        asFUNCTION(Script_getEntity), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Entity@ getEntityByTargetname(const string &in)", 
        asFUNCTION(Script_getEntityByTargetname), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Entity@ getEntityByClassname(const string &in)", 
        asFUNCTION(Script_getEntityByClassname), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Entity@ createEntity(const string &in)", 
        asFUNCTION(Script_createEntity), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void deleteEntity(int)", 
        asFUNCTION(Script_deleteEntity), asCALL_CDECL); assert(r >= 0);
    
    // Map functions
    r = engine->RegisterGlobalFunction("string getMapName()", 
        asFUNCTION(Script_getMapName), asCALL_CDECL); assert(r >= 0);
    
    // Refresh function - IMPORTANT for visual updates
    r = engine->RegisterGlobalFunction("void refreshEntities()", 
        asFUNCTION(Script_refreshEntities), asCALL_CDECL); assert(r >= 0);
    
    // Math utility functions (sin, cos, tan, sqrt, abs, floor, ceil are already in scriptmath)
    r = engine->RegisterGlobalFunction("float degToRad(float)", 
        asFUNCTION(Script_degToRad), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float radToDeg(float)", 
        asFUNCTION(Script_radToDeg), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float round(float)", 
        asFUNCTION(Script_round), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float min(float, float)", 
        asFUNCTION(Script_min), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float max(float, float)", 
        asFUNCTION(Script_max), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float clamp(float, float, float)", 
        asFUNCTION(Script_clamp), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float lerp(float, float, float)", 
        asFUNCTION(Script_lerp), asCALL_CDECL); assert(r >= 0);
    
    // String conversion functions
    r = engine->RegisterGlobalFunction("string intToString(int)", 
        asFUNCTION(Script_intToString), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string floatToString(float)", 
        asFUNCTION(Script_floatToString), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int stringToInt(const string &in)", 
        asFUNCTION(Script_stringToInt), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float stringToFloat(const string &in)", 
        asFUNCTION(Script_stringToFloat), asCALL_CDECL); assert(r >= 0);
}

void ScriptManager::refreshScriptList() {
    scriptList.clear();
    
    if (!dirExists(scriptsFolder)) {
        createDir(scriptsFolder);
        return;
    }
    
#ifdef _WIN32
    // Use Windows API to iterate directory
    std::string searchPath = scriptsFolder + "\\*.as";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                ScriptInfo info;
                info.name = findData.cFileName;
                info.path = scriptsFolder + "\\" + findData.cFileName;
                info.hasError = false;
                scriptList.push_back(info);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    // Use POSIX API for Linux/Mac
    DIR* dir = opendir(scriptsFolder.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename.size() > 3 && filename.substr(filename.size() - 3) == ".as") {
                ScriptInfo info;
                info.name = filename;
                info.path = scriptsFolder + "/" + filename;
                info.hasError = false;
                scriptList.push_back(info);
            }
        }
        closedir(dir);
    }
#endif
    
    // Sort alphabetically
    std::sort(scriptList.begin(), scriptList.end(), 
        [](const ScriptInfo& a, const ScriptInfo& b) { 
            return a.name < b.name; 
        });
}

std::vector<ScriptInfo>& ScriptManager::getScriptList() {
    return scriptList;
}

std::string ScriptManager::getScriptsFolder() const {
    return scriptsFolder;
}

void ScriptManager::openScriptsFolder() {
    if (!dirExists(scriptsFolder)) {
        createDir(scriptsFolder);
    }
    
#ifdef _WIN32
    ShellExecuteA(NULL, "explore", scriptsFolder.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
    std::string cmd = "open \"" + scriptsFolder + "\"";
    system(cmd.c_str());
#else
    std::string cmd = "xdg-open \"" + scriptsFolder + "\"";
    system(cmd.c_str());
#endif
}

bool ScriptManager::loadScript(const std::string& scriptPath) {
    if (!engine) {
        logf("AngelScript engine not initialized\n");
        return false;
    }
    
    // Clear entity cache before running a new script
    clearEntityCache();
    
    // Create a new module for this script
    CScriptBuilder builder;
    int r = builder.StartNewModule(engine, "ScriptModule");
    if (r < 0) {
        logf("Failed to start new script module\n");
        return false;
    }
    
    r = builder.AddSectionFromFile(scriptPath.c_str());
    if (r < 0) {
        logf("Failed to load script file: %s\n", scriptPath.c_str());
        return false;
    }
    
    r = builder.BuildModule();
    if (r < 0) {
        logf("Failed to build script: %s\n", scriptPath.c_str());
        return false;
    }
    
    currentModule = engine->GetModule("ScriptModule");
    return true;
}

bool ScriptManager::executeScript(const std::string& scriptPath) {
    logf("\n--- Executing script: %s ---\n", basename(scriptPath).c_str());
    
    if (!loadScript(scriptPath)) {
        return false;
    }
    
    // Look for a main() function
    asIScriptFunction* func = currentModule->GetFunctionByDecl("void main()");
    if (!func) {
        logf("No main() function found in script. Looking for other entry points...\n");
        
        // Try other common entry point names
        func = currentModule->GetFunctionByDecl("void run()");
        if (!func) {
            func = currentModule->GetFunctionByDecl("void execute()");
        }
        if (!func) {
            logf("No entry point function found (main, run, or execute)\n");
            return false;
        }
    }
    
    // Execute the function
    context->Prepare(func);
    int r = context->Execute();
    
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            logf("Script exception: %s\n", context->GetExceptionString());
            int line = context->GetExceptionLineNumber();
            logf("  at line %d\n", line);
            return false;
        }
    }
    
    logf("--- Script finished ---\n\n");
    return true;
}

bool ScriptManager::executeFunction(const std::string& functionName) {
    if (!currentModule || !context) {
        logf("No script loaded\n");
        return false;
    }
    
    std::string decl = "void " + functionName + "()";
    asIScriptFunction* func = currentModule->GetFunctionByDecl(decl.c_str());
    if (!func) {
        logf("Function not found: %s\n", functionName.c_str());
        return false;
    }
    
    context->Prepare(func);
    int r = context->Execute();
    
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            logf("Script exception: %s\n", context->GetExceptionString());
            return false;
        }
    }
    
    return true;
}

void ScriptManager::clearEntityCache() {
    for (auto& pair : entityCache) {
        // Don't delete, let AngelScript handle it through reference counting
        if (pair.second) {
            pair.second->entity = nullptr; // Invalidate the pointer
        }
    }
    entityCache.clear();
}

ScriptEntity* ScriptManager::getEntity(int index) {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return nullptr;
    
    Bsp* map = app->mapRenderer->map;
    if (index < 0 || index >= (int)map->ents.size()) return nullptr;
    
    // Check cache
    auto it = entityCache.find(index);
    if (it != entityCache.end() && it->second->entity) {
        it->second->addRef(); // Caller gets a reference
        return it->second;
    }
    
    // Create new wrapper
    ScriptEntity* wrapper = new ScriptEntity(map->ents[index], map, index);
    entityCache[index] = wrapper;
    wrapper->addRef(); // Cache holds one reference
    return wrapper; // Return with refCount = 1 for the caller
}

int ScriptManager::getEntityCount() {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return 0;
    return (int)app->mapRenderer->map->ents.size();
}

ScriptEntity* ScriptManager::getEntityByTargetname(const std::string& targetname) {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return nullptr;
    
    Bsp* map = app->mapRenderer->map;
    for (int i = 0; i < (int)map->ents.size(); i++) {
        if (map->ents[i]->getTargetname() == targetname) {
            return getEntity(i);
        }
    }
    return nullptr;
}

ScriptEntity* ScriptManager::getEntityByClassname(const std::string& classname) {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return nullptr;
    
    Bsp* map = app->mapRenderer->map;
    for (int i = 0; i < (int)map->ents.size(); i++) {
        if (map->ents[i]->getClassname() == classname) {
            return getEntity(i);
        }
    }
    return nullptr;
}

ScriptEntity* ScriptManager::createEntity(const std::string& classname) {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return nullptr;
    
    Bsp* map = app->mapRenderer->map;
    Entity* ent = new Entity(classname);
    map->ents.push_back(ent);
    
    int index = (int)map->ents.size() - 1;
    ScriptEntity* wrapper = new ScriptEntity(ent, map, index);
    entityCache[index] = wrapper;
    wrapper->addRef();
    
    logf("Created entity: %s (index %d)\n", classname.c_str(), index);
    return wrapper;
}

void ScriptManager::deleteEntity(int index) {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return;
    
    Bsp* map = app->mapRenderer->map;
    if (index <= 0 || index >= (int)map->ents.size()) {
        logf("Cannot delete entity at index %d (invalid or worldspawn)\n", index);
        return;
    }
    
    // Invalidate cache entry
    auto it = entityCache.find(index);
    if (it != entityCache.end()) {
        it->second->entity = nullptr;
        entityCache.erase(it);
    }
    
    // Delete the entity
    delete map->ents[index];
    map->ents.erase(map->ents.begin() + index);
    
    // Update cache indices
    std::unordered_map<int, ScriptEntity*> newCache;
    for (auto& pair : entityCache) {
        if (pair.first > index) {
            pair.second->entityIndex = pair.first - 1;
            newCache[pair.first - 1] = pair.second;
        } else {
            newCache[pair.first] = pair.second;
        }
    }
    entityCache = newCache;
    
    logf("Deleted entity at index %d\n", index);
}

void ScriptManager::print(const std::string& message) {
    logf("[Script] %s\n", message.c_str());
}

void ScriptManager::printWarning(const std::string& message) {
    logf("[Script WARNING] %s\n", message.c_str());
}

void ScriptManager::printError(const std::string& message) {
    logf("[Script ERROR] %s\n", message.c_str());
}

std::string ScriptManager::getMapName() const {
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return "";
    return app->mapRenderer->map->name;
}

void ScriptManager::refreshEntityDisplay() {
    if (!app || !app->mapRenderer) return;
    
    // Refresh the entity rendering
    app->mapRenderer->preRenderEnts();
    
    logf("[Script] Entity display refreshed\n");
}

std::vector<ScriptEntity*> ScriptManager::getAllEntitiesByClassname(const std::string& classname) {
    std::vector<ScriptEntity*> result;
    if (!app || !app->mapRenderer || !app->mapRenderer->map) return result;
    
    Bsp* map = app->mapRenderer->map;
    for (int i = 0; i < (int)map->ents.size(); i++) {
        if (map->ents[i]->getClassname() == classname) {
            result.push_back(getEntity(i));
        }
    }
    return result;
}

float ScriptManager::degToRad(float degrees) {
    return degrees * (float)(M_PI / 180.0);
}

float ScriptManager::radToDeg(float radians) {
    return radians * (float)(180.0 / M_PI);
}
