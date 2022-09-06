
/// @file
/// @author Adopted from "scripthandle" AngelScript addon, distributed with AngelScript SDK.

#pragma once

#include <angelscript.h>
#include "RefCountingObject.h"
#include "Testbed/debug_log.h"

template<class T>
class RefCountingObjectPtr
{
public:
    // Constructors
    RefCountingObjectPtr();
    RefCountingObjectPtr(const RefCountingObjectPtr<T> &other);
    RefCountingObjectPtr(T *ref); // Only invoke directly using C++! AngelScript must use a wrapper.
    ~RefCountingObjectPtr();

    // Assignments
    RefCountingObjectPtr &operator=(const RefCountingObjectPtr<T> &other);
    // Intentionally omitting raw-pointer assignment, for simplicity - see raw pointer constructor.

    // Compare equalness
    bool operator==(const RefCountingObjectPtr<T> &o) const { return m_ref == o.m_ref; }
    bool operator!=(const RefCountingObjectPtr<T> &o) const { return m_ref != o.m_ref; }

    // Get the reference
    T *GetRef() { return m_ref; }
    T* operator->() { return m_ref; }

    // GC callback
    void EnumReferences(asIScriptEngine *engine);
    void ReleaseReferences(asIScriptEngine *engine);

    static void RegisterRefCountingObjectPtr(const char* handle_name, const char* obj_name, asIScriptEngine *engine);

protected:

    void Set(T* ref);
    void ReleaseHandle();
    void AddRefHandle();

    // Wrapper functions, to be invoked by AngelScript only!
    static void ConstructDefault(RefCountingObjectPtr<T> *self) { new(self) RefCountingObjectPtr(); }
    static void ConstructCopy(RefCountingObjectPtr<T> *self, const RefCountingObjectPtr &o) { new(self) RefCountingObjectPtr(o); }
    static void ConstructRef(RefCountingObjectPtr<T>* self, void* objhandle);
    static void Destruct(RefCountingObjectPtr<T> *self) { self->~RefCountingObjectPtr(); }
    static T* OpImplCast(RefCountingObjectPtr<T>* self);
    static RefCountingObjectPtr & OpAssign(RefCountingObjectPtr<T>* self, void* objhandle);
    static bool OpEquals(RefCountingObjectPtr<T>* self, void* objhandle);

    T *m_ref;
};

template<class T>
void RefCountingObjectPtr<T>::RegisterRefCountingObjectPtr(const char* handle_name, const char* obj_name, asIScriptEngine *engine)
{
    int r;
    const size_t DECLBUF_MAX = 300;
    char decl_buf[DECLBUF_MAX];

    // With C++11 it is possible to use asGetTypeTraits to automatically determine the flags that represent the C++ class
    r = engine->RegisterObjectType(handle_name, sizeof(RefCountingObjectPtr), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC | asGetTypeTraits<RefCountingObjectPtr>()); assert( r >= 0 );
    // construct/destruct
    r = engine->RegisterObjectBehaviour(handle_name, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(RefCountingObjectPtr::ConstructDefault), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    snprintf(decl_buf, DECLBUF_MAX, "void f(%s@&in)", obj_name);
    r = engine->RegisterObjectBehaviour(handle_name, asBEHAVE_CONSTRUCT, decl_buf, asFUNCTION(RefCountingObjectPtr::ConstructRef), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    snprintf(decl_buf, DECLBUF_MAX, "void f(const %s &in)", handle_name);
    r = engine->RegisterObjectBehaviour(handle_name, asBEHAVE_CONSTRUCT, decl_buf, asFUNCTION(RefCountingObjectPtr::ConstructCopy), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(handle_name, asBEHAVE_DESTRUCT, "void f()", asFUNCTION(RefCountingObjectPtr::Destruct), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    // GC
    r = engine->RegisterObjectBehaviour(handle_name, asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(RefCountingObjectPtr,EnumReferences), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour(handle_name, asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(RefCountingObjectPtr, ReleaseReferences), asCALL_THISCALL); assert(r >= 0);
    // Cast
    snprintf(decl_buf, DECLBUF_MAX, "%s@ opImplCast()", obj_name);
    r = engine->RegisterObjectMethod(handle_name, decl_buf, asFUNCTION(RefCountingObjectPtr::OpImplCast), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    // Assign
    snprintf(decl_buf, DECLBUF_MAX, "%s &opHndlAssign(const %s &in)", handle_name, handle_name);
    r = engine->RegisterObjectMethod(handle_name, decl_buf, asMETHOD(RefCountingObjectPtr, operator=), asCALL_THISCALL); assert( r >= 0 );
    snprintf(decl_buf, DECLBUF_MAX, "%s &opHndlAssign(const %s@&in)", handle_name, obj_name);
    r = engine->RegisterObjectMethod(handle_name, decl_buf, asFUNCTION(RefCountingObjectPtr::OpAssign), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    // Equals
    snprintf(decl_buf, DECLBUF_MAX, "bool opEquals(const %s &in) const", handle_name);
    r = engine->RegisterObjectMethod(handle_name, decl_buf, asMETHODPR(RefCountingObjectPtr, operator==, (const RefCountingObjectPtr &) const, bool), asCALL_THISCALL); assert( r >= 0 );
    snprintf(decl_buf, DECLBUF_MAX, "bool opEquals(const %s@&in) const", obj_name);
    r = engine->RegisterObjectMethod(handle_name, decl_buf, asFUNCTION(RefCountingObjectPtr::OpEquals), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
}


// ---------------------------- Internals ------------------------------

template<class T>
inline void RefCountingObjectPtr<T>::ConstructRef(RefCountingObjectPtr<T>* self, void* objhandle)
{
    // Dereference the handle to get the object itself.
    // See AngelScript SDK, addon 'generic handle', function `Assign()`.
    void* obj = *static_cast<void**>(objhandle);
    T* ref = static_cast<T*>(obj);

    new(self)RefCountingObjectPtr(ref);

    // Increase refcount manually because constructor is designed for C++ use only.
    if (ref)
        ref->AddRef();
}

template<class T>
inline T* RefCountingObjectPtr<T>::OpImplCast(RefCountingObjectPtr<T>* self)
{
    T* ref = self->GetRef();
    ref->AddRef();
    return ref;
}

template<class T>
inline RefCountingObjectPtr<T> & RefCountingObjectPtr<T>::OpAssign(RefCountingObjectPtr<T>* self, void* objhandle)
{
    // Dereference the handle to get the object itself.
    // See AngelScript SDK, addon 'generic handle', function `Assign()`.
    void* obj = *static_cast<void**>(objhandle);
    T* ref = static_cast<T*>(obj);

    self->Set(ref);
    return *self;
}

template<class T>
inline bool RefCountingObjectPtr<T>::OpEquals(RefCountingObjectPtr<T>* self, void* objhandle)
{
    // Dereference the handle to get the object itself.
    // See AngelScript SDK, addon 'generic handle', function `Equals()`.
    void* obj = *static_cast<void**>(objhandle);
    T* ref = static_cast<T*>(obj);

    return self->GetRef() == ref;
}

template<class T>
inline RefCountingObjectPtr<T>::RefCountingObjectPtr()
{
    m_ref  = 0;

    RCOP_DEBUGTRACE_SELF();
}

template<class T>
inline RefCountingObjectPtr<T>::RefCountingObjectPtr(const RefCountingObjectPtr<T> &other)
{
    m_ref  = other.m_ref;

    AddRefHandle();

    RCOP_DEBUGTRACE_ARG_PTR(other);
}

template<class T>
inline RefCountingObjectPtr<T>::RefCountingObjectPtr(T *ref)
{
    // Used directly from C++, DO NOT increase refcount!
    // It's already been done by constructor/factory/AngelScript(if retrieved from script context).
    // See README.
    // ------------------------------------------

    m_ref  = ref;

    RCOP_DEBUGTRACE_ARG_OBJ(ref);
}

template<class T>
inline RefCountingObjectPtr<T>::~RefCountingObjectPtr()
{
    RCOP_DEBUGTRACE_SELF();

    ReleaseHandle();
}

template<class T>
inline void RefCountingObjectPtr<T>::ReleaseHandle()
{
    RCOP_DEBUGTRACE_SELF();

    if( m_ref )
    {
        m_ref->Release();
        m_ref = nullptr;
    }
}

template<class T>
inline void RefCountingObjectPtr<T>::AddRefHandle()
{
    RCOP_DEBUGTRACE_SELF();

    if( m_ref )
    {
        m_ref->AddRef();
    }
}

template<class T>
inline RefCountingObjectPtr<T> &RefCountingObjectPtr<T>::operator =(const RefCountingObjectPtr<T> &other)
{
    RCOP_DEBUGTRACE_ARG_PTR(other);

    Set(other.m_ref);

    return *this;
}

template<class T>
inline void RefCountingObjectPtr<T>::Set(T* ref)
{
    if( m_ref == ref ) return;

    ReleaseHandle();

    m_ref  = ref;

    AddRefHandle();
}

template<class T>
inline void RefCountingObjectPtr<T>::EnumReferences(asIScriptEngine *inEngine)
{
    // If we're holding a reference, we'll notify the garbage collector of it
    if (m_ref)
        inEngine->GCEnumCallback(m_ref);
}

template<class T>
inline void RefCountingObjectPtr<T>::ReleaseReferences(asIScriptEngine * /*inEngine*/)
{
    // Simply clear the content to release the references
    Set(nullptr);
}

