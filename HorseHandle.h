
/// @file
/// @author Adopted from "scripthandle" AngelScript addon, distributed with AngelScript SDK.

#pragma once

#include <angelscript.h>
#include "RefCountingObject.h"

class Horse: public RefCountingObject<Horse>
{
public:
    Horse(std::string name): RefCountingObject(name) {}
    void Neigh() { std::cout << m_name << ": neigh!" << std::endl; }
};

class HorseHandle
{
public:
    // Constructors
    HorseHandle();
    HorseHandle(const HorseHandle &other);
    HorseHandle(Horse *ref); // Only invoke directly using C++! AngelScript must use a wrapper.
    ~HorseHandle();

    // Assignments
    HorseHandle &operator=(const HorseHandle &other);
    // Intentionally omitting raw-pointer assignment, for simplicity - see raw pointer constructor.

    // Set the reference
    void Set(Horse* ref);

    // Compare equalness
    bool operator==(const HorseHandle &o) const;
    bool operator!=(const HorseHandle &o) const;

    // Get the reference
    void *GetRef();
    Horse* operator->() { return m_ref; }

    // GC callback
    void EnumReferences(asIScriptEngine *engine);
    void ReleaseReferences(asIScriptEngine *engine);

protected:
    friend void RegisterHorseHandle(asIScriptEngine *engine);

    void ReleaseHandle();
    void AddRefHandle();

    Horse        *m_ref;
};

void RegisterHorseHandle(asIScriptEngine *engine);

