#include "ScriptManager.h"
#include "Renderer.h"
#include "BspRenderer.h"
#include "Gui.h"
#include "Bsp.h"
#include "Entity.h"
#include "Command.h"
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

bool ScriptEntity::isValid() const {
    return entity != nullptr;
}

ScriptVec3 ScriptEntity::getOrigin() const {
    if (!entity) return ScriptVec3();
    vec3 o = entity->getOrigin();
    return ScriptVec3(o.x, o.y, o.z);
}

void ScriptEntity::setOriginVec(const ScriptVec3& origin) {
    setOrigin(origin.x, origin.y, origin.z);
}

ScriptVec3 ScriptEntity::getAngles() const {
    if (!entity) return ScriptVec3();
    vec3 a = entity->getAngles();
    return ScriptVec3(a.x, a.y, a.z);
}

void ScriptEntity::setAnglesVec(const ScriptVec3& angles) {
    setAngles(angles.x, angles.y, angles.z);
}

float ScriptEntity::distanceTo(const ScriptEntity& other) const {
    if (!entity || !other.entity) return 0.0f;
    ScriptVec3 myOrigin = getOrigin();
    ScriptVec3 otherOrigin = other.getOrigin();
    return myOrigin.distance(otherOrigin);
}

float ScriptEntity::distanceToPoint(const ScriptVec3& point) const {
    if (!entity) return 0.0f;
    ScriptVec3 myOrigin = getOrigin();
    return myOrigin.distance(point);
}

// ============================================================================
// ScriptVec3 Implementation
// ============================================================================

ScriptVec3::ScriptVec3() : x(0), y(0), z(0) {}
ScriptVec3::ScriptVec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
ScriptVec3::ScriptVec3(const ScriptVec3& other) : x(other.x), y(other.y), z(other.z) {}

ScriptVec3& ScriptVec3::operator=(const ScriptVec3& other) {
    x = other.x; y = other.y; z = other.z;
    return *this;
}

ScriptVec3 ScriptVec3::operator+(const ScriptVec3& other) const {
    return ScriptVec3(x + other.x, y + other.y, z + other.z);
}

ScriptVec3 ScriptVec3::operator-(const ScriptVec3& other) const {
    return ScriptVec3(x - other.x, y - other.y, z - other.z);
}

ScriptVec3 ScriptVec3::operator*(float scalar) const {
    return ScriptVec3(x * scalar, y * scalar, z * scalar);
}

ScriptVec3 ScriptVec3::operator/(float scalar) const {
    if (scalar == 0) return ScriptVec3();
    return ScriptVec3(x / scalar, y / scalar, z / scalar);
}

ScriptVec3 ScriptVec3::operator-() const {
    return ScriptVec3(-x, -y, -z);
}

bool ScriptVec3::operator==(const ScriptVec3& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool ScriptVec3::operator!=(const ScriptVec3& other) const {
    return !(*this == other);
}

ScriptVec3& ScriptVec3::operator+=(const ScriptVec3& other) {
    x += other.x; y += other.y; z += other.z;
    return *this;
}

ScriptVec3& ScriptVec3::operator-=(const ScriptVec3& other) {
    x -= other.x; y -= other.y; z -= other.z;
    return *this;
}

ScriptVec3& ScriptVec3::operator*=(float scalar) {
    x *= scalar; y *= scalar; z *= scalar;
    return *this;
}

ScriptVec3& ScriptVec3::operator/=(float scalar) {
    if (scalar != 0) { x /= scalar; y /= scalar; z /= scalar; }
    return *this;
}

float ScriptVec3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

float ScriptVec3::lengthSq() const {
    return x*x + y*y + z*z;
}

float ScriptVec3::dot(const ScriptVec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

ScriptVec3 ScriptVec3::cross(const ScriptVec3& other) const {
    return ScriptVec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

ScriptVec3 ScriptVec3::normalized() const {
    float len = length();
    if (len == 0) return ScriptVec3();
    return *this / len;
}

void ScriptVec3::normalize() {
    float len = length();
    if (len > 0) {
        x /= len; y /= len; z /= len;
    }
}

float ScriptVec3::distance(const ScriptVec3& other) const {
    return (*this - other).length();
}

ScriptVec3 ScriptVec3::lerp(const ScriptVec3& other, float t) const {
    return ScriptVec3(
        x + (other.x - x) * t,
        y + (other.y - y) * t,
        z + (other.z - z) * t
    );
}

std::string ScriptVec3::toString() const {
    char buf[128];
    snprintf(buf, sizeof(buf), "%g %g %g", x, y, z);
    return std::string(buf);
}

ScriptVec3 ScriptVec3::fromString(const std::string& str) {
    ScriptVec3 v;
    sscanf(str.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
    return v;
}

ScriptVec3 ScriptVec3::zero() { return ScriptVec3(0, 0, 0); }
ScriptVec3 ScriptVec3::one() { return ScriptVec3(1, 1, 1); }
ScriptVec3 ScriptVec3::up() { return ScriptVec3(0, 0, 1); }
ScriptVec3 ScriptVec3::forward() { return ScriptVec3(1, 0, 0); }
ScriptVec3 ScriptVec3::right() { return ScriptVec3(0, 1, 0); }

// ============================================================================
// ScriptRGB Implementation
// ============================================================================

ScriptRGB::ScriptRGB() : r(255), g(255), b(255) {}
ScriptRGB::ScriptRGB(int r_, int g_, int b_) : r(r_), g(g_), b(b_) {}
ScriptRGB::ScriptRGB(const ScriptRGB& other) : r(other.r), g(other.g), b(other.b) {}

ScriptRGB& ScriptRGB::operator=(const ScriptRGB& other) {
    r = other.r; g = other.g; b = other.b;
    return *this;
}

bool ScriptRGB::operator==(const ScriptRGB& other) const {
    return r == other.r && g == other.g && b == other.b;
}

bool ScriptRGB::operator!=(const ScriptRGB& other) const {
    return !(*this == other);
}

ScriptRGB ScriptRGB::lerp(const ScriptRGB& other, float t) const {
    return ScriptRGB(
        (int)(r + (other.r - r) * t),
        (int)(g + (other.g - g) * t),
        (int)(b + (other.b - b) * t)
    );
}

void ScriptRGB::clamp() {
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
}

std::string ScriptRGB::toString() const {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d %d %d", r, g, b);
    return std::string(buf);
}

std::string ScriptRGB::toLightString() const {
    // GoldSrc light format: "R G B brightness"
    char buf[64];
    snprintf(buf, sizeof(buf), "%d %d %d 200", r, g, b);
    return std::string(buf);
}

ScriptRGB ScriptRGB::fromString(const std::string& str) {
    ScriptRGB c;
    sscanf(str.c_str(), "%d %d %d", &c.r, &c.g, &c.b);
    return c;
}

ScriptRGB ScriptRGB::white() { return ScriptRGB(255, 255, 255); }
ScriptRGB ScriptRGB::black() { return ScriptRGB(0, 0, 0); }
ScriptRGB ScriptRGB::red() { return ScriptRGB(255, 0, 0); }
ScriptRGB ScriptRGB::green() { return ScriptRGB(0, 255, 0); }
ScriptRGB ScriptRGB::blue() { return ScriptRGB(0, 0, 255); }
ScriptRGB ScriptRGB::yellow() { return ScriptRGB(255, 255, 0); }
ScriptRGB ScriptRGB::cyan() { return ScriptRGB(0, 255, 255); }
ScriptRGB ScriptRGB::magenta() { return ScriptRGB(255, 0, 255); }

// ============================================================================
// ScriptManager Implementation
// ============================================================================

ScriptManager::ScriptManager() 
    : engine(nullptr), context(nullptr), currentModule(nullptr), app(nullptr), isBatchMode(false) {
    
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
    
    // ====== Register Vec3 as a value type ======
    r = engine->RegisterObjectType("Vec3", sizeof(ScriptVec3), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<ScriptVec3>()); assert(r >= 0);
    
    // Vec3 constructors
    r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f()", 
        asFUNCTIONPR([](void* mem) { new(mem) ScriptVec3(); }, (void*), void), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", 
        asFUNCTIONPR([](void* mem, float x, float y, float z) { new(mem) ScriptVec3(x, y, z); }, (void*, float, float, float), void), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(const Vec3 &in)", 
        asFUNCTIONPR([](void* mem, const ScriptVec3& other) { new(mem) ScriptVec3(other); }, (void*, const ScriptVec3&), void), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    
    // Vec3 properties
    r = engine->RegisterObjectProperty("Vec3", "float x", asOFFSET(ScriptVec3, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vec3", "float y", asOFFSET(ScriptVec3, y)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vec3", "float z", asOFFSET(ScriptVec3, z)); assert(r >= 0);
    
    // Vec3 operators
    r = engine->RegisterObjectMethod("Vec3", "Vec3 &opAssign(const Vec3 &in)", asMETHOD(ScriptVec3, operator=), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 opAdd(const Vec3 &in) const", asMETHODPR(ScriptVec3, operator+, (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 opSub(const Vec3 &in) const", asMETHODPR(ScriptVec3, operator-, (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 opMul(float) const", asMETHODPR(ScriptVec3, operator*, (float) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 opDiv(float) const", asMETHODPR(ScriptVec3, operator/, (float) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 opNeg() const", asMETHODPR(ScriptVec3, operator-, () const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "bool opEquals(const Vec3 &in) const", asMETHOD(ScriptVec3, operator==), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 &opAddAssign(const Vec3 &in)", asMETHOD(ScriptVec3, operator+=), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 &opSubAssign(const Vec3 &in)", asMETHOD(ScriptVec3, operator-=), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 &opMulAssign(float)", asMETHODPR(ScriptVec3, operator*=, (float), ScriptVec3&), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 &opDivAssign(float)", asMETHODPR(ScriptVec3, operator/=, (float), ScriptVec3&), asCALL_THISCALL); assert(r >= 0);
    
    // Vec3 methods
    r = engine->RegisterObjectMethod("Vec3", "float length() const", asMETHOD(ScriptVec3, length), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "float lengthSq() const", asMETHOD(ScriptVec3, lengthSq), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "float dot(const Vec3 &in) const", asMETHOD(ScriptVec3, dot), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 cross(const Vec3 &in) const", asMETHOD(ScriptVec3, cross), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 normalized() const", asMETHOD(ScriptVec3, normalized), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "void normalize()", asMETHOD(ScriptVec3, normalize), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "float distance(const Vec3 &in) const", asMETHOD(ScriptVec3, distance), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "Vec3 lerp(const Vec3 &in, float) const", asMETHOD(ScriptVec3, lerp), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Vec3", "string toString() const", asMETHOD(ScriptVec3, toString), asCALL_THISCALL); assert(r >= 0);
    
    // Vec3 static factory functions (registered as global functions)
    r = engine->RegisterGlobalFunction("Vec3 Vec3_fromString(const string &in)", asFUNCTION(ScriptVec3::fromString), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 Vec3_zero()", asFUNCTION(ScriptVec3::zero), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 Vec3_one()", asFUNCTION(ScriptVec3::one), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 Vec3_up()", asFUNCTION(ScriptVec3::up), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 Vec3_forward()", asFUNCTION(ScriptVec3::forward), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 Vec3_right()", asFUNCTION(ScriptVec3::right), asCALL_CDECL); assert(r >= 0);
    
    // ====== Register RGB as a value type ======
    r = engine->RegisterObjectType("RGB", sizeof(ScriptRGB), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<ScriptRGB>()); assert(r >= 0);
    
    // RGB constructors
    r = engine->RegisterObjectBehaviour("RGB", asBEHAVE_CONSTRUCT, "void f()", 
        asFUNCTIONPR([](void* mem) { new(mem) ScriptRGB(); }, (void*), void), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("RGB", asBEHAVE_CONSTRUCT, "void f(int, int, int)", 
        asFUNCTIONPR([](void* mem, int r, int g, int b) { new(mem) ScriptRGB(r, g, b); }, (void*, int, int, int), void), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("RGB", asBEHAVE_CONSTRUCT, "void f(const RGB &in)", 
        asFUNCTIONPR([](void* mem, const ScriptRGB& other) { new(mem) ScriptRGB(other); }, (void*, const ScriptRGB&), void), asCALL_CDECL_OBJFIRST); assert(r >= 0);
    
    // RGB properties
    r = engine->RegisterObjectProperty("RGB", "int r", asOFFSET(ScriptRGB, r)); assert(r >= 0);
    r = engine->RegisterObjectProperty("RGB", "int g", asOFFSET(ScriptRGB, g)); assert(r >= 0);
    r = engine->RegisterObjectProperty("RGB", "int b", asOFFSET(ScriptRGB, b)); assert(r >= 0);
    
    // RGB operators
    r = engine->RegisterObjectMethod("RGB", "RGB &opAssign(const RGB &in)", asMETHOD(ScriptRGB, operator=), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("RGB", "bool opEquals(const RGB &in) const", asMETHOD(ScriptRGB, operator==), asCALL_THISCALL); assert(r >= 0);
    
    // RGB methods
    r = engine->RegisterObjectMethod("RGB", "RGB lerp(const RGB &in, float) const", asMETHOD(ScriptRGB, lerp), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("RGB", "void clamp()", asMETHOD(ScriptRGB, clamp), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("RGB", "string toString() const", asMETHOD(ScriptRGB, toString), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("RGB", "string toLightString() const", asMETHOD(ScriptRGB, toLightString), asCALL_THISCALL); assert(r >= 0);
    
    // RGB static factory functions
    r = engine->RegisterGlobalFunction("RGB RGB_fromString(const string &in)", asFUNCTION(ScriptRGB::fromString), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_white()", asFUNCTION(ScriptRGB::white), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_black()", asFUNCTION(ScriptRGB::black), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_red()", asFUNCTION(ScriptRGB::red), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_green()", asFUNCTION(ScriptRGB::green), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_blue()", asFUNCTION(ScriptRGB::blue), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_yellow()", asFUNCTION(ScriptRGB::yellow), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_cyan()", asFUNCTION(ScriptRGB::cyan), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("RGB RGB_magenta()", asFUNCTION(ScriptRGB::magenta), asCALL_CDECL); assert(r >= 0);
    
    // ====== Register ScriptEntity as a reference type ======
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
    
    // Validity check
    r = engine->RegisterObjectMethod("Entity", "bool isValid() const", 
        asMETHOD(ScriptEntity, isValid), asCALL_THISCALL); assert(r >= 0);
    
    // Vec3 methods for origin and angles
    r = engine->RegisterObjectMethod("Entity", "Vec3 getOrigin() const", 
        asMETHOD(ScriptEntity, getOrigin), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void setOriginVec(const Vec3 &in)", 
        asMETHOD(ScriptEntity, setOriginVec), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "Vec3 getAngles() const", 
        asMETHOD(ScriptEntity, getAngles), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "void setAnglesVec(const Vec3 &in)", 
        asMETHOD(ScriptEntity, setAnglesVec), asCALL_THISCALL); assert(r >= 0);
    
    // Distance methods
    r = engine->RegisterObjectMethod("Entity", "float distanceTo(const Entity &in) const", 
        asMETHOD(ScriptEntity, distanceTo), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Entity", "float distanceToPoint(const Vec3 &in) const", 
        asMETHOD(ScriptEntity, distanceToPoint), asCALL_THISCALL); assert(r >= 0);
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

// Camera wrapper functions
static float Script_getCameraX() {
    if (g_scriptManager) return g_scriptManager->getCameraX();
    return 0.0f;
}

static float Script_getCameraY() {
    if (g_scriptManager) return g_scriptManager->getCameraY();
    return 0.0f;
}

static float Script_getCameraZ() {
    if (g_scriptManager) return g_scriptManager->getCameraZ();
    return 0.0f;
}

static float Script_getCameraPitch() {
    if (g_scriptManager) return g_scriptManager->getCameraAnglesPitch();
    return 0.0f;
}

static float Script_getCameraYaw() {
    if (g_scriptManager) return g_scriptManager->getCameraAnglesYaw();
    return 0.0f;
}

static float Script_getCameraRoll() {
    if (g_scriptManager) return g_scriptManager->getCameraAnglesRoll();
    return 0.0f;
}

static ScriptVec3 Script_getCameraPos() {
    if (g_scriptManager) return g_scriptManager->getCameraPosition();
    return ScriptVec3();
}

static ScriptVec3 Script_getCameraAngles() {
    if (g_scriptManager) return g_scriptManager->getCameraAnglesVec();
    return ScriptVec3();
}

// Batch mode for grouping entity creation into single undo
static void Script_beginEntityBatch() {
    if (g_scriptManager) g_scriptManager->beginEntityBatch();
}

static void Script_endEntityBatch() {
    if (g_scriptManager) g_scriptManager->endEntityBatch();
}

static float Script_degToRad(float degrees) {
    return ScriptManager::degToRad(degrees);
}

static float Script_radToDeg(float radians) {
    return ScriptManager::radToDeg(radians);
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
    
    // Camera functions
    r = engine->RegisterGlobalFunction("float getCameraX()", 
        asFUNCTION(Script_getCameraX), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float getCameraY()", 
        asFUNCTION(Script_getCameraY), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float getCameraZ()", 
        asFUNCTION(Script_getCameraZ), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float getCameraPitch()", 
        asFUNCTION(Script_getCameraPitch), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float getCameraYaw()", 
        asFUNCTION(Script_getCameraYaw), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float getCameraRoll()", 
        asFUNCTION(Script_getCameraRoll), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 getCameraPos()", 
        asFUNCTION(Script_getCameraPos), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vec3 getCameraAngles()", 
        asFUNCTION(Script_getCameraAngles), asCALL_CDECL); assert(r >= 0);
    
    // Entity batch operations (for undo grouping)
    r = engine->RegisterGlobalFunction("void beginEntityBatch()", 
        asFUNCTION(Script_beginEntityBatch), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void endEntityBatch()", 
        asFUNCTION(Script_endEntityBatch), asCALL_CDECL); assert(r >= 0);
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
    
    // In batch mode, track entity for grouped undo but don't add to map yet
    // Actually we need to add to map immediately for the script to modify it
    // We'll track it for the undo command
    if (isBatchMode) {
        // Store a copy for the undo command
        batchCreatedEntities.push_back(ent);
    }
    
    map->ents.push_back(ent);
    
    int index = (int)map->ents.size() - 1;
    ScriptEntity* wrapper = new ScriptEntity(ent, map, index);
    entityCache[index] = wrapper;
    wrapper->addRef();
    
    logf("Created entity: %s (index %d)%s\n", classname.c_str(), index, 
         isBatchMode ? " [batched]" : "");
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

float ScriptManager::getCameraX() const {
    if (!app) return 0.0f;
    return app->cameraOrigin.x;
}

float ScriptManager::getCameraY() const {
    if (!app) return 0.0f;
    return app->cameraOrigin.y;
}

float ScriptManager::getCameraZ() const {
    if (!app) return 0.0f;
    return app->cameraOrigin.z;
}

float ScriptManager::getCameraAnglesPitch() const {
    if (!app) return 0.0f;
    return app->cameraAngles.x;
}

float ScriptManager::getCameraAnglesYaw() const {
    if (!app) return 0.0f;
    return app->cameraAngles.y;
}

float ScriptManager::getCameraAnglesRoll() const {
    if (!app) return 0.0f;
    return app->cameraAngles.z;
}

ScriptVec3 ScriptManager::getCameraPosition() const {
    if (!app) return ScriptVec3();
    return ScriptVec3(app->cameraOrigin.x, app->cameraOrigin.y, app->cameraOrigin.z);
}

ScriptVec3 ScriptManager::getCameraAnglesVec() const {
    if (!app) return ScriptVec3();
    return ScriptVec3(app->cameraAngles.x, app->cameraAngles.y, app->cameraAngles.z);
}

void ScriptManager::beginEntityBatch() {
    if (isBatchMode) {
        logf("[Script WARNING] beginEntityBatch() called while already in batch mode\n");
        return;
    }
    isBatchMode = true;
    batchCreatedEntities.clear();
    logf("[Script] Entity batch started\n");
}

void ScriptManager::endEntityBatch() {
    if (!isBatchMode) {
        logf("[Script WARNING] endEntityBatch() called without beginEntityBatch()\n");
        return;
    }
    
    isBatchMode = false;
    
    if (batchCreatedEntities.empty()) {
        logf("[Script] Entity batch ended (no entities created)\n");
        return;
    }
    
    if (!app || !app->mapRenderer || !app->mapRenderer->map) {
        batchCreatedEntities.clear();
        return;
    }
    
    int numBatched = (int)batchCreatedEntities.size();
    
    // The entities are already in the map and modified by the script.
    // We need to create an undo command that knows about these entities.
    // CreateEntitiesCommand stores copies of entities.
    // When undo is called, it removes the last N entities from the map.
    // When redo is called (execute), it adds the copies back.
    
    // Create the undo command with the entities we created
    // Note: CreateEntitiesCommand makes copies, so our pointers remain valid
    CreateEntitiesCommand* cmd = new CreateEntitiesCommand("Script: Create Entities", batchCreatedEntities);
    
    // Push to undo stack (this does NOT execute the command)
    // The entities are already in the map, so we're set up correctly:
    // - Undo will remove the last N entities (which are ours)
    // - Redo (execute) will add them back from the stored copies
    app->pushUndoCommand(cmd);
    
    logf("[Script] Entity batch ended (%d entities, grouped for undo)\n", numBatched);
    batchCreatedEntities.clear();
}
