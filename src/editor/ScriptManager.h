#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "angelscript.h"

class Entity;
class Bsp;
class Renderer;

// Wrapper class for exposing Entity to AngelScript
class ScriptEntity {
public:
    Entity* entity;
    Bsp* map;
    int entityIndex;

    ScriptEntity();
    ScriptEntity(Entity* ent, Bsp* map, int index);
    
    // AngelScript exposed methods
    std::string getKeyvalue(const std::string& key) const;
    void setKeyvalue(const std::string& key, const std::string& value);
    bool hasKey(const std::string& key) const;
    void removeKeyvalue(const std::string& key);
    
    std::string getClassname() const;
    std::string getTargetname() const;
    
    // Origin and angles
    float getOriginX() const;
    float getOriginY() const;
    float getOriginZ() const;
    void setOrigin(float x, float y, float z);
    
    float getAnglesPitch() const;
    float getAnglesYaw() const;
    float getAnglesRoll() const;
    void setAngles(float pitch, float yaw, float roll);
    
    // Model
    int getBspModelIdx() const;
    bool isBspModel() const;
    
    // Keyvalue iteration
    int getKeyCount() const;
    std::string getKeyAt(int index) const;
    std::string getValueAt(int index) const;
    
    // Index in entity list
    int getIndex() const;
    
    // Reference counting for AngelScript
    void addRef();
    void release();
    
private:
    int refCount;
};

// Script information structure
struct ScriptInfo {
    std::string name;
    std::string path;
    bool hasError;
    std::string errorMessage;
};

class ScriptManager {
public:
    ScriptManager();
    ~ScriptManager();
    
    // Initialize AngelScript engine
    bool init(Renderer* renderer);
    void shutdown();
    
    // Script file management
    void refreshScriptList();
    std::vector<ScriptInfo>& getScriptList();
    std::string getScriptsFolder() const;
    void openScriptsFolder();
    
    // Script execution
    bool loadScript(const std::string& scriptPath);
    bool executeScript(const std::string& scriptPath);
    bool executeFunction(const std::string& functionName);
    
    // Entity access for scripts
    ScriptEntity* getEntity(int index);
    int getEntityCount();
    ScriptEntity* getEntityByTargetname(const std::string& targetname);
    ScriptEntity* getEntityByClassname(const std::string& classname);
    ScriptEntity* createEntity(const std::string& classname);
    void deleteEntity(int index);
    
    // Visual refresh after modifications
    void refreshEntityDisplay();
    
    // Get all entities by classname (returns array)
    std::vector<ScriptEntity*> getAllEntitiesByClassname(const std::string& classname);
    
    // Utility functions for scripts
    void print(const std::string& message);
    void printWarning(const std::string& message);
    void printError(const std::string& message);
    
    // Map access
    std::string getMapName() const;
    
    // Math utility functions
    static float degToRad(float degrees);
    static float radToDeg(float radians);
    
private:
    asIScriptEngine* engine;
    asIScriptContext* context;
    asIScriptModule* currentModule;
    Renderer* app;
    
    std::vector<ScriptInfo> scriptList;
    std::string scriptsFolder;
    
    // Entity wrappers cache
    std::unordered_map<int, ScriptEntity*> entityCache;
    
    void clearEntityCache();
    
    // Registration methods
    void registerTypes();
    void registerEntityMethods();
    void registerGlobalFunctions();
    void registerMathTypes();
    
    // AngelScript callbacks
    static void messageCallback(const asSMessageInfo* msg, void* param);
};

// Global script manager instance
extern ScriptManager* g_scriptManager;
