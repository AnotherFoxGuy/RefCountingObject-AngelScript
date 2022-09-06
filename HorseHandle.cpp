#include "HorseHandle.h"
#include "RefCountingObject.h"

#include "Testbed/debug_log.h"

#include <new>
#include <assert.h>
#include <string.h>

static void Construct(HorseHandle *self) { new(self) HorseHandle(); }
static void Construct(HorseHandle *self, const HorseHandle &o) { new(self) HorseHandle(o); }
static void Destruct(HorseHandle *self) { self->~HorseHandle(); }

HorseHandle::HorseHandle()
{
    m_ref  = 0;

    HORSEHANDLE_DEBUGTRACE();
}

HorseHandle::HorseHandle(const HorseHandle &other)
{
    m_ref  = other.m_ref;

    AddRefHandle();

    HORSEHANDLE_DEBUGTRACE();
}

HorseHandle::HorseHandle(Horse *ref)
{
    // Used directly from C++, DO NOT increase refcount!
    // It's already been done by constructor/factory/AngelScript(if retrieved from script context).
    // See README.
    // ------------------------------------------

    m_ref  = ref;

    HORSEHANDLE_DEBUGTRACE();
}



HorseHandle::~HorseHandle()
{
    HORSEHANDLE_DEBUGTRACE();

    ReleaseHandle();
}

void HorseHandle::ReleaseHandle()
{
    HORSEHANDLE_DEBUGTRACE();

    if( m_ref )
    {
        m_ref->Release();
        m_ref = nullptr;
    }
}

void HorseHandle::AddRefHandle()
{
    HORSEHANDLE_DEBUGTRACE();

    if( m_ref )
    {
        m_ref->AddRef();
    }
}

HorseHandle &HorseHandle::operator =(const HorseHandle &other)
{
    HORSEHANDLE_DEBUGTRACE();

    Set(other.m_ref);

    return *this;
}

void HorseHandle::Set(Horse* ref)
{
    if( m_ref == ref ) return;

    ReleaseHandle();

    m_ref  = ref;

    AddRefHandle();
}

void *HorseHandle::GetRef()
{
    return m_ref;
}

bool HorseHandle::operator==(const HorseHandle &o) const
{
    return m_ref  == o.m_ref;
}

bool HorseHandle::operator!=(const HorseHandle &o) const
{
    return !(*this == o);
}

void HorseHandle::EnumReferences(asIScriptEngine *inEngine)
{
    // If we're holding a reference, we'll notify the garbage collector of it
    if (m_ref)
        inEngine->GCEnumCallback(m_ref);
}

void HorseHandle::ReleaseReferences(asIScriptEngine * /*inEngine*/)
{
    // Simply clear the content to release the references
    Set(nullptr);
}

// This will be a template in the future...
static void ConstructHorseHandleFromPtr(HorseHandle* self, void* objhandle)
{
    // Dereference the handle to get the object itself.
    // See AngelScript SDK, addon 'generic handle', function `Assign()`.
    void* obj = *static_cast<void**>(objhandle);
    Horse* ref = static_cast<Horse*>(obj);

    new(self)HorseHandle(ref);

    // Increase refcount manually because constructor is designed for C++ use only.
    if (ref)
        ref->AddRef();
}

// This will be a template in the future...
static Horse* ImplicitCastHorseHandle(HorseHandle* self)
{
    Horse* ref = static_cast<Horse*>(self->GetRef());
    ref->AddRef();
    return ref;
}

// This will be a template in the future...
static HorseHandle & HorseHandleOpAssign(HorseHandle* self, void* objhandle)
{
    // Dereference the handle to get the object itself.
    // See AngelScript SDK, addon 'generic handle', function `Assign()`.
    void* obj = *static_cast<void**>(objhandle);
    Horse* ref = static_cast<Horse*>(obj);

    self->Set(ref);
    return *self;
}

// This will be a template in the future...
static bool EquineOpEquals(HorseHandle* self, void* objhandle)
{
    // Dereference the handle to get the object itself.
    // See AngelScript SDK, addon 'generic handle', function `Equals()`.
    void* obj = *static_cast<void**>(objhandle);
    Horse* ref = static_cast<Horse*>(obj);

    return self->GetRef() == ref;
}

void RegisterHorseHandle(asIScriptEngine *engine)
{
    int r;

    // With C++11 it is possible to use asGetTypeTraits to automatically determine the flags that represent the C++ class
    r = engine->RegisterObjectType("HorseHandle", sizeof(HorseHandle), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC | asGetTypeTraits<HorseHandle>()); assert( r >= 0 );
    // construct/destruct
    r = engine->RegisterObjectBehaviour("HorseHandle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(Construct, (HorseHandle *), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("HorseHandle", asBEHAVE_CONSTRUCT, "void f(Horse@&in)", asFUNCTION(ConstructHorseHandleFromPtr), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("HorseHandle", asBEHAVE_CONSTRUCT, "void f(const HorseHandle &in)", asFUNCTIONPR(Construct, (HorseHandle *, const HorseHandle &), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("HorseHandle", asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(Destruct, (HorseHandle *), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    // GC
    r = engine->RegisterObjectBehaviour("HorseHandle", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(HorseHandle,EnumReferences), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("HorseHandle", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(HorseHandle, ReleaseReferences), asCALL_THISCALL); assert(r >= 0);
    // Cast
    r = engine->RegisterObjectMethod("HorseHandle", "Horse@ opImplCast()", asFUNCTION(ImplicitCastHorseHandle), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    // Assign
    r = engine->RegisterObjectMethod("HorseHandle", "HorseHandle &opHndlAssign(const HorseHandle &in)", asMETHOD(HorseHandle, operator=), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("HorseHandle", "HorseHandle &opHndlAssign(const Horse@&in)", asFUNCTION(HorseHandleOpAssign), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    // Equals
    r = engine->RegisterObjectMethod("HorseHandle", "bool opEquals(const HorseHandle &in) const", asMETHODPR(HorseHandle, operator==, (const HorseHandle &) const, bool), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("HorseHandle", "bool opEquals(const Horse@&in) const", asFUNCTION(EquineOpEquals), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
}



