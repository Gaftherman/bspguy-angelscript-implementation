#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "angelscript.h"

class Entity;
class Bsp;
class Renderer;
class CScriptArray;

// ============================================================================
// Script Vector3 - A vec3 type for AngelScript
// ============================================================================
struct ScriptVec3 {
    float x, y, z;
    
    ScriptVec3();
    ScriptVec3(float x, float y, float z);
    ScriptVec3(const ScriptVec3& other);
    
    // Operators
    ScriptVec3& operator=(const ScriptVec3& other);
    ScriptVec3 operator+(const ScriptVec3& other) const;
    ScriptVec3 operator-(const ScriptVec3& other) const;
    ScriptVec3 operator*(float scalar) const;
    ScriptVec3 operator/(float scalar) const;
    ScriptVec3 operator-() const;
    bool operator==(const ScriptVec3& other) const;
    bool operator!=(const ScriptVec3& other) const;
    ScriptVec3& operator+=(const ScriptVec3& other);
    ScriptVec3& operator-=(const ScriptVec3& other);
    ScriptVec3& operator*=(float scalar);
    ScriptVec3& operator/=(float scalar);
    
    // Methods
    float length() const;
    float lengthSq() const;
    float dot(const ScriptVec3& other) const;
    ScriptVec3 cross(const ScriptVec3& other) const;
    ScriptVec3 normalized() const;
    void normalize();
    float distance(const ScriptVec3& other) const;
    ScriptVec3 lerp(const ScriptVec3& other, float t) const;
    std::string toString() const;
    
    // Static constructors
    static ScriptVec3 fromString(const std::string& str);
    static ScriptVec3 zero();
    static ScriptVec3 one();
    static ScriptVec3 up();
    static ScriptVec3 forward();
    static ScriptVec3 right();
};

// ============================================================================
// Script RGB Color - An RGB type for AngelScript  
// ============================================================================
struct ScriptRGB {
    int r, g, b;
    
    ScriptRGB();
    ScriptRGB(int r, int g, int b);
    ScriptRGB(const ScriptRGB& other);
    
    // Operators
    ScriptRGB& operator=(const ScriptRGB& other);
    bool operator==(const ScriptRGB& other) const;
    bool operator!=(const ScriptRGB& other) const;
    
    // Methods
    ScriptRGB lerp(const ScriptRGB& other, float t) const;
    void clamp();
    std::string toString() const;
    std::string toLightString() const; // Format: "r g b brightness"
    
    // Static constructors
    static ScriptRGB fromString(const std::string& str);
    static ScriptRGB white();
    static ScriptRGB black();
    static ScriptRGB red();
    static ScriptRGB green();
    static ScriptRGB blue();
    static ScriptRGB yellow();
    static ScriptRGB cyan();
    static ScriptRGB magenta();
};

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
    
    // Origin and angles - component access
    float getOriginX() const;
    float getOriginY() const;
    float getOriginZ() const;
    void setOrigin(float x, float y, float z);
    
    float getAnglesPitch() const;
    float getAnglesYaw() const;
    float getAnglesRoll() const;
    void setAngles(float pitch, float yaw, float roll);
    
    // Origin and angles - Vec3 access
    ScriptVec3 getOrigin() const;
    void setOriginVec(const ScriptVec3& origin);
    ScriptVec3 getAngles() const;
    void setAnglesVec(const ScriptVec3& angles);
    
    // Model
    int getBspModelIdx() const;
    bool isBspModel() const;
    
    // Keyvalue iteration
    int getKeyCount() const;
    std::string getKeyAt(int index) const;
    std::string getValueAt(int index) const;
    
    // Index in entity list
    int getIndex() const;
    
    // Utility
    bool isValid() const;
    float distanceTo(const ScriptEntity& other) const;
    float distanceToPoint(const ScriptVec3& point) const;
    
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

// Script folder structure for hierarchical menu
struct ScriptFolder {
    std::string name;
    std::string path;
    std::vector<ScriptInfo> scripts;
    std::vector<ScriptFolder> subfolders;
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
    ScriptFolder& getScriptRoot();  // Hierarchical folder structure
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
    
    // Camera access
    float getCameraX() const;
    float getCameraY() const;
    float getCameraZ() const;
    float getCameraAnglesPitch() const;
    float getCameraAnglesYaw() const;
    float getCameraAnglesRoll() const;
    ScriptVec3 getCameraPosition() const;
    ScriptVec3 getCameraAnglesVec() const;
    ScriptVec3 getCameraForward() const;  // Direction the camera is looking
    
    // Entity selection (highlight)
    CScriptArray* getSelectedEntities();  // Returns array of selected Entity@
    int getSelectedEntityCount() const;
    void selectEntity(int index);
    void deselectEntity(int index);
    void deselectAllEntities();
    bool isEntitySelected(int index) const;
    
    // Script entity batch operations (for undo grouping)
    void beginEntityBatch();
    void endEntityBatch();
    
    // Math utility functions
    static float degToRad(float degrees);
    static float radToDeg(float radians);
    
private:
    asIScriptEngine* engine;
    asIScriptContext* context;
    asIScriptModule* currentModule;
    Renderer* app;
    
    std::vector<ScriptInfo> scriptList;
    ScriptFolder scriptRoot;  // Hierarchical folder structure
    std::string scriptsFolder;
    
    void scanScriptsFolder(const std::string& folderPath, ScriptFolder& folder);
    
    // Entity wrappers cache
    std::unordered_map<int, ScriptEntity*> entityCache;
    
    // Entity batch for undo grouping
    std::vector<Entity*> batchCreatedEntities;
    bool isBatchMode;
    
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
